#include <BasicWorkerFinder.h>
#include <Arbitrator.h>
#include <BWAPI.h>
#include <MacroManager.h>
using namespace BWAPI;
void BasicWorkerFinder::update(TaskStream* ts)
{
  newStatus(ts);
}
void BasicWorkerFinder::attached(TaskStream* ts)
{
  this->ts = ts;
  newStatus(ts);
}
void BasicWorkerFinder::newStatus(TaskStream* ts)
{
  if (ts->isWorkerReady()==false)
  {
    if (ts->getTask(0).getType()==TaskTypes::Unit && ts->getTask(0).getUnit().isBuilding() && (!ts->getTask(0).getTilePosition().isValid()))
      return;
    if (ts->getBuildUnit()!=NULL && ts->getBuildUnit()->exists() && ts->getBuildUnit()->getType().isBuilding() && ts->getBuildUnit()->getType().getRace()==Races::Protoss)
      return;

    std::set<BWAPI::Unit*> units;
    for each(Unit* u in Broodwar->self()->getUnits())
    {
      if (u->exists() && u->isCompleted() && u->getRemainingBuildTime()==0 && u->getType()==ts->getTask(0).getWorkerType() && u->isLoaded()==false)
      {
        if (ts->getTask(0).getType()==TaskTypes::Unit && ts->getTask(0).getUnit().isAddon() && u->getAddon()!=NULL)
          continue;
        if (TheArbitrator->hasBid(u) && TheArbitrator->getHighestBidder(u).second>=100.0)
          continue;
        if (TheMacroManager->getTaskStream(u)!=NULL)
          continue;

        units.insert(u);
      }
    }
    Unit* chosenWorker = NULL;
    if (ts->getTask(0).getTilePosition().isValid())
    {
      //if the task has a tile position, choose the worker closest to it
      Position p=Position(ts->getTask(0).getTilePosition());
      double minDist=-1;
      for each(Unit* u in units)
      {
        double d = u->getDistance(p);
        if (minDist < 0 || d<minDist)
        {
          minDist = d;
          chosenWorker = u;
        }
      }
    }
    else
    {
      //otherwise just choose the first worker we find
      if (units.empty()==false)
        chosenWorker = (*units.begin());
    }
    if (units.size()>0)
      chosenWorker = (*units.begin());
    ts->setWorker(chosenWorker);
    if (chosenWorker!=NULL)
    {
      if (ts->getTask(0).getType()==TaskTypes::Unit && ts->getTask(0).getUnit().isAddon())
        ts->getTask(0).setTilePosition(chosenWorker->getTilePosition());
    }
  }
}
std::string BasicWorkerFinder::getName() const
{
  return "Basic Worker Finder";
}
std::string BasicWorkerFinder::getShortName() const
{
  return "BWF";
}