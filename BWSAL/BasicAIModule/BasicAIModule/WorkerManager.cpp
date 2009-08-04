#include <WorkerManager.h>
#include <BaseManager.h>
#include <algorithm>
WorkerManager::WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BaseManager* baseManager)
{
  this->arbitrator=arbitrator;
  this->baseManager=baseManager;
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

bool mineralCompare (const BWAPI::Unit* i, const BWAPI::Unit*  j) { return (i->getResources()>j->getResources()); }
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
  std::vector<BWAPI::Unit*> mineralOrder;
  std::map<BWAPI::Unit*, int> mineralsWorkerCount;
  std::map<BWAPI::Unit*, Base*> mineralBase;
  std::set<Base*> bases=this->baseManager->getActiveBases();
  for(std::set<Base*>::iterator b=bases.begin();b!=bases.end();b++)
  {
    std::set<BWAPI::Unit*> baseMinerals=(*b)->getMinerals();
    for(std::set<BWAPI::Unit*>::iterator m=baseMinerals.begin();m!=baseMinerals.end();m++)
    {
      mineralBase[*m]=*b;
      mineralsWorkerCount[*m]=0;
      mineralOrder.push_back(*m);
    }
  }
  if (mineralBase.empty()) return;
  std::sort(mineralOrder.begin(),mineralOrder.end(),mineralCompare);
  int remainingWorkers=this->workers.size();
  int i=0;
  int m=mineralOrder.size();
  while(remainingWorkers>0)
  {
    mineralsWorkerCount[mineralOrder[i]]++;
    remainingWorkers--;
    i=(i+1) % m;
  }
  for(std::map<BWAPI::Unit*,int>::iterator i=mineralsWorkerCount.begin();i!=mineralsWorkerCount.end();i++)
  {
    int x=(*i).first->getPosition().x();
    int y=(*i).first->getPosition().y();
    BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,x,y,"%d workers",(*i).second);
  }

  // extremely stupid gather AI just as a place-holder for now
  // each worker is told to mine the nearest mineral patch to it.
  std::set<BWAPI::Unit*> minerals=BWAPI::Broodwar->getMinerals();
  for(std::set<BWAPI::Unit*>::iterator u=workers.begin();u!=workers.end();u++)
  {
    if ((*u)->isIdle())
    {
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
