#include <ProductionManager.h>

ProductionManager::ProductionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer)
{
  this->arbitrator = arbitrator;
  this->placer = placer;
}

void ProductionManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end();i++)
  {
    if ((*i)->getType().canProduce())
    {
      factories[(*i)->getType()].push_back(*i);
      factoryBuildUnit[*i].unit=NULL;
      factoryBuildUnit[*i].type=BWAPI::UnitTypes::None;
      arbitrator->accept(this,*i);
      factoriesQueues[(*i)->getType()];
    }
    else
      arbitrator->decline(this,*i,0);
  }
}

void ProductionManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  onRemoveUnit(unit);
}

void ProductionManager::update()
{
  std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
  for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
    if ((*u)->getType().canProduce() && (*u)->isCompleted())
      if ((*u)->getType().getRace()!=BWAPI::Races::Zerg || (*u)->getType()==BWAPI::UnitTypes::Zerg_Infested_Command_Center)
        arbitrator->setBid(this, *u, 50);
  for(std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> >::iterator l=this->factoriesQueues.begin();l!=this->factoriesQueues.end();l++)
  {
    for(std::list<BWAPI::Unit*>::iterator f=this->factories[l->first].begin();f!=this->factories[l->first].end();f++)
    {
      if ((*f)->isLifted())
      {
        if ((*f)->isIdle())
          (*f)->land(placer->getBuildLocationNear((*f)->getTilePosition()+BWAPI::TilePosition(0,1),l->first));
      }
      else
      {
        if ((*f)->isTraining() && (*f)->getBuildUnit()!=NULL)
          factoryBuildUnit[*f].unit=(*f)->getBuildUnit();
        if (factoryBuildUnit[*f].type==BWAPI::UnitTypes::None)
        {
          if (factoryBuildUnit[*f].unit!=NULL)
          {
            (*f)->cancelTrain();
            factoryBuildUnit[*f].unit=NULL;
          }
          else
          {
            if (!l->second.empty() && !(*f)->isTraining() && BWAPI::Broodwar->canMake(*f,l->second.front()))
            {
              factoryBuildUnit[*f].type=l->second.front();
              factoryBuildUnit[*f].lastAttemptFrame=-50;
              l->second.pop_front();
            }
          }
        }
        else
        {
          if (factoryBuildUnit[*f].unit==NULL)
          {
            if (BWAPI::Broodwar->getFrameCount()>factoryBuildUnit[*f].lastAttemptFrame+BWAPI::Broodwar->getLatency()*2)
              if (BWAPI::Broodwar->canMake(*f,factoryBuildUnit[*f].type))
              {
                (*f)->train(factoryBuildUnit[*f].type);
                factoryBuildUnit[*f].lastAttemptFrame=BWAPI::Broodwar->getFrameCount();
              }
          }
          else
          {
            if (factoryBuildUnit[*f].unit->isCompleted())
            {
              if (factoryBuildUnit[*f].unit->getType()==factoryBuildUnit[*f].type)
                factoryBuildUnit[*f].type=BWAPI::UnitTypes::None;
              factoryBuildUnit[*f].unit=NULL;
            }
            else
            {
              if (!(*f)->isTraining())
                factoryBuildUnit[*f].unit=NULL;
            }
          }
        }
      }
    }
  }
}

std::string ProductionManager::getName() const
{
  return "Production Manager";
}

void ProductionManager::onRemoveUnit(BWAPI::Unit* unit)
{
  if (factories.find(unit->getType())!=factories.end())
  {
    factories[unit->getType()].remove(unit);
    if (factoryBuildUnit.find(unit)!=factoryBuildUnit.end())
    {
      if (factoryBuildUnit[unit].unit!=NULL)
        factoriesQueues[unit->getType()].push_front(factoryBuildUnit[unit].type);
      factoryBuildUnit.erase(unit);
    }
  }
}

bool ProductionManager::train(BWAPI::UnitType type)
{
  if (!type.whatBuilds().first->isBuilding() || !type.whatBuilds().first->canProduce() || type.isBuilding())
    return false;
  factoriesQueues[*type.whatBuilds().first].push_back(type);
  return true;
}