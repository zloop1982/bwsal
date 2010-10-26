#include <MacroManager/TaskStream.h>
#include <MacroManager/TaskStreamObserver.h>
using namespace BWAPI;
TaskStream::TaskStream(Task t, Task nt)
{
  task        = t;
  nextTask    = nt;
  worker      = NULL;
  buildUnit   = NULL;
  status      = None;
  isStarted   = false;
  isCompleted = false;
  name        = "Task Stream";
}
TaskStream::~TaskStream()
{
  //delete observers that we own
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    if (obs.second)
      delete obs.first;
  }
  observers.clear();
}
void TaskStream::terminate()
{
  TheMacroManager->killSet.insert(this);
}
void TaskStream::onOffer(std::set<BWAPI::Unit*> units)
{
  for each(Unit* u in units)
  {
    if (u == worker)
      TheArbitrator->accept(this,u);
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
  if (worker == NULL || !worker->exists())
  {
    status = Error_Worker_Not_Specified;
    return;
  }
  if (TheArbitrator->getHighestBidder(worker).first!=this)
  {
    status = Error_Worker_Not_Owned;
    return;
  }
  if (task == NULL)
  {
    status = Error_Task_Not_Specified;
    return;
  }
  if (task.getType()==TaskTypes::Unit)
  {
    UnitType ut = task.getUnit();
    if (ut.isBuilding() && !ut.whatBuilds().first.isBuilding() && buildUnit == NULL)
    {
      if (task.getTilePosition().isValid()==false)
      {
        status = Error_Location_Not_Specified;
        return;
      }
      if (!worker->hasPath(Position(task.getTilePosition())))
      {
        status = Error_Location_Unreachable;
        return;
      }
      if (!Broodwar->canBuildHere(worker,task.getTilePosition(),ut))
      {
        status = Error_Location_Blocked;
        return;
      }
    }
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
  if (task.getType()==TaskTypes::Upgrade)
  {
    if (Broodwar->self()->isUpgrading(task.getUpgrade()))
    {
      status = Waiting_For_Required_Upgrade;
      return;
    }
  }
  if (status != Executing_Task)
  {
    if (TheMacroManager->rtl.reserveResources(Broodwar->getFrameCount(),task.getResources()))
    {
      status = Executing_Task;
    }
    else
    {
      if (TheMacroManager->rtl.getLastError() == ResourceTimeline::Insufficient_Supply)
        status = Waiting_For_Supply;
      else if (TheMacroManager->rtl.getLastError() == ResourceTimeline::Insufficient_Gas)
        status = Waiting_For_Gas;
      else
        status = Waiting_For_Minerals;
      return;
    }
  }
}
void TaskStream::update()
{
  if (status == Executing_Task)
  {
    if (isCompleted)
    {
      notifyCompletedTask();
      isStarted = false;
      isCompleted = false;
      status = None;
      task = nextTask;
      nextTask = Task();
      Broodwar->printf("Completed Task!");
    }
    Broodwar->drawTextMap(worker->getPosition().x(),worker->getPosition().y(),"Task: %s",task.getName().c_str());
  }
  else
  {
    if (status != Error_Worker_Not_Specified && status != Error_Worker_Not_Owned)
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
      else if (worker->isIdle()==false)
      {
        worker->stop();
      }
    }
  }
  for each(std::pair<TaskStreamObserver*, bool> obs in observers)
  {
    obs.first->update(this);
  }
  Status oldStatus = status;
  computeStatus();
  if (status != oldStatus)
  {
    notifyNewStatus();
  }
}
void TaskStream::attach(TaskStreamObserver* obs, bool owned)
{
  observers.insert(std::make_pair(obs, owned));
}
void TaskStream::detach(TaskStreamObserver* obs)
{
  observers.erase(obs);
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
    obs.first->completedTask(this,task);
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
    case Error_Location_Unreachable:
      return "Error_Location_Unreachable";
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
  task = t;
}
Task& TaskStream::getTask()
{
  return task;
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
  nextTask = t;
}
Task& TaskStream::getNextTask()
{
  return nextTask;
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
  Broodwar->drawTextScreen(x,y,"Task: %s %s, Status: %s, Worker: %x, Started: %d",task.getVerb().c_str(),task.getName().c_str(),getStatusString().c_str(),worker,isStarted);
}

void TaskStream::setTaskStarted(bool started)
{
  isStarted = started;
}
void TaskStream::completeTask()
{
  isCompleted = true;
}
bool TaskStream::isTaskStarted() const
{
  return isStarted;
}
bool TaskStream::isTaskCompleted() const
{
  return isCompleted;
}