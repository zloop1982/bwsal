#include <WorkerManager.h>
#include <BaseManager.h>
#include <algorithm>
WorkerManager::WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BaseManager* baseManager)
{
  this->arbitrator     = arbitrator;
  this->baseManager    = baseManager;
  this->lastSCVBalance = 0;
  this->WorkersPerGas  = 3;
}
void WorkerManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
  {
    if ((*u)->getType().isWorker())
    {
      arbitrator->accept(this, *u);
      WorkerData temp;
      this->desiredWorkerCount[this->mineralOrder[this->mineralOrderIndex].first]++;
      this->currentWorkers[this->mineralOrder[this->mineralOrderIndex].first].insert(*u);
      temp.newResource = this->mineralOrder[this->mineralOrderIndex].first;
      this->mineralOrderIndex = (this->mineralOrderIndex+1) % mineralOrder.size();
      workers.insert(std::make_pair(*u,temp));
    }
    else
      arbitrator->decline(this, *u, 0);
  }
}
void WorkerManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  this->onRemoveUnit(unit);
}

void WorkerManager::updateWorkerAssignments()
{
  //determine current worker assignments
  //also workers that are mining from resources that dont belong to any of our bases will be set to free

  for(std::map<BWAPI::Unit*,WorkerData >::iterator w = this->workers.begin(); w != this->workers.end(); w++)
  {
    if (w->second.newResource != NULL)
    {
      if (resourceBase.find(w->second.newResource) == resourceBase.end())
        w->second.newResource = NULL;
      else
        currentWorkers[w->second.newResource].insert(w->first);
    }
  }

  // get free workers
  std::set<BWAPI::Unit*> freeWorkers;
  for(std::map<BWAPI::Unit*,WorkerData>::iterator w = this->workers.begin(); w != this->workers.end(); w++)
    if (w->second.newResource == NULL)
      freeWorkers.insert(w->first);

  // free workers from resources with too many workers
  for(std::map<BWAPI::Unit*,int>::iterator i = desiredWorkerCount.begin(); i != desiredWorkerCount.end(); i++)
    if (i->second < (int)currentWorkers[i->first].size())
    {
      // desired worker count is less than the current worker count for this resource, so lets remove some workers.
      int amountToRemove = currentWorkers[i->first].size() - i->second;
      for(int j = 0; j < amountToRemove; j++)
      {
        BWAPI::Unit* worker = *currentWorkers[i->first].begin();
        freeWorkers.insert(worker);
        workers[worker].newResource = NULL;
        currentWorkers[i->first].erase(worker);
      }
    }

  // assign workers to resources that need more workers
  for(std::map<BWAPI::Unit*,int>::iterator i = desiredWorkerCount.begin(); i != desiredWorkerCount.end(); i++)
    if (i->second>(int)currentWorkers[i->first].size())
    {
      //desired worker count is more than the current worker count for this resource, so lets assign some workers
      int amountToAdd = i->second - currentWorkers[i->first].size();
      for(int j = 0; j < amountToAdd; j++)
      {
        BWAPI::Unit* worker = *freeWorkers.begin();
        freeWorkers.erase(worker);
        workers[worker].newResource = i->first;
        currentWorkers[i->first].insert(worker);
      }
    }
}

bool mineralCompare (const std::pair<BWAPI::Unit*, int> i, const std::pair<BWAPI::Unit*, int> j) { return (i.second>j.second); }

void WorkerManager::rebalanceWorkers()
{  
  mineralOrder.clear();
  desiredWorkerCount.clear();
  currentWorkers.clear();
  resourceBase.clear();
  int remainingWorkers = this->workers.size();
  
  // iterate over all the resources of each active base
  for(std::set<Base*>::iterator b = this->basesCache.begin(); b != this->basesCache.end(); b++)
  {
    std::set<BWAPI::Unit*> baseMinerals = (*b)->getMinerals();
    for(std::set<BWAPI::Unit*>::iterator m = baseMinerals.begin(); m != baseMinerals.end(); m++)
    {
      resourceBase[*m] = *b;
      desiredWorkerCount[*m] = 0;
      currentWorkers[*m].clear();
      mineralOrder.push_back(std::make_pair(*m, (*m)->getResources() - 2*(int)(*m)->getPosition().getDistance((*b)->getBaseLocation()->getPosition())));
    }
    std::set<BWAPI::Unit*> baseGeysers = (*b)->getGeysers();
    for(std::set<BWAPI::Unit*>::iterator g = baseGeysers.begin(); g != baseGeysers.end(); g++)
    {
      resourceBase[*g] = *b;
      desiredWorkerCount[*g]=0;
      if ((*g)->getType().isRefinery() && (*g)->getPlayer()==BWAPI::Broodwar->self() && (*g)->isCompleted())
      {
        for(int w=0;w<this->WorkersPerGas;w++)
        {
          desiredWorkerCount[*g]++;
          remainingWorkers--;
        }
      }
      currentWorkers[*g].clear();
    }
  }

  //if no resources exist, return
  if (resourceBase.empty())
    return;

  //order minerals by score (based on distance and resource amount)
  std::sort(mineralOrder.begin(), mineralOrder.end(), mineralCompare);

  //calculate optimal worker count for each mineral patch
  mineralOrderIndex = 0;
  while(remainingWorkers > 0)
  {
    desiredWorkerCount[mineralOrder[mineralOrderIndex].first]++;
    remainingWorkers--;
    mineralOrderIndex = (mineralOrderIndex + 1) % mineralOrder.size();
  }

  //update the worker assignments so the actual workers per resource is the same as the desired workers per resource
  updateWorkerAssignments();
}

void WorkerManager::update()
{
  //bid a constant value of 10 on all completed workers
  std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
  for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
  {
    if ((*u)->isCompleted() && (*u)->getType().isWorker())
    {
      arbitrator->setBid(this, *u, 10);
    }
  }

  //rebalance workers when necessary
  std::set<Base*> bases = this->baseManager->getActiveBases();
  if (BWAPI::Broodwar->getFrameCount() > lastSCVBalance + 5*25 || bases != this->basesCache || lastSCVBalance == 0)
  {
    this->basesCache = bases;
    lastSCVBalance   = BWAPI::Broodwar->getFrameCount();
    this->rebalanceWorkers();
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
  
  //order workers to gather from their assigned resources
  for(std::map<BWAPI::Unit*,WorkerData>::iterator u = workers.begin(); u != workers.end(); u++)
  {
    BWAPI::Unit* i = u->first;

    //switch current resource to newResource when appropiate
    if (u->second.resource == NULL || (i->getTarget() != NULL && i->getTarget()->getType().isResourceDepot()))
      u->second.resource = u->second.newResource;

    if (u->second.resource->getType().isRefinery())
    {
      //geyser micro
      BWAPI::Unit* geyser = u->second.resource;
      if ((
           i->getOrder() == BWAPI::Orders::MoveToMinerals || 
           i->getOrder() == BWAPI::Orders::WaitForMinerals || 
           i->getOrder() == BWAPI::Orders::MoveToGas || 
           i->getOrder() == BWAPI::Orders::WaitForGas || 
           i->getOrder() == BWAPI::Orders::PlayerGuard))
        if (i->getTarget() != resourceBase[geyser]->getResourceDepot())
          if (i->getTarget() != geyser)
            i->rightClick(geyser);
    }
    else
    {
      //mineral micro
      BWAPI::Unit* mineral = u->second.resource;
      BWAPI::Unit* miningBuddy = NULL;
      if ((
           i->isGatheringGas() ||
           i->getOrder() == BWAPI::Orders::MoveToMinerals || 
           i->getOrder() == BWAPI::Orders::WaitForMinerals || 
           i->getOrder() == BWAPI::Orders::MoveToGas || 
           i->getOrder() == BWAPI::Orders::WaitForGas || 
           i->getOrder() == BWAPI::Orders::PlayerGuard))
        if (i->getTarget() != resourceBase[mineral]->getResourceDepot())
        {
          for(std::set<BWAPI::Unit*>::iterator b = currentWorkers[mineral].begin(); b != currentWorkers[mineral].end(); b++)
            if ((*b) != i && (*b)->getOrder() == BWAPI::Orders::MiningMinerals)
              miningBuddy = *b;
          if (i->getTarget() != mineral ||
             (false && i->getDistance(mineral) <= BWAPI::Broodwar->getLatency()*3 &&
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
std::string WorkerManager::getName() const
{
  return "Worker Manager";
}

void WorkerManager::onRemoveUnit(BWAPI::Unit* unit)
{
  workers.erase(unit);
}

void WorkerManager::setWorkersPerGas(int count)
{
  this->WorkersPerGas=count;
}