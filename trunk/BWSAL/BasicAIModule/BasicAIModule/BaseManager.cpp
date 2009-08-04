#include <BaseManager.h>
BaseManager::BaseManager(ConstructionManager* builder)
{
  this->builder=builder;
}

void BaseManager::update()
{
  for(std::set<Base*>::iterator b=this->allBases.begin();b!=this->allBases.end();b++)
  {
    if (!(*b)->isActive())
    {
      if ((*b)->getResourceDepot()==NULL)
      {
        BWAPI::TilePosition tile=(*b)->getBaseLocation()->getTilePosition();
        std::set<BWAPI::Unit*> units=BWAPI::Broodwar->unitsOnTile(tile.x(),tile.y());
        for(std::set<BWAPI::Unit*>::iterator u=units.begin();u!=units.end();u++)
        {
          if ((*u)->getPlayer()==BWAPI::Broodwar->self() && (*u)->getType().isResourceDepot())
          {
            (*b)->setResourceDepot(*u);
            break;
          }
        }
      }
      if ((*b)->getResourceDepot()!=NULL)
      {
        if ((*b)->getResourceDepot()->isCompleted())
        {
          (*b)->setActive(true);
        }
      }
    }
  }
}
void BaseManager::addBase(BWTA::BaseLocation* location)
{
  allBases.insert(new Base(location));
}
std::set<Base*> BaseManager::getActiveBases() const
{
  std::set<Base*> activeBases;
  for(std::set<Base*>::const_iterator b=this->allBases.begin();b!=this->allBases.end();b++)
  {
    if ((*b)->isActive())
    {
      activeBases.insert(*b);
    }
  }
  return activeBases;
}
std::set<Base*> BaseManager::getAllBases() const
{
  std::set<Base*> allBases;
  for(std::set<Base*>::const_iterator b=this->allBases.begin();b!=this->allBases.end();b++)
  {
    allBases.insert(*b);
  }
  return allBases;
}
std::string BaseManager::getName()
{
  return "Base Manager";
}
void BaseManager::onRemoveUnit(BWAPI::Unit* unit)
{
}