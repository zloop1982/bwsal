#include <BasicTaskExecutor.h>
#include <MacroManager.h>
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
  taskStreams[ts].isReady = false;
  taskStreams[ts].isExecuting = false;
  taskStreams[ts].targetLevel = -1;
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
}
void BasicTaskExecutor::update(TaskStream* ts)
{
  if (ts == NULL) return;
  if (ts->isWorkerReady()==false) return;
  if (ts->isLocationReady()==false) return;
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  if (ts->getStatus() != TaskStream::Executing_Task)
  {
    if (ts->getTask().getStartTime()>=0 &&
        ts->getTask().getType()==TaskTypes::Unit &&
        ts->getTask().getUnit().isBuilding() &&
        !ts->getTask().getWorkerType().isBuilding())
    {
      UnitType ut = ts->getTask().getUnit();
      Position targetPosition=Position(ts->getTask().getTilePosition());
      targetPosition.x()+=ut.tileWidth()*16;
      targetPosition.y()+=ut.tileHeight()*16;
      if (ts->getTask().getStartTime()<Broodwar->getFrameCount()+ts->getWorker()->getDistance(targetPosition)/ts->getTask().getWorkerType().topSpeed()*1.4+0.8*30)
      {
        if (worker->getDistance(targetPosition)>100)
        {
          if (Broodwar->getFrameCount()>=ts->getWorker()->getLastOrderFrame()+5)
            worker->rightClick(targetPosition);
        }
      }
    }
    return;
  }
  computeBuildUnit(ts);
  computeIsCompleted(ts);
  Broodwar->drawTextMap(worker->getPosition().x(),worker->getPosition().y()+15,"r: %d, e: %d, c: %d",taskStreams[ts].isReady,taskStreams[ts].isExecuting, ts->getTask().isCompleted());
  if (ts->getTask().isCompleted()) return;
  computeIsExecuting(ts);
  if (taskStreams[ts].isExecuting) return;
  computeIsReady(ts);
  if (!taskStreams[ts].isReady)
    getReady(ts);
  else
    execute(ts);
}
void BasicTaskExecutor::computeBuildUnit(TaskStream* ts)
{
  Unit* worker = ts->getWorker();
  UnitType ut = ts->getTask().getUnit();
  if (ts->getTask().getType() != TaskTypes::Unit) return;

  if (worker->getBuildUnit() != NULL && worker->getBuildUnit()->exists() && (worker->getBuildUnit()->getType() == ut || worker->getBuildUnit()->getBuildType() == ut))
    ts->setBuildUnit(worker->getBuildUnit());

  if (worker->getAddon() != NULL && worker->getAddon()->exists() && (worker->getAddon()->getType() == ut || worker->getAddon()->getBuildType() == ut))
    ts->setBuildUnit(worker->getAddon());

  //check to see if the worker is the right type
  //Zerg_Nydus_Canal is speciall since Zerg_Nydus_Canal can construct Zerg_Nydus_Canal
  if ((worker->getType() == ut || worker->getBuildType() == ut) && worker->getType()!=UnitTypes::Zerg_Nydus_Canal)
    ts->setBuildUnit(worker);

  //if the building dies, or isn't the right type, set it to null
  if (!(ts->getBuildUnit() != NULL && ts->getBuildUnit()->exists() && (ts->getBuildUnit()->getType() == ut || ts->getBuildUnit()->getBuildType() == ut)))
    ts->setBuildUnit(NULL);

  if (ts->getBuildUnit()==NULL && ut.isBuilding()) //if we don't have a building yet, look for it
  {
    TilePosition bl = ts->getTask().getTilePosition();
    //look at the units on the tile to see if it exists yet
    for each(Unit* u in Broodwar->unitsOnTile(bl.x(), bl.y()))
      if (u->getType() == ut && !u->isLifted())
      {
        //we found the building
        ts->setBuildUnit(u);
        break;
      }
  }
  if (ts->getBuildUnit()==NULL && ut.isAddon()) //if we don't have a building yet, look for it
  {
    TilePosition bl = ts->getTask().getTilePosition();
    bl.x()+=4;
    bl.y()++;
    for each(Unit* u in Broodwar->unitsOnTile(bl.x(), bl.y()))
      if (u->getType() == ut && !u->isLifted())
      {
        //we found the building
        ts->setBuildUnit(u);
        break;
      }
  }

}
void BasicTaskExecutor::computeIsExecuting(TaskStream* ts)
{
  taskStreams[ts].isExecuting = false;
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  if (type == TaskTypes::Unit)
  {
    UnitType ut=ts->getTask().getUnit();
    if (ts->getBuildUnit()!=NULL && ts->getBuildUnit()->exists() && (ts->getBuildUnit()->getType() == ut || ts->getBuildUnit()->getBuildType() == ut))
      taskStreams[ts].isExecuting = true;
    if (Broodwar->getFrameCount()>ts->getWorker()->getLastOrderFrame()+5)
    {
      if (worker->getType()==UnitTypes::Terran_SCV && worker->isConstructing()==false)
        worker->rightClick(ts->getBuildUnit());
    }
  }
  else if (type == TaskTypes::Tech)
  {
    if (worker->isResearching() && worker->getTech()==ts->getTask().getTech())
      taskStreams[ts].isExecuting = true;
  }
  else if (type == TaskTypes::Upgrade)
  {
    if (worker->isUpgrading() && worker->getUpgrade()==ts->getTask().getUpgrade())
      taskStreams[ts].isExecuting = true;
  }
  if (taskStreams[ts].isExecuting) ts->getTask().setSpentResources(true);
}
void BasicTaskExecutor::computeIsCompleted(TaskStream* ts)
{
  if (ts->getTask().isCompleted()) return;
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  if (type == TaskTypes::Unit)
  {
    if (ts->getBuildUnit()!=NULL &&
        ts->getBuildUnit()->exists() &&
        ts->getBuildUnit()->isCompleted() &&
        ts->getBuildUnit()->getType() == ts->getTask().getUnit())
    {
      ts->getTask().setCompleted(true);
    }
  }
  else if (type == TaskTypes::Tech)
  {
    if (Broodwar->self()->hasResearched(ts->getTask().getTech()))
      ts->getTask().setCompleted(true);
  }
  else if (type == TaskTypes::Upgrade)
  {
    if (taskStreams[ts].targetLevel>0 && Broodwar->self()->getUpgradeLevel(ts->getTask().getUpgrade()) >= taskStreams[ts].targetLevel)
      ts->getTask().setCompleted(true);
  }
}
void BasicTaskExecutor::computeIsReady(TaskStream* ts)
{
  if (taskStreams[ts].isReady) return;
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  taskStreams[ts].isReady = !(worker->isResearching() || worker->isUpgrading() || worker->isTraining() || worker->isConstructing() || worker->isMorphing());
}
void BasicTaskExecutor::getReady(TaskStream* ts)
{
  if (taskStreams[ts].isReady) return;
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  if (worker->isResearching() || worker->isUpgrading() || worker->isTraining() || worker->isConstructing() || worker->isMorphing())
    worker->stop();
}
void BasicTaskExecutor::execute(TaskStream* ts)
{
  if (ts->getTask().isCompleted()) return;
  if (taskStreams[ts].isExecuting) return;
  if (taskStreams[ts].isReady==false) return;
  Unit* worker = ts->getWorker();
  TaskTypes::Enum type = ts->getTask().getType();
  if (Broodwar->getFrameCount()<ts->getWorker()->getLastOrderFrame()+5) return;

  if (type == TaskTypes::Unit)
  {
    UnitType ut=ts->getTask().getUnit();
    if (ut.getRace()==Races::Zerg && ut.isBuilding()==ut.whatBuilds().first.isBuilding())
    {
      if (worker->morph(ut))
      {
        if (ts->getTask().hasSpentResources()==false)
        {
          ts->getTask().setSpentResources(true);
          TheMacroManager->spentResources+=ts->getTask().getResources();
        }
      }

    }
    else if (ut.isAddon())
    {
      if (worker->isLifted()==false)
      {
        if (worker->getTilePosition()!=ts->getTask().getTilePosition())
        {
          worker->lift();
        }
        else
        {
          if (worker->buildAddon(ut))
          {
            if (ts->getTask().hasSpentResources()==false)
            {
              ts->getTask().setSpentResources(true);
              TheMacroManager->spentResources+=ts->getTask().getResources();
            }
          }
        }
      }
      else
      {
        worker->land(ts->getTask().getTilePosition());
      }
    }
    else if (ut.isBuilding())
    {
      Position targetPosition=Position(ts->getTask().getTilePosition());
      targetPosition.x()+=ut.tileWidth()*16;
      targetPosition.y()+=ut.tileHeight()*16;
      if (worker->getDistance(targetPosition)>100)
        worker->rightClick(targetPosition);
      else
      {
        if (worker->build(ts->getTask().getTilePosition(),ut))
        {
          if (ts->getTask().hasSpentResources()==false)
          {
            ts->getTask().setSpentResources(true);
            TheMacroManager->spentResources+=ts->getTask().getResources();
          }
        }
      }
    }
    else
    {
      if (worker->train(ut))
      {
        if (ts->getTask().hasSpentResources()==false)
        {
          ts->getTask().setSpentResources(true);
          TheMacroManager->spentResources+=ts->getTask().getResources();
        }
      }
    }
  }
  else if (type == TaskTypes::Tech)
  {
    if (worker->research(ts->getTask().getTech()))
    {
      if (ts->getTask().hasSpentResources()==false)
      {
        ts->getTask().setSpentResources(true);
        TheMacroManager->spentResources+=ts->getTask().getResources();
      }
    }
  }
  else if (type == TaskTypes::Upgrade)
  {
    if (worker->upgrade(ts->getTask().getUpgrade()))
    {
      if (ts->getTask().hasSpentResources()==false)
      {
        ts->getTask().setSpentResources(true);
        taskStreams[ts].targetLevel = Broodwar->self()->getUpgradeLevel(ts->getTask().getUpgrade())+1;
        TheMacroManager->spentResources+=ts->getTask().getResources();
      }
    }
  }
}