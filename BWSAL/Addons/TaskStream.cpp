#include <MacroManager/TaskStream.h>
#include <MacroManager/TaskStreamObserver.h>
using namespace BWAPI;
TaskStream::TaskStream(Task t, Task nt)
{
  task = t;
  nextTask = nt;
  worker = NULL;
  status = Not_Initialized;
}
void TaskStream::onOffer(std::set<BWAPI::Unit*> units)
{
}
void TaskStream::onRevoke(BWAPI::Unit* unit, double bid)
{
}
void TaskStream::update()
{
  if (worker == NULL || !worker->exists())
  {
    status = Error_Worker_Not_Specified;
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
    if (ut.isBuilding() && !ut.whatBuilds().first.isBuilding())
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
      if (t.second < Broodwar->self()->completedUnitCount(t.first))
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
  if (TheArbitrator->getHighestBidder(worker).first!=this)
  {
    status = Waiting_For_Worker;
    return;
  }
  //if current task is completed
  //{
  //  task = nextTask
  //  nextTask = Task()
  //}
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
  if (status == Executing_Task)
  {
    Broodwar->drawTextMap(worker->getPosition().x(),worker->getPosition().y(),"Task: %s",task.getName().c_str());
  }
}
void TaskStream::attach(TaskStreamObserver* obs)
{
  observers.insert(obs);
}
void TaskStream::detach(TaskStreamObserver* obs)
{
  observers.erase(obs);
}

void TaskStream::notifyObservers()
{
  for each(TaskStreamObserver* obs in observers)
  {
    //obs->notify();
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
    case Not_Initialized:
      return "Not_Initialized";
    break;
    case Error_Worker_Not_Specified:
      return "Error_Worker_Not_Specified";
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
    case Waiting_For_Worker:
      return "Waiting_For_Worker";
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
    TheArbitrator->removeBid(this,worker);
  worker = w;
  TheArbitrator->setBid(this,worker,100);
}
BWAPI::Unit* TaskStream::getWorker() const
{
  return worker;
}
void TaskStream::setTask(Task t)
{
  task = t;
}
Task TaskStream::getTask() const
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
Task TaskStream::getNextTask() const
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
  Broodwar->drawTextScreen(x,y,"Task: %s %s, Status: %s, Worker: %x",task.getVerb().c_str(),task.getName().c_str(),getStatusString().c_str(),worker);
}