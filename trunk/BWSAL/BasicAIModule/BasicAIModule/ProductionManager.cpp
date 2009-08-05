#include "ProductionManager.h"

ProductionManager::ProductionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  this->arbitrator = arbitrator;
}
void ProductionManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end();i++)
  {
    if ((*i)->getType().canProduce())
    {
      factories[(*i)->getType()].push_back(*i);
      arbitrator->accept(this,*i);
    }
    else
    {
      arbitrator->decline(this,*i,0);
    }
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
  {
    if ((*u)->getType().canProduce() && (*u)->isCompleted())
    {
      arbitrator->setBid(this, *u, 50);
    }
  }
  for(std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> >::iterator l=this->factoriesQueues.begin();l!=this->factoriesQueues.end();l++)
  {
    if (!l->second.empty() && BWAPI::Broodwar->canMake(NULL,l->second.front()))
    {
      for(std::list<BWAPI::Unit*>::iterator f=this->factories[l->first].begin();f!=this->factories[l->first].end();f++)
      {
        if (!(*f)->isTraining() && BWAPI::Broodwar->canMake(*f,l->second.front()))
        {
          (*f)->train(l->second.front());
          l->second.pop_front();
          if (l->second.empty()) break;
        }
      }
    }
  }
}
std::string ProductionManager::getName()
{
  return "Production Manager";
}
void ProductionManager::onRemoveUnit(BWAPI::Unit* unit)
{
  if (factories.find(unit->getType())!=factories.end())
  {
    factories[unit->getType()].remove(unit);
  }
}
bool ProductionManager::train(BWAPI::UnitType type)
{
  if (!type.whatBuilds().first->isBuilding() || !type.whatBuilds().first->canProduce())
    return false;
  factoriesQueues[*type.whatBuilds().first].push_back(type);
  return true;
}