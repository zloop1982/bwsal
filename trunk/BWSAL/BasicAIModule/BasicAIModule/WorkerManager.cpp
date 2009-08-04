#include <WorkerManager.h>
#include <BaseManager.h>
#include <algorithm>
WorkerManager::WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BaseManager* baseManager)
{
  this->arbitrator=arbitrator;
  this->baseManager=baseManager;
  this->lastSCVBalance=0;
}
void WorkerManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator u=units.begin();u!=units.end();u++)
  {
    if ((*u)->getType().isWorker())
    {
      arbitrator->accept(this,*u);
      WorkerData temp;
      this->desiredWorkerCount[this->mineralOrder[this->mineralOrderIndex].first]++;
      this->currentWorkers[this->mineralOrder[this->mineralOrderIndex].first].insert(*u);
      temp.newMineral=this->mineralOrder[this->mineralOrderIndex].first;
      this->mineralOrderIndex=(this->mineralOrderIndex+1) % mineralOrder.size();
      workers.insert(std::make_pair(*u,temp));

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

bool mineralCompare (const std::pair<BWAPI::Unit*, int> i, const std::pair<BWAPI::Unit*, int> j) { return (i.second>j.second); }

double distanceBetweenUnits(BWAPI::Unit* i, BWAPI::Unit* j)
{
  if (i == j)
    return 0;
  double result=0;
  if (i->getPosition().y() - i->getType().dimensionUp() <= j->getPosition().y() + j->getType().dimensionDown())
    if (i->getPosition().y() + i->getType().dimensionDown() >= j->getPosition().y() - j->getType().dimensionUp())
      if (i->getPosition().x() > j->getPosition().x())
        result = i->getPosition().x() - i->getType().dimensionLeft()  - j->getPosition().x() - j->getType().dimensionRight();
      else
        result = j->getPosition().x() - j->getType().dimensionRight() - i->getPosition().x() - i->getType().dimensionLeft();

  if (i->getPosition().x() - i->getType().dimensionLeft() <= j->getPosition().x() + j->getType().dimensionRight())
    if (i->getPosition().x() + i->getType().dimensionRight() >= j->getPosition().x() - j->getType().dimensionLeft())
      if (i->getPosition().y() > j->getPosition().y())
        result = i->getPosition().y() - i->getType().dimensionUp()   - j->getPosition().y() - j->getType().dimensionDown();
      else
        result = j->getPosition().y() - j->getType().dimensionDown() - i->getPosition().y() - i->getType().dimensionUp();

  if (i->getPosition().x() > j->getPosition().x())
  {
    if (i->getPosition().y() > j->getPosition().y())
      result = BWAPI::Position(i->getPosition().x() - i->getType().dimensionLeft(),
                               i->getPosition().y() - i->getType().dimensionUp()).getDistance(
               BWAPI::Position(j->getPosition().x() + j->getType().dimensionRight(),
                               j->getPosition().y() + j->getType().dimensionDown()));
    else
      result = BWAPI::Position(i->getPosition().x() - i->getType().dimensionLeft(),
                               i->getPosition().y() + i->getType().dimensionDown()).getDistance(
               BWAPI::Position(j->getPosition().x() + j->getType().dimensionRight(),
                               j->getPosition().y() - j->getType().dimensionUp()));
  }
  else
  {
    if (i->getPosition().y() > j->getPosition().y())
      result = BWAPI::Position(i->getPosition().x() + i->getType().dimensionRight(),
                               i->getPosition().y() - i->getType().dimensionUp()).getDistance(
               BWAPI::Position(j->getPosition().x() - j->getType().dimensionLeft(),
                               j->getPosition().y() + j->getType().dimensionDown()));
    else
      result = BWAPI::Position(i->getPosition().x() + i->getType().dimensionRight(),
                               i->getPosition().y() + i->getType().dimensionDown()).getDistance(
               BWAPI::Position(j->getPosition().x() - j->getType().dimensionLeft(),
                               j->getPosition().y() - j->getType().dimensionUp()));
  }
  if (result > 0)
    return result;
  return 0;
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
  std::set<Base*> bases=this->baseManager->getActiveBases();
  if (BWAPI::Broodwar->getFrameCount()>lastSCVBalance+10*25 || bases!=this->basesCache || lastSCVBalance==0)
  {
    this->basesCache=bases;
    lastSCVBalance=BWAPI::Broodwar->getFrameCount();
    mineralOrder.clear();
    desiredWorkerCount.clear();
    currentWorkers.clear();
    mineralBase.clear();
    std::set<Base*> bases=this->baseManager->getActiveBases();
    for(std::set<Base*>::iterator b=bases.begin();b!=bases.end();b++)
    {
      std::set<BWAPI::Unit*> baseMinerals=(*b)->getMinerals();
      for(std::set<BWAPI::Unit*>::iterator m=baseMinerals.begin();m!=baseMinerals.end();m++)
      {
        mineralBase[*m]=*b;
        desiredWorkerCount[*m]=0;
        currentWorkers[*m].clear();
        mineralOrder.push_back(std::make_pair(*m, (*m)->getResources()-2*(int)(*m)->getPosition().getDistance((*b)->getBaseLocation()->getPosition())));
      }
    }
    if (mineralBase.empty()) return;
    std::sort(mineralOrder.begin(),mineralOrder.end(),mineralCompare);
    int remainingWorkers=this->workers.size();
    mineralOrderIndex=0;
    int m=mineralOrder.size();
    while(remainingWorkers>0)
    {
      desiredWorkerCount[mineralOrder[mineralOrderIndex].first]++;
      remainingWorkers--;
      mineralOrderIndex=(mineralOrderIndex+1) % m;
    }
    for(std::map<BWAPI::Unit*,WorkerData >::iterator w=this->workers.begin();w!=this->workers.end();w++)
    {
      if (w->second.newMineral!=NULL)
      {
        if (mineralBase.find(w->second.newMineral)==mineralBase.end())
        {
          w->second.newMineral=NULL;
        }
        else
        {
          currentWorkers[w->second.newMineral].insert(w->first);
        }
      }
    }
    std::set<BWAPI::Unit*> freeWorkers;
    for(std::map<BWAPI::Unit*,WorkerData>::iterator w=this->workers.begin();w!=this->workers.end();w++)
    {
      if (w->second.newMineral==NULL)
      {
        freeWorkers.insert(w->first);
      }
    }

    for(std::map<BWAPI::Unit*,int>::iterator i=desiredWorkerCount.begin();i!=desiredWorkerCount.end();i++)
    {
      if (i->second<(int)currentWorkers[i->first].size())
      {
        //desired worker count is less than the current worker count for this resource, so lets remove some workers.
        int amountToRemove=currentWorkers[i->first].size()-i->second;
        for(int j=0;j<amountToRemove;j++)
        {
          BWAPI::Unit* worker=*currentWorkers[i->first].begin();
          freeWorkers.insert(worker);
          workers[worker].newMineral=NULL;
          currentWorkers[i->first].erase(worker);
        }
      }
    }

    for(std::map<BWAPI::Unit*,int>::iterator i=desiredWorkerCount.begin();i!=desiredWorkerCount.end();i++)
    {
      if (i->second>(int)currentWorkers[i->first].size())
      {
        //desired worker count is more than the current worker count for this resource, so lets assign some workers
        int amountToAdd=i->second-currentWorkers[i->first].size();
        for(int j=0;j<amountToAdd;j++)
        {
          BWAPI::Unit* worker=*freeWorkers.begin();
          freeWorkers.erase(worker);
          workers[worker].newMineral=i->first;
          currentWorkers[i->first].insert(worker);
        }
      }
    }
  }
  /*
  for(std::map<BWAPI::Unit*,int>::iterator i=desiredWorkerCount.begin();i!=desiredWorkerCount.end();i++)
  {
    int x=(*i).first->getPosition().x();
    int y=(*i).first->getPosition().y();
    BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,x,y,"Desired: %d",(*i).second);
    BWAPI::Broodwar->text(BWAPI::CoordinateType::Map,x,y+20,"Actual: %d",currentWorkers[(*i).first].size());
  }
  */
  for(std::map<BWAPI::Unit*,WorkerData >::iterator u=workers.begin();u!=workers.end();u++)
  {
    BWAPI::Unit* i=u->first;
    if (u->second.mineral==NULL || (i->getTarget()!=NULL && i->getTarget()->getType().isResourceDepot()))
    {
      u->second.mineral=u->second.newMineral;
    }
    BWAPI::Unit* mineral=u->second.mineral;
    BWAPI::Unit* miningBuddy=NULL;
    if ((
         i->getOrder() == BWAPI::Orders::MoveToMinerals || 
         i->getOrder() == BWAPI::Orders::WaitForMinerals || 
         i->getOrder() == BWAPI::Orders::PlayerGuard))
    {
      if (i->getTarget() != mineralBase[mineral]->getResourceDepot())
      {
        for(std::set<BWAPI::Unit*>::iterator b=currentWorkers[mineral].begin();b!=currentWorkers[mineral].end();b++)
        {
          if ((*b)!=i && (*b)->getOrder() ==BWAPI::Orders::MiningMinerals)
          {
            miningBuddy=*b;
          }
        }
        if (i->getTarget() != mineral ||
           (false && distanceBetweenUnits(i,mineral) <= BWAPI::Broodwar->getLatency()*3 &&
            miningBuddy != NULL &&
            miningBuddy->getOrderTimer() >= BWAPI::Broodwar->getLatency() &&
            (
              miningBuddy->getOrderTimer() == BWAPI::Broodwar->getLatency()-1 ||
              BWAPI::Broodwar->getFrameCount() - u->second.lastFrameSpam > 4
            )))
        {
          u->second.lastFrameSpam = BWAPI::Broodwar->getFrameCount();
          i->rightClick(mineral);
        }
      }
    }
  }
}
std::string WorkerManager::getName()
{
  return "Worker Manager";
}
