#include "SupplyManager.h"

SupplyManager::SupplyManager(ConstructionManager* builder)
{
  this->builder=builder;
  this->plannedSupply=BWAPI::Broodwar->self()->supplyTotal();
  this->lastFrameCheck=0;
}
void SupplyManager::onRemoveUnit(BWAPI::Unit* unit)
{
  this->plannedSupply-=unit->getType().supplyProvided();
}

void SupplyManager::update()
{
  if (BWAPI::Broodwar->getFrameCount()>lastFrameCheck+25)
  {
    int productionCapacity=0;
    lastFrameCheck=BWAPI::Broodwar->getFrameCount();
    std::set<BWAPI::Unit*> units=BWAPI::Broodwar->self()->getUnits();
    for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end();i++)
    {
      if ((*i)->getType().canProduce())
      {
        productionCapacity+=2;
      }
    }
    if (this->plannedSupply<=BWAPI::Broodwar->self()->supplyUsed()+productionCapacity)
    {
      this->builder->build(*BWAPI::Broodwar->self()->getRace().getSupplyProvider());
      this->plannedSupply+=BWAPI::Broodwar->self()->getRace().getSupplyProvider()->supplyProvided();
    }
  }
}

std::string SupplyManager::getName()
{
  return "Supply Manager";
}
