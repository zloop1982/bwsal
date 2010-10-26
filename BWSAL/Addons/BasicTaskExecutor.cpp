#include <BasicTaskExecutor.h>
using namespace BWAPI;
BasicTaskExecutor* instance = NULL;
BasicTaskExecutor* BasicTaskExecutor::getInstance()
{
  if (instance==NULL)
    instance = new BasicTaskExecutor();
  return instance;
}
void BasicTaskExecutor::attached(TaskStream* ts)
{
  taskStreams[ts].buildUnit = NULL;
  taskStreams[ts].started = false;
}
void BasicTaskExecutor::detached(TaskStream* ts)
{
  taskStreams.erase(ts);
}
void BasicTaskExecutor::newStatus(TaskStream* ts)
{
}
void BasicTaskExecutor::completedTask(TaskStream* ts, const Task &t)
{
  taskStreams[ts].buildUnit = NULL;
  ts->setBuildUnit(NULL);
  taskStreams[ts].started = false;
}
void BasicTaskExecutor::update(TaskStream* ts)
{
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  if (ts->getStatus() != TaskStream::Executing_Task) return;
  if (ts->isTaskCompleted()) return;


  if ( taskStreams[ts].started == false )
  {
    if (Broodwar->getFrameCount()<ts->getWorker()->getLastOrderFrame()+5) return;

    if (worker->isConstructing() || worker->isTraining() || worker->isMorphing())
      worker->stop();

    if (type == TaskTypes::Unit)
    {
      UnitType ut=ts->getTask().getUnit();
      if (ut.getRace()==Races::Zerg && ut.isBuilding()==ut.whatBuilds().first.isBuilding())
        worker->morph(ut);
      else if (ut.isBuilding())
      {
        Position targetPosition=Position(ts->getTask().getTilePosition());
        targetPosition.x()+=ut.tileWidth()*16;
        targetPosition.y()+=ut.tileHeight()*16;
        if (worker->getDistance(targetPosition)>100)
          worker->rightClick(targetPosition);
        else
          worker->build(ts->getTask().getTilePosition(),ut);
      }
      else
        worker->train(ut);
    }
    else if (type == TaskTypes::Tech)
    {
      worker->research(ts->getTask().getTech());
    }
    else if (type == TaskTypes::Upgrade)
    {
      worker->upgrade(ts->getTask().getUpgrade());
      taskStreams[ts].targetLevel = Broodwar->self()->getUpgradeLevel(ts->getTask().getUpgrade())+1;
    }
  }
  else
  {
    if (type == TaskTypes::Unit)
    {
      if (worker->getBuildUnit()!=NULL && worker->getBuildUnit()->exists())
        taskStreams[ts].buildUnit = worker->getBuildUnit();
      if (worker->getType().getRace()==Races::Zerg && worker->getType()!=UnitTypes::Zerg_Infested_Command_Center)
        taskStreams[ts].buildUnit = worker;
      if (taskStreams[ts].buildUnit!=NULL && taskStreams[ts].buildUnit->exists() == false)
        taskStreams[ts].buildUnit = NULL;
      if (taskStreams[ts].buildUnit!=NULL &&
          taskStreams[ts].buildUnit->isCompleted() &&
          taskStreams[ts].buildUnit->getType() == ts->getTask().getUnit())
      {
        ts->completeTask();
      }
    }
    else if (type == TaskTypes::Tech)
    {
      if (Broodwar->self()->hasResearched(ts->getTask().getTech()))
        ts->completeTask();
    }
    else if (type == TaskTypes::Upgrade)
    {
      if (Broodwar->self()->getUpgradeLevel(ts->getTask().getUpgrade()) >= taskStreams[ts].targetLevel)
        ts->completeTask();
    }
  }
  if (ts->isTaskCompleted()==false)
  {
    taskStreams[ts].started = ts->getTask().isBeingExecutedBy(worker);
    ts->setTaskStarted(taskStreams[ts].started);
    ts->setBuildUnit(taskStreams[ts].buildUnit);
  }
}