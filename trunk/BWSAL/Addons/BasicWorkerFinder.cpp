#include <BasicWorkerFinder.h>
#include <Arbitrator.h>
#include <BWAPI.h>
#include <MacroManager.h>
using namespace BWAPI;
void BasicWorkerFinder::onFrame(TaskStream* ts)
{
  /*
  for each(WorkBench* wb in ts->workBenches)
  {
    //don't look for a worker if we already have one
    if (wb->isWorkerReady())
      continue;

    //don't look for a worker unless we have a build location (if needed)
    if (wb->getTask().getType()==TaskTypes::Unit && wb->getTask().getUnit().isBuilding() && (!wb->isLocationReady()))
      continue;

    //don't look for a worker if we have a build unit as protoss
    if (wb->getBuildUnit()!=NULL && wb->getBuildUnit()->exists() && wb->getBuildUnit()->getType().isBuilding() && wb->getBuildUnit()->getType().getRace()==Races::Protoss)
      continue;

//Todo: translate into work benches
//    if (wb->getStartTime()<0 || wb->getStartTime()>Broodwar->getFrameCount()+20*24)
//      return;

    std::set<BWAPI::Unit*> units;
    for each(Unit* u in Broodwar->self()->getUnits())
    {
      if (u->exists() && u->isCompleted() && u->isLoaded()==false)
      {
        bool isCorrectType = false;
        if (wb->getTask().getWorkerType()==UnitTypes::Zerg_Larva)
          isCorrectType = u->getType().producesLarva();
        else
          isCorrectType = (u->getType()==wb->getTask().getWorkerType());
        if (isCorrectType)
        {
          if (wb->getTask().getType()==TaskTypes::Unit && wb->getTask().getUnit().isAddon() && u->getAddon()!=NULL)
            continue;
            if (TheArbitrator->hasBid(u) && TheArbitrator->getHighestBidder(u).second>=100.0 && TheArbitrator->getHighestBidder(u).first!=TheMacroManager)
            continue;
          if (TheMacroManager->getTaskStreams(u).empty()==false)
            continue;
          units.insert(u);
        }
      }
    }
    if (units.empty())
    {
      for each(Unit* u in Broodwar->self()->getUnits())
      {
        if (u->exists() && u->isCompleted() && u->isLoaded()==false)
        {
          bool isCorrectType = false;
          if (wb->getTask().getWorkerType()==UnitTypes::Zerg_Larva)
            isCorrectType = u->getType().producesLarva();
          else
            isCorrectType = (u->getType()==wb->getTask().getWorkerType());
          if (isCorrectType)
          {
            if (wb->getTask().getType()==TaskTypes::Unit && wb->getTask().getUnit().isAddon() && u->getAddon()!=NULL)
              continue;
            if (TheArbitrator->hasBid(u) && TheArbitrator->getHighestBidder(u).second>=100.0 && TheArbitrator->getHighestBidder(u).first!=TheMacroManager)
              continue;
            units.insert(u);
          }
        }
      }
    }
    Unit* chosenWorker = NULL;
    if (wb->getTask().getTilePosition().isValid())
    {
      //if the task has a tile position, choose the worker closest to it
      Position p=Position(wb->getTask().getTilePosition());
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
    wb->setWorker(chosenWorker);
    if (chosenWorker!=NULL)
    {
      if (wb->getTask().getType()==TaskTypes::Unit && wb->getTask().getUnit().isAddon())
        wb->getTask().setTilePosition(chosenWorker->getTilePosition());
    }
  }
  */
}
void BasicWorkerFinder::onAttach(TaskStream* ts)
{
  this->ts = ts;
  onFrame(ts);
}