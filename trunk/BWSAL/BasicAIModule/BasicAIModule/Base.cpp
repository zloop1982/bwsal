#include <BaseManager.h>
Base::Base(BWTA::BaseLocation* location)
{
  this->baseLocation=location;
  this->resourceDepot=NULL;
  this->active=false;
  this->beingConstructed=false;
}

BWTA::BaseLocation* Base::getBaseLocation() const
{
  return this->baseLocation;
}

BWAPI::Unit* Base::getResourceDepot() const
{
  return this->resourceDepot;
}

std::set<BWAPI::Unit*> Base::getMinerals() const
{
  std::set<BWAPI::Unit*> myMinerals;
  std::set<BWAPI::Unit*> allMinerals=BWAPI::Broodwar->getMinerals();
  BWAPI::Position position=this->baseLocation->getPosition();
  for(std::set<BWAPI::Unit*>::iterator m=allMinerals.begin();m!=allMinerals.end();m++)
  {
    if ((*m)->getPosition().getDistance(position)<32*10)
    {
      myMinerals.insert(*m);
    }
  }
  return myMinerals;
}

std::set<BWAPI::Unit*> Base::getGeysers() const
{
  std::set<BWAPI::Unit*> myGeysers;
  std::set<BWAPI::Unit*> allGeysers=BWAPI::Broodwar->getGeysers();
  BWAPI::Position position=this->baseLocation->getPosition();
  for(std::set<BWAPI::Unit*>::iterator g=allGeysers.begin();g!=allGeysers.end();g++)
  {
    if ((*g)->getPosition().getDistance(position)<32*10)
    {
      myGeysers.insert(*g);
    }
  }
  return myGeysers;
}
bool Base::isActive() const
{
  return this->active;
}
bool Base::isBeingConstructed() const
{
  return this->beingConstructed;
}

void Base::setResourceDepot(BWAPI::Unit* unit)
{
  this->resourceDepot=unit;
}
void Base::setActive(bool active)
{
  this->active=active;
}
