#include "WorkerManager.h"

WorkerManager::WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  this->arbitrator=arbitrator;
}
void WorkerManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator u=units.begin();u!=units.end();u++)
  {
    if ((*u)->getType().isWorker())
    {
      arbitrator->accept(this,*u);
      workers.insert(*u);
    }
    else
    {
      arbitrator->decline(this,*u,0);
    }
  }
}
void WorkerManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  workers.erase(unit);
}
void WorkerManager::update()
{
  std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
  for(std::set<BWAPI::Unit*>::iterator u=myPlayerUnits.begin();u!=myPlayerUnits.end();u++)
  {
    if ((*u)->getType().isWorker())
    {
      arbitrator->setBid(this,*u,10);
    }
  }
  std::set<BWAPI::Unit*> minerals=BWAPI::Broodwar->getMinerals();
  for(std::set<BWAPI::Unit*>::iterator u=workers.begin();u!=workers.end();u++)
  {
    if ((*u)->isIdle())
    {
      // extremely stupid gather AI just as a place-holder for now
      // each worker is told to mine the nearest mineral patch to it.
      double minDist=1000000;
      BWAPI::Unit* closestField=NULL;
      for(std::set<BWAPI::Unit*>::iterator m=minerals.begin();m!=minerals.end();m++)
      {
        double dist=(*u)->getPosition().getDistance((*m)->getPosition());
        if (dist<minDist)
        {
          minDist=dist;
          closestField=*m;
        }
      }
      if (closestField!=NULL)
      {
        (*u)->rightClick(closestField);
      }
    }
  }
}
std::string WorkerManager::getName()
{
  return "Worker Manager";
}
