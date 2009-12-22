#include <WorkerManager.h>
#include <BaseManager.h>
#include <RectangleArray.h>
#include "Util.h"
#include <algorithm>
using namespace BWAPI;
using namespace std;
using namespace Util;
WorkerManager::WorkerManager(Arbitrator::Arbitrator<Unit*,double>* arbitrator)
{
  this->arbitrator     = arbitrator;
  this->baseManager    = NULL;
  this->lastSCVBalance = 0;
  this->WorkersPerGas  = 3;
  this->mineralRate    = 0;
  this->gasRate        = 0;
}
void WorkerManager::setBaseManager(BaseManager* baseManager)
{
  this->baseManager = baseManager;
}
void WorkerManager::onOffer(set<Unit*> units)
{
  for(set<Unit*>::iterator u = units.begin(); u != units.end(); u++)
  {
    if ((*u)->getType().isWorker() && !this->mineralOrder.empty())
    {
      arbitrator->accept(this, *u);
      WorkerData temp;
      this->desiredWorkerCount[this->mineralOrder[this->mineralOrderIndex].first]++;
      this->currentWorkers[this->mineralOrder[this->mineralOrderIndex].first].insert(*u);
      temp.newResource = this->mineralOrder[this->mineralOrderIndex].first;
      this->mineralOrderIndex = (this->mineralOrderIndex+1) % mineralOrder.size();
      workers.insert(make_pair(*u,temp));
    }
    else
      arbitrator->decline(this, *u, 0);
  }
}
void WorkerManager::onRevoke(Unit* unit, double bid)
{
  this->onRemoveUnit(unit);
}

map<int, int> makeAssignments(RectangleArray< double> &cost)
{
  map<int, int> assignments;
  vector<bool> deletedRows;
  vector<bool> deletedColumns;
  for(int r=0;r<(int)cost.getHeight();r++)
    deletedRows.push_back(false);
  for(int c=0;c<(int)cost.getWidth();c++)
    deletedColumns.push_back(false);

  bool outer_done=false;
  while(outer_done==false)
  {
    outer_done=true;
    bool done=false;
    while(done==false)
    {
      done=true;
      for(int r=0;r<(int)cost.getHeight();r++)
      {
        if (deletedRows[r]) continue;
        int firstZero=-1;
        int lastZero=-2;
        for(int c=0;c<(int)cost.getWidth();c++)
        {
          if (deletedColumns[c]) continue;
          if (cost[r][c]==0)
          {
            if (firstZero==-1)
              firstZero=c;
            lastZero=c;
          }
        }
        if (firstZero==lastZero)
        {
          assignments[r]=firstZero;
          deletedRows[r]=true;
          deletedColumns[firstZero]=true;
          done=false;
        }
      }
      for(int c=0;c<(int)cost.getWidth();c++)
      {
        if (deletedColumns[c]) continue;
        int firstZero=-1;
        int lastZero=-2;
        for(int r=0;r<(int)cost.getHeight();r++)
        {
          if (deletedRows[r]) continue;
          if (cost[r][c]==0)
          {
            if (firstZero==-1)
              firstZero=r;
            lastZero=r;
          }
        }
        if (firstZero==lastZero)
        {
          assignments[firstZero]=c;
          deletedRows[firstZero]=true;
          deletedColumns[c]=true;
          done=false;
        }
      }
    }
    for(int r=0;r<(int)cost.getHeight();r++)
    {
      if (deletedRows[r]) continue;
      for(int c=0;c<(int)cost.getWidth();c++)
      {
        if (deletedColumns[c]) continue;
        if (cost[r][c]==0)
        {
          assignments[r]=c;
          deletedRows[r]=true;
          deletedColumns[c]=true;
          outer_done=false;
          break;
        }
      }
      if (outer_done==false) break;
    }
  }
  return assignments;
}

// Hungarian Algorithm
// http://en.wikipedia.org/wiki/Hungarian_algorithm

map<int, int> computeAssignments(RectangleArray< double> &cost)
{
  if (cost.getWidth()==0 || cost.getHeight()==0)
  {
    map<int, int> nothing;
    return nothing;
  }
  //cost is an n by n matrix
  //Step 1: From each row subtract off the row min
  for(int r=0;r<(int)cost.getHeight();r++)
  {
    double minCost=cost[r][0];
    for(int c=0;c<(int)cost.getWidth();c++)
      if (cost[r][c]<minCost)
        minCost=cost[r][c];
    for(int c=0;c<(int)cost.getWidth();c++)
      cost[r][c]-=minCost;
  }

  //Step 2: From each column subtract off the column min
  for(int c=0;c<(int)cost.getWidth();c++)
  {
    double minCost=cost[0][c];
    for(int r=0;r<(int)cost.getHeight();r++)
      if (cost[r][c]<minCost)
        minCost=cost[r][c];
    for(int r=0;r<(int)cost.getHeight();r++)
      cost[r][c]-=minCost;
  }
  //Step 3: Use as few lines as possible to cover all zeros in the matrix
  for(;;)
  {
    //Assign as many task as possible
    map<int, int> assignments=makeAssignments(cost);
    if (assignments.size() == cost.getHeight())
    {
      return assignments;
    }
    vector<bool> markedRows;
    vector<bool> markedColumns;
    vector<bool> linedRows;
    vector<bool> linedColumns;
    for(int r=0;r<(int)cost.getHeight();r++)
    {
      markedRows.push_back(false);
      linedRows.push_back(false);
    }
    for(int c=0;c<(int)cost.getWidth();c++)
    {
      markedColumns.push_back(false);
      linedColumns.push_back(false);
    }
    //Mark all rows having no assignments
    for(int r=0;r<(int)cost.getHeight();r++)
    {
      if (assignments.find(r)==assignments.end())
      {
        markedRows[r]=true;
      }
    }
    bool done=false;
    while(done==false)
    {
      done=true;
      for(int r=0;r<(int)cost.getHeight();r++)
      {
        if (!markedRows[r]) continue;
        for(int c=0;c<(int)cost.getWidth();c++)
        {
          if (cost[r][c]==0)
          {
            if (!markedColumns[c]) done=false;
            markedColumns[c]=true;
          }
        }
      }
      for(int c=0;c<(int)cost.getWidth();c++)
      {
        if (!markedColumns[c]) continue;
        for(int r=0;r<(int)cost.getHeight();r++)
        {
          if (assignments.find(r)!=assignments.end() && assignments.find(r)->second==c)
          {
            if (!markedRows[r]) done=false;
            markedRows[r]=true;
          }
        }
      }
    }
    int k=0;
    for(int r=0;r<(int)cost.getHeight();r++)
    {
      linedRows[r]=!markedRows[r];
      if (linedRows[r])
        k++;
    }
    for(int c=0;c<(int)cost.getWidth();c++)
    {
      linedColumns[c]=markedColumns[c];
      if (linedColumns[c])
        k++;
    }
    if (k==cost.getWidth())
      break;

    //set m to the minimum uncovered number
    double m;
    bool setM=false;
    for(int r=0;r<(int)cost.getHeight();r++)
      for(int c=0;c<(int)cost.getWidth();c++)
        if (!linedRows[r] && !linedColumns[c])
          if (!setM || cost[r][c]<m)
          {
            m=cost[r][c];
            setM=true;
          }

    //Subtract m from every uncovered number
    for(int r=0;r<(int)cost.getHeight();r++)
      for(int c=0;c<(int)cost.getWidth();c++)
        if (!linedColumns[c] && !linedRows[r])
          cost[r][c]-=m;

    //Add m to every number covered with 2 lines
    for(int r=0;r<(int)cost.getHeight();r++)
      for(int c=0;c<(int)cost.getWidth();c++)
        if (linedColumns[c] && linedRows[r])
          cost[r][c]+=m;

  }
  map<int, int> assignments=makeAssignments(cost);
  return assignments;
}

void WorkerManager::updateWorkerAssignments()
{
  //determine current worker assignments
  //also workers that are mining from resources that dont belong to any of our bases will be set to free

  for(map<Unit*,WorkerData >::iterator w = this->workers.begin(); w != this->workers.end(); w++)
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
  set<Unit*> freeWorkers;
  for(map<Unit*,WorkerData>::iterator w = this->workers.begin(); w != this->workers.end(); w++)
  {
    if (w->second.newResource == NULL)
      freeWorkers.insert(w->first);
    else
    {
      // free workers that are too far away from their resources
      if (w->first->getDistance(w->second.newResource)>32*10)
      {
        freeWorkers.insert(w->first);
        //erase worker from resource's current workers set
        currentWorkers[w->second.newResource].erase(w->first);
      }
    }
  }

  // free workers from resources with too many workers
  for(map<Unit*,int>::iterator i = desiredWorkerCount.begin(); i != desiredWorkerCount.end(); i++)
    if (i->second < (int)currentWorkers[i->first].size())
    {
      // desired worker count is less than the current worker count for this resource, so lets remove some workers.
      int amountToRemove = currentWorkers[i->first].size() - i->second;
      for(int j = 0; j < amountToRemove; j++)
      {
        Unit* worker = *currentWorkers[i->first].begin();
        freeWorkers.insert(worker);
        workers[worker].newResource = NULL;
        currentWorkers[i->first].erase(worker);
      }
    }

  vector< Unit* > workerUnit;
  vector< Unit* > taskUnit;
  map<int,int> assignment;

  for(set<Unit*>::iterator i=freeWorkers.begin();i!=freeWorkers.end();i++)
    workerUnit.push_back(*i);

  // assign workers to resources that need more workers
  for(map<Unit*,int>::iterator i = desiredWorkerCount.begin(); i != desiredWorkerCount.end(); i++)
    if (i->second>(int)currentWorkers[i->first].size())
      for(int j=(int)currentWorkers[i->first].size();j<i->second;j++)
        taskUnit.push_back(i->first);

  //construct cost matrix
  //currently just uses euclidean distance, but walking distance would be more accurate
  RectangleArray<double> cost(workerUnit.size(),taskUnit.size());
  for(int w=0;w<(int)workerUnit.size();w++)
    for(int t=0;t<(int)taskUnit.size();t++)
      cost[w][t]=workerUnit[w]->getDistance(taskUnit[t]);

  //calculate assignment for workers and tasks (resources)
  assignment=computeAssignments(cost);

  //use assignment
  for(map<int,int>::iterator a=assignment.begin();a!=assignment.end();a++)
  {
    Unit* worker=workerUnit[a->first];
    Unit* resource=taskUnit[a->second];
    workers[worker].newResource = resource;
    currentWorkers[resource].insert(worker);
  }
}

bool mineralCompare (const pair<Unit*, int> i, const pair<Unit*, int> j) { return (i.second>j.second); }

void WorkerManager::rebalanceWorkers()
{
  mineralOrder.clear();
  desiredWorkerCount.clear();
  currentWorkers.clear();
  resourceBase.clear();
  int remainingWorkers = this->workers.size();
  
  // iterate over all the resources of each active base
  for(set<Base*>::iterator b = this->basesCache.begin(); b != this->basesCache.end(); b++)
  {
    set<Unit*> baseMinerals = (*b)->getMinerals();
    for(set<Unit*>::iterator m = baseMinerals.begin(); m != baseMinerals.end(); m++)
    {
      resourceBase[*m] = *b;
      desiredWorkerCount[*m] = 0;
      currentWorkers[*m].clear();
      mineralOrder.push_back(make_pair(*m, (*m)->getResources() - 2*(int)(*m)->getPosition().getDistance((*b)->getBaseLocation()->getPosition())));
    }
    set<Unit*> baseGeysers = (*b)->getGeysers();
    for(set<Unit*>::iterator g = baseGeysers.begin(); g != baseGeysers.end(); g++)
    {
      resourceBase[*g] = *b;
      desiredWorkerCount[*g]=0;
      if ((*g)->getType().isRefinery() && (*g)->getPlayer()==Broodwar->self() && (*g)->isCompleted())
      {
        for(int w=0;w<this->WorkersPerGas && remainingWorkers>0;w++)
        {
          desiredWorkerCount[*g]++;
          remainingWorkers--;
        }
      }
      currentWorkers[*g].clear();
    }
  }

  //if no resources exist, return
  if (!mineralOrder.empty())
  {

    //order minerals by score (based on distance and resource amount)
    sort(mineralOrder.begin(), mineralOrder.end(), mineralCompare);

    //calculate optimal worker count for each mineral patch
    mineralOrderIndex = 0;
    while(remainingWorkers > 0)
    {
      desiredWorkerCount[mineralOrder[mineralOrderIndex].first]++;
      remainingWorkers--;
      mineralOrderIndex = (mineralOrderIndex + 1) % mineralOrder.size();
    }
  }

  //update the worker assignments so the actual workers per resource is the same as the desired workers per resource
  updateWorkerAssignments();
}

void WorkerManager::update()
{
  //bid a constant value of 10 on all completed workers
  set<Unit*> myPlayerUnits=Broodwar->self()->getUnits();
  for(set<Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
  {
    if ((*u)->isCompleted() && (*u)->getType().isWorker())
    {
      arbitrator->setBid(this, *u, 10);
    }
  }

  //rebalance workers when necessary
  set<Base*> bases = this->baseManager->getActiveBases();
  if (Broodwar->getFrameCount() > lastSCVBalance + 5*24 || bases != this->basesCache || lastSCVBalance == 0)
  {
    this->basesCache = bases;
    lastSCVBalance   = Broodwar->getFrameCount();
    this->rebalanceWorkers();
  }
  
  //order workers to gather from their assigned resources
  this->mineralRate=0;
  this->gasRate=0;
  for(map<Unit*,WorkerData>::iterator u = workers.begin(); u != workers.end(); u++)
  {
    Unit* i = u->first;
    if (u->second.resource!=NULL)
    {
      if (u->second.resource->getType()==UnitTypes::Resource_Mineral_Field)
        mineralRate+=8/180.0;
      else
        gasRate+=8/180.0;
    }
    
    //switch current resource to newResource when appropiate
    if (u->second.resource == NULL || (i->getTarget() != NULL && !i->getTarget()->getType().isResourceDepot()))
      u->second.resource = u->second.newResource;

    Unit* resource = u->second.resource;
    if (i->getOrder() == Orders::MoveToMinerals || 
        i->getOrder() == Orders::WaitForMinerals || 
        i->getOrder() == Orders::MoveToGas || 
        i->getOrder() == Orders::WaitForGas || 
        i->getOrder() == Orders::PlayerGuard)
      if ((i->getTarget()==NULL || !i->getTarget()->exists() || !i->getTarget()->getType().isResourceDepot()) && i->getTarget() != resource)
        i->rightClick(resource);
  }
}
string WorkerManager::getName() const
{
  return "Worker Manager";
}

void WorkerManager::onRemoveUnit(Unit* unit)
{
  workers.erase(unit);
}

void WorkerManager::setWorkersPerGas(int count)
{
  this->WorkersPerGas=count;
}
double WorkerManager::getMineralRate() const
{
  return this->mineralRate;
}
double WorkerManager::getGasRate() const
{
  return this->gasRate;
}
