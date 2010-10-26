#include <MacroManager/TaskStream.h>
#include <MacroManager/TaskStreamObserver.h>
using namespace BWAPI;
TaskStream::TaskStream(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  this->arbitrator = arbitrator;
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
  if (task.getType()==TaskType::Unit)
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
  if (task.getType()==TaskType::Upgrade)
  {
    if (Broodwar->self()->isUpgrading(task.getUpgrade()))
    {
      status = Waiting_For_Required_Upgrade;
      return;
    }
  }
  if (arbitrator->getHighestBidder(worker).first!=this)
  {
    status = Waiting_For_Worker;
    return;
  }
  /*
  Implement supply/mineral/gas reserve system

  if (task.getType()==TaskType::Unit && task.getUnit().supplyUsed()>0)
  {
    if (Broodwar->self()->supplyUsed()+task.getUnit().supplyUsed()>Broodwar->self()->supplyTotal())
    {
      status = Waiting_For_Supply;
      return;
    }
  }
  */



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
void TaskStream::setWorker(BWAPI::Unit* w)
{
  worker = w;
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