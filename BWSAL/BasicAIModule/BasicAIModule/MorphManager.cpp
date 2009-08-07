#include <MorphManager.h>

MorphManager::MorphManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  this->arbitrator = arbitrator;
}

void MorphManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end();i++)
  {
    std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> >::iterator q=morphQueues.find((*i)->getType());
    if (q!=morphQueues.end() && !q->second.empty())
    {
      morphingUnits.insert(std::make_pair(*i,q->second.front()));
      q->second.pop_front();
      arbitrator->accept(this,*i);
      arbitrator->setBid(this,*i,100.0);
    }
    else
      arbitrator->decline(this,*i,0);
  }
}

void MorphManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  std::map<BWAPI::Unit*,BWAPI::UnitType>::iterator m=morphingUnits.find(unit);
  if (m!=morphingUnits.end())
  {
    BWAPI::UnitType type=m->second;
    if (m->first->getType()!=m->second || m->first->isMorphing())
      morphQueues[*type.whatBuilds().first].push_front(type);
    morphingUnits.erase(m);
  }
}

void MorphManager::update()
{
  std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
  for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
  {
    std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> >::iterator q=morphQueues.find((*u)->getType());
    if (q!=morphQueues.end() && !q->second.empty())
      if (!q->second.empty())
        arbitrator->setBid(this, *u, 50);
  }
  std::map<BWAPI::Unit*,BWAPI::UnitType>::iterator i_next;
  for(std::map<BWAPI::Unit*,BWAPI::UnitType>::iterator i=morphingUnits.begin();i!=morphingUnits.end();i=i_next)
  {
    i_next=i;
    i_next++;
    if (!i->first->isMorphing())
    {
      if (i->first->getType()==i->second)
        morphingUnits.erase(i);
      else
        if (BWAPI::Broodwar->canMake(NULL,i->second))
          i->first->morph(i->second);
    }
  }
}

std::string MorphManager::getName() const
{
  return "Morph Manager";
}

void MorphManager::onRemoveUnit(BWAPI::Unit* unit)
{
  std::map<BWAPI::Unit*,BWAPI::UnitType>::iterator m=morphingUnits.find(unit);
  if (m!=morphingUnits.end())
  {
    BWAPI::UnitType type=m->second;
    morphQueues[*type.whatBuilds().first].push_front(type);
    morphingUnits.erase(m);
  }
}

bool MorphManager::morph(BWAPI::UnitType type)
{
  if (type.isBuilding()!=type.whatBuilds().first->isBuilding())
    return false;
  morphQueues[*type.whatBuilds().first].push_back(type);
  return true;
}