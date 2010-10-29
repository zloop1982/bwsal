#include <MacroManager/TaskStream.h>
#include <MacroManager/TaskStreamObserver.h>
#include <math.h>
using namespace BWAPI;
using namespace std;
TaskStream::TaskStream(Task t, Task nt)
{
  task[0]     = t;
  task[1]     = nt;
  worker      = NULL;
  buildUnit   = NULL;
  status      = None;
  killSwitch  = false;
  name        = "Task Stream";
}
TaskStream::~TaskStream()
{
  //delete observers that we own
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->detached(this);
    if (obs.second)
      delete obs.first;
  }
  observers.clear();
  TheArbitrator->removeAllBids(this);
}
void TaskStream::terminate()
{
  TheMacroManager->killSet.insert(this);
  TheArbitrator->removeAllBids(this);
  killSwitch = true;
}
void TaskStream::onOffer(std::set<BWAPI::Unit*> units)
{
  for each(Unit* u in units)
  {
    if (u == worker)
    {
      TheArbitrator->accept(this,u);
      TheArbitrator->setBid(this,u,100);
      workerReady = true;
    }
    else
      TheArbitrator->decline(this,u,0);
  }
}
void TaskStream::onRevoke(BWAPI::Unit* unit, double bid)
{
  setWorker(NULL);
}
void TaskStream::computeStatus()
{
  locationReady = true;
  workerReady = (worker != NULL) && worker->exists() && TheArbitrator->hasBid(worker) && TheArbitrator->getHighestBidder(worker).first==this;
  if (task[0].getType()==TaskTypes::Unit)
  {
    UnitType ut = task[0].getUnit();
    if (ut.isBuilding() && !ut.whatBuilds().first.isBuilding() && buildUnit == NULL)
    {
      if (task[0].getTilePosition().isValid()==false || !Broodwar->canBuildHere(worker,task[0].getTilePosition(),ut))
        locationReady = false;
    }
  }
  if (task[0] == NULL)
  {
    status = Error_Task_Not_Specified;
    return;
  }
  if (task[0].getType()==TaskTypes::Unit)
  {
    UnitType ut = task[0].getUnit();
    if (ut.isBuilding() && !ut.whatBuilds().first.isBuilding() && buildUnit == NULL)
    {
      if (task[0].getTilePosition().isValid()==false)
      {
        status = Error_Location_Not_Specified;
        return;
      }
      if (!Broodwar->canBuildHere(worker,task[0].getTilePosition(),ut))
      {
        status = Error_Location_Blocked;
        return;
      }
    }
  }
  if (worker == NULL || !worker->exists())
  {
    status = Error_Worker_Not_Specified;
    worker = NULL;
    return;
  }
  if (TheArbitrator->hasBid(worker)==false || TheArbitrator->getHighestBidder(worker).first!=this)
  {
    status = Error_Worker_Not_Owned;
    return;
  }
  if (task[0].getType()==TaskTypes::Unit)
  {
    UnitType ut = task[0].getUnit();
    for each(std::pair<UnitType, int> t in ut.requiredUnits())
    {
      if (t.first.isAddon() && t.first.whatBuilds().first == worker->getType() && worker->getAddon() == NULL)
      {
        status = Error_Task_Requires_Addon;
        return;
      }
    }
    for each(std::pair<UnitType, int> t in ut.requiredUnits())
    {
      if (t.second > Broodwar->self()->completedUnitCount(t.first))
      {
        status = Waiting_For_Required_Units;
        return;
      }
    }
    if (ut.requiredTech()!=TechTypes::None && Broodwar->self()->hasResearched(ut.requiredTech())==false)
    {
      status = Waiting_For_Required_Tech;
      return;
    }
  }
  if (task[0].getType()==TaskTypes::Upgrade)
  {
    if (Broodwar->self()->isUpgrading(task[0].getUpgrade()))
    {
      status = Waiting_For_Required_Upgrade;
      return;
    }
  }
  ResourceTimeline::ErrorCode error = ResourceTimeline::None;
  for(int i=0;i<2;i++)
  {
    if (i>0 && task[i-1].getStartFrame()==-1) break;
    if (!task[i].hasReservedResourcesThisFrame())
    {
      int frame = TheMacroManager->rtl.getFirstValidTime(task[i].getResources());

      if (frame==-1 && i==0)
        error = TheMacroManager->rtl.getLastError();
      if (i>0 && frame!=-1)
        frame = max(frame,task[i-1].getStartFrame()+task[i-1].getTime());

      if (frame==-1) break;

      task[i].setStartFrame(frame);
      TheMacroManager->rtl.reserveResources(frame,task[i].getResources());
      task[i].setReservedResourcesThisFrame(true);
    }
    if (task[i].hasReservedResourcesThisFrame() && !task[0].hasCreatedSupplyThisFrame())
    {
      if (task[i].getType()==TaskTypes::Unit && task[i].getUnit().supplyProvided()>0)
        TheMacroManager->rtl.registerSupplyIncrease(task[i].getStartFrame()+task[i].getTime(), task[i].getUnit().supplyProvided());
      task[i].setCreatedSupplyThisFrame(true);
    }
  }
  if (task[0].getStartFrame()!=-1 && task[0].getStartFrame()<=Broodwar->getFrameCount())
  {
    status = Executing_Task;
  }
  else
  {
    if (error == ResourceTimeline::Insufficient_Supply)
      status = Waiting_For_Supply;
    else if (error == ResourceTimeline::Insufficient_Gas)
      status = Waiting_For_Gas;
    else
      status = Waiting_For_Minerals;
    return;
  }
}
void TaskStream::update()
{
  if (killSwitch) return;
  if (status == Executing_Task)
  {
    if (task[0].isCompleted())
    {
      notifyCompletedTask();
      status = None;
      task[0] = task[1];
      task[1] = Task();
      buildUnit = NULL;
      Broodwar->printf("Completed Task!");
    }
    Broodwar->drawTextMap(worker->getPosition().x(),worker->getPosition().y(),"Task: %s",task[0].getName().c_str());
  }
  else
  {
    if (workerReady)
    {
      if (worker->isResearching())
        worker->cancelResearch();
      else if (worker->isUpgrading())
        worker->cancelUpgrade();
      else if (worker->isTraining())
        worker->cancelTrain();
      else if (worker->isMorphing())
        worker->cancelMorph();
      else if (worker->isConstructing())
      {
        if (worker->getBuildUnit() && worker->getBuildUnit()->getType().isBuilding())
          worker->getBuildUnit()->cancelConstruction();
        worker->cancelConstruction();
      }
    }
  }
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->update(this);
  }
}
void TaskStream::updateStatus()
{
  if (killSwitch) return;
  Status lastStatus = status;
  computeStatus();
  if (status != lastStatus)
    notifyNewStatus();
}
void TaskStream::attach(TaskStreamObserver* obs, bool owned)
{
  observers.insert(std::make_pair(obs, owned));
  obs->attached(this);
}
void TaskStream::detach(TaskStreamObserver* obs)
{
  observers.erase(obs);
  obs->detached(this);
}

void TaskStream::notifyNewStatus()
{
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->newStatus(this);
  }
}
void TaskStream::notifyCompletedTask()
{
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->completedTask(this,task[0]);
  }
}
TaskStream::Status TaskStream::getStatus() const
{
  return status;
}
std::string TaskStream::getStatusString() const
{
  switch (status)
  {
    case None:
      return "None";
    break;
    case Error_Worker_Not_Specified:
      return "Error_Worker_Not_Specified";
    break;
    case Error_Worker_Not_Owned:
      return "Error_Worker_Not_Owned";
    break;
    case Error_Task_Not_Specified:
      return "Error_Task_Not_Specified";
    break;
    case Error_Location_Not_Specified:
      return "Error_Location_Not_Specified";
    break;
    case Error_Location_Blocked:
      return "Error_Location_Blocked";
    break;
    case Error_Task_Requires_Addon:
      return "Error_Task_Requires_Addon";
    break;
    case Waiting_For_Required_Units:
      return "Waiting_For_Required_Units";
    break;
    case Waiting_For_Required_Tech:
      return "Waiting_For_Required_Tech";
    break;
    case Waiting_For_Required_Upgrade:
      return "Waiting_For_Required_Upgrade";
    break;
    case Waiting_For_Supply:
      return "Waiting_For_Supply";
    break;
    case Waiting_For_Gas:
      return "Waiting_For_Gas";
    break;
    case Waiting_For_Minerals:
      return "Waiting_For_Minerals";
    break;
    case Executing_Task:
      return "Executing_Task";
    break;
    default:
      return "Unknown";
  }
  return "Unknown";
}
void TaskStream::setWorker(BWAPI::Unit* w)
{
  if (worker!=NULL)
  {
    TheArbitrator->removeBid(this,worker);
    TheMacroManager->unitToTaskStream.erase(worker);
  }
  worker = w;
  if (worker!=NULL)
  {
    TheArbitrator->setBid(this,worker,100);
    TheMacroManager->unitToTaskStream[worker] = this;
  }
  workerReady = false;
}
BWAPI::Unit* TaskStream::getWorker() const
{
  return worker;
}
void TaskStream::setBuildUnit(BWAPI::Unit* b)
{
  buildUnit = b;
}
BWAPI::Unit* TaskStream::getBuildUnit() const
{
  return buildUnit;
}
void TaskStream::setTask(Task t)
{
  task[0] = t;
}
Task& TaskStream::getTask()
{
  return task[0];
}
void TaskStream::setUrgent(bool isUrgent)
{
  urgent = isUrgent;
}
bool TaskStream::isUrgent() const
{
  return urgent;
}
void TaskStream::setNextTask(Task t)
{
  task[1] = t;
}
Task& TaskStream::getNextTask()
{
  return task[1];
}
void TaskStream::setName(std::string s)
{
  name = s;
}
std::string TaskStream::getName() const
{
  return name;
}
std::string TaskStream::getShortName() const
{
  return "Macro";
}
void TaskStream::printToScreen(int x, int y)
{
  Broodwar->drawTextScreen(x,y,"[ ] %s - %d ",
    getStatusString().c_str(),
    task[0].isCompleted());
  Broodwar->drawTextScreen(x+200,y,"%s %s %d",
    task[0].getVerb().c_str(),
    task[0].getName().c_str(),
    task[0].getStartFrame());
  Broodwar->drawTextScreen(x+350,y,"%s %s %d",
    task[1].getVerb().c_str(),
    task[1].getName().c_str(),
    task[1].getStartFrame());
}

bool TaskStream::isWorkerReady() const
{
  return workerReady;
}
bool TaskStream::isLocationReady() const
{
  return locationReady;
}
void TaskStream::clearPlanningData()
{
  task[0].setReservedResourcesThisFrame(task[0].hasSpentResources());
  task[0].setCreatedSupplyThisFrame(task[0].isCompleted());
  if (!task[0].hasSpentResources())
    task[0].setStartFrame(-1);
  task[1].setReservedResourcesThisFrame(task[1].hasSpentResources());
  task[1].setCreatedSupplyThisFrame(task[0].isCompleted());
  if (!task[1].hasSpentResources())
    task[1].setStartFrame(-1);
}