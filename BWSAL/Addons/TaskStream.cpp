#include <MacroManager/TaskStream.h>
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/UnitReadyTimeCalculator.h>
#include <math.h>
using namespace BWAPI;
using namespace std;
TaskStream::TaskStream(Task t, Task nt)
{
  task[0]    = t;
  task[1]    = nt;
  worker     = NULL;
  buildUnit  = NULL;
  status     = None;
  killSwitch = false;
  name       = "Task Stream";
  plannedAdditionalResources = false;
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
    {
      TheArbitrator->decline(this,u,0);
      TheArbitrator->removeBid(this,u);
    }
  }
}
void TaskStream::onRevoke(BWAPI::Unit* unit, double bid)
{
  if (worker == unit)
  {
    TheArbitrator->removeBid(this,unit);
    worker = NULL;
    workerReady = false;
  }
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
      TilePosition tp = task[0].getTilePosition();
      if (ut.isAddon())
      {
        tp.x()+=4;
        tp.y()++;
      }
      if (!Broodwar->canBuildHere(worker,tp,ut)) //doesn't work for blocked addons!
      {
        status = Error_Location_Blocked;
        return;
      }
    }
  }
  if (worker == NULL || !worker->exists())
  {
    status = Error_Worker_Not_Specified;
    setWorker(NULL);
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
  }
  if (task[0].hasSpentResources())
    status = Executing_Task;
  for(int i=0;i<2;i++)
  {
    if (i>0 && task[i-1].getStartTime()==-1) break;
    if (!task[i].hasReservedResourcesThisFrame())
    {
      UnitReadyTimeStatus::Enum reason;
      int first_valid_frame = UnitReadyTimeCalculator::getReadyTime(worker, task[i], reason, true, false);
      if (i>0 && first_valid_frame!=-1)
      {
        if (first_valid_frame<task[i-1].getFinishTime())
          first_valid_frame=task[i-1].getFinishTime();
        reason = UnitReadyTimeStatus::Waiting_For_Worker_To_Be_Ready;
      }

      task[i].setStartTime(first_valid_frame);
      //if we need to wait to start the first task, compute the correct status
      if ( i==0 ) //compute task stream status based on status of first unit
      {
        if (task[0].getStartTime()!=-1 && task[0].getStartTime()<=Broodwar->getFrameCount())
          status = Executing_Task;
        else
        {
          if (reason == UnitReadyTimeStatus::Waiting_For_Worker_To_Be_Ready)
            status = Waiting_For_Worker_To_Be_Ready;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Required_Units)
            status = Waiting_For_Required_Units;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Required_Tech)
            status = Waiting_For_Required_Tech;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Required_Upgrade)
            status = Waiting_For_Required_Upgrade;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Supply)
            status = Waiting_For_Supply;
          else if (reason == UnitReadyTimeStatus::Waiting_For_Gas)
            status = Waiting_For_Gas;
          else
            status = Waiting_For_Minerals;
        }
      }

      if (first_valid_frame==-1) break;
      TheMacroManager->rtl.reserveResources(first_valid_frame,task[i].getResources());
      if (task[i].getType()==TaskTypes::Tech)
        TheMacroManager->ttl.registerTechStart(first_valid_frame,task[i].getTech());
      task[i].setReservedResourcesThisFrame(true);
    }
    if (task[i].hasReservedResourcesThisFrame() && !task[0].hasReservedFinishDataThisFrame())
    {
      if (task[i].getType()==TaskTypes::Unit)
      {
        if (task[i].getUnit().supplyProvided()>0)
          TheMacroManager->rtl.registerSupplyIncrease(task[i].getFinishTime(), task[i].getUnit().supplyProvided());
        int count = 1;
        if (task[i].getUnit().isTwoUnitsInOneEgg())
          count = 2;
        TheMacroManager->uctl.registerUnitCountChange(task[i].getFinishTime(), task[i].getUnit(), count);
        if (task[i].getType()==TaskTypes::Tech)
          TheMacroManager->ttl.registerTechFinish(task[i].getFinishTime(),task[i].getTech());
        if (task[i].getType()==TaskTypes::Upgrade)
          TheMacroManager->utl.registerUpgradeLevelIncrease(task[i].getFinishTime(),task[i].getUpgrade());
        plannedAdditionalResources = true;
      }

      task[i].setReservedFinishDataThisFrame(true);
    }
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
bool TaskStream::updateStatus()
{
  plannedAdditionalResources = false;
  if (killSwitch) return false;
  Status lastStatus = status;
  computeStatus();
  if (status != lastStatus)
    notifyNewStatus();
  return plannedAdditionalResources;
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
    case Waiting_For_Worker_To_Be_Ready:
      return "Waiting_For_Worker_To_Be_Ready";
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
  BWAPI::Unit* oldWorker = worker;
  worker = w;
  if (oldWorker!=NULL)
  {
    TheArbitrator->removeBid(this,oldWorker);
    if (TheMacroManager->unitToTaskStream.find(oldWorker)!=TheMacroManager->unitToTaskStream.end() && TheMacroManager->unitToTaskStream[oldWorker] == this)
      TheMacroManager->unitToTaskStream.erase(oldWorker);
  }
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
  Broodwar->drawTextScreen(x,y,"[ ] %s - %x ",
    getStatusString().c_str(),
    getWorker());
  Broodwar->drawTextScreen(x+200,y,"%s %s %d",
    task[0].getVerb().c_str(),
    task[0].getName().c_str(),
    task[0].getStartTime());
  Broodwar->drawTextScreen(x+350,y,"%s %s %d",
    task[1].getVerb().c_str(),
    task[1].getName().c_str(),
    task[1].getStartTime());
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
  task[0].setReservedFinishDataThisFrame(task[0].isCompleted());
  if (!task[0].hasSpentResources())
    task[0].setStartTime(-1);
  task[1].setReservedResourcesThisFrame(task[1].hasSpentResources());
  task[1].setReservedFinishDataThisFrame(task[0].isCompleted());
  if (!task[1].hasSpentResources())
    task[1].setStartTime(-1);
  plannedAdditionalResources = false;
}
int TaskStream::getStartTime() const
{
  return task[0].getStartTime();
}
int TaskStream::getFinishTime() const
{
  if (task[0]==NULL)
    return Broodwar->getFrameCount();
  if (task[0].getFinishTime() == -1)
    return -1;
  if (task[1]==NULL)
    return task[0].getFinishTime();
  if (task[1].getFinishTime() == -1)
    return -1;
  return task[1].getFinishTime();
}