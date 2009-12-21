#include <UnitGroup.h>

using namespace BWAPI;
using namespace std;

bool passesint(Unit* u, int f)
{
  if (f<0)
    return !passesint(u,-f);
  switch(f)
  {
    case exists:
      if (u->exists()) return true;
    break;
    case isAccelerating:
      if (u->isAccelerating()) return true;
    break;
    case isBeingConstructed:
      if (u->isBeingConstructed()) return true;
    break;
    case isBeingGathered:
      if (u->isBeingGathered()) return true;
    break;
    case isBeingHealed:
      if (u->isBeingHealed()) return true;
    break;
    case isBlind:
      if (u->isBlind()) return true;
    break;
    case isBraking:
      if (u->isBraking()) return true;
    break;
    case isBurrowed:
      if (u->isBurrowed()) return true;
    break;
    case isCarryingGas:
      if (u->isCarryingGas()) return true;
    break;
    case isCarryingMinerals:
      if (u->isCarryingMinerals()) return true;
    break;
    case isCloaked:
      if (u->isCloaked()) return true;
    break;
    case isCompleted:
      if (u->isCompleted()) return true;
    break;
    case isConstructing:
      if (u->isConstructing()) return true;
    break;
    case isDefenseMatrixed:
      if (u->isDefenseMatrixed()) return true;
    break;
    case isEnsnared:
      if (u->isEnsnared()) return true;
    break;
    case isFollowing:
      if (u->isFollowing()) return true;
    break;
    case isGatheringGas:
      if (u->isGatheringGas()) return true;
    break;
    case isGatheringMinerals:
      if (u->isGatheringMinerals()) return true;
    break;
    case isHallucination:
      if (u->isHallucination()) return true;
    break;
    case isIdle:
      if (u->isIdle()) return true;
    break;
    case isIrradiated:
      if (u->isIrradiated()) return true;
    break;
    case isLifted:
      if (u->isLifted()) return true;
    break;
    case isLoaded:
      if (u->isLoaded()) return true;
    break;
    case isLockedDown:
      if (u->isLockedDown()) return true;
    break;
    case isMaelstrommed:
      if (u->isMaelstrommed()) return true;
    break;
    case isMorphing:
      if (u->isMorphing()) return true;
    break;
    case isMoving:
      if (u->isMoving()) return true;
    break;
    case isParasited:
      if (u->isParasited()) return true;
    break;
    case isPatrolling:
      if (u->isPatrolling()) return true;
    break;
    case isPlagued:
      if (u->isPlagued()) return true;
    break;
    case isRepairing:
      if (u->isRepairing()) return true;
    break;
    case isResearching:
      if (u->isResearching()) return true;
    break;
    case isSelected:
      if (u->isSelected()) return true;
    break;
    case isSieged:
      if (u->isSieged()) return true;
    break;
    case isStartingAttack:
      if (u->isStartingAttack()) return true;
    break;
    case isStasised:
      if (u->isStasised()) return true;
    break;
    case isStimmed:
      if (u->isStimmed()) return true;
    break;
    case isTraining:
      if (u->isTraining()) return true;
    break;
    case isUnderStorm:
      if (u->isUnderStorm()) return true;
    break;
    case isUnpowered:
      if (u->isUnpowered()) return true;
    break;
    case isUpgrading:
      if (u->isUpgrading()) return true;
    break;
    case isVisible:
      if (u->isVisible()) return true;
    break;
    case canProduce:
      if (u->getType().canProduce()) return true;
    break;
    case canAttack:
      if (u->getType().canAttack()) return true;
    break;
    case canMove:
      if (u->getType().canMove()) return true;
    break;
    case isFlyer:
      if (u->getType().isFlyer()) return true;
    break;
    case regeneratesHP:
      if (u->getType().regeneratesHP()) return true;
    break;
    case isSpellcaster:
      if (u->getType().isSpellcaster()) return true;
    break;
    case hasPermanentCloak:
      if (u->getType().hasPermanentCloak()) return true;
    break;
    case isInvincible:
      if (u->getType().isInvincible()) return true;
    break;
    case isOrganic:
      if (u->getType().isOrganic()) return true;
    break;
    case isMechanical:
      if (u->getType().isMechanical()) return true;
    break;
    case isRobotic:
      if (u->getType().isRobotic()) return true;
    break;
    case isDetector:
      if (u->getType().isDetector()) return true;
    break;
    case isResourceContainer:
      if (u->getType().isResourceContainer()) return true;
    break;
    case isResourceDepot:
      if (u->getType().isResourceDepot()) return true;
    break;
    case isRefinery:
      if (u->getType().isRefinery()) return true;
    break;
    case isWorker:
      if (u->getType().isWorker()) return true;
    break;
    case requiresPsi:
      if (u->getType().requiresPsi()) return true;
    break;
    case requiresCreep:
      if (u->getType().requiresCreep()) return true;
    break;
    case isTwoUnitsInOneEgg:
      if (u->getType().isTwoUnitsInOneEgg()) return true;
    break;
    case isBurrowable:
      if (u->getType().isBurrowable()) return true;
    break;
    case isCloakable:
      if (u->getType().isCloakable()) return true;
    break;
    case isBuilding:
      if (u->getType().isBuilding()) return true;
    break;
    case isAddon:
      if (u->getType().isAddon()) return true;
    break;
    case isFlyingBuilding:
      if (u->getType().isFlyingBuilding()) return true;
    break;
    case isNeutral:
      if (u->getType().isNeutral()) return true;
    break;
  }
  return false;
}

UnitGroup UnitGroup::operator+(const UnitGroup& other) const
{
  UnitGroup result=*this;
  for(set<Unit*>::const_iterator i=other.begin();i!=other.end();i++)
  {
    result.insert(*i);
  }
  return result;
}
UnitGroup UnitGroup::operator*(const UnitGroup& other) const
{
  UnitGroup result=*this;
  set<Unit*>::iterator i2;
  for(set<Unit*>::iterator i=result.begin();i!=result.end();i=i2)
  {
    i2=i;
    i2++;
    if (other.find(*i)==other.end())
      result.erase(*i);
  }
  return result;
}
UnitGroup UnitGroup::operator^(const UnitGroup& other) const
{
  UnitGroup result=*this;
  set<Unit*>::const_iterator i2;
  for(set<Unit*>::const_iterator i=other.begin();i!=other.end();i=i2)
  {
    i2=i;
    i2++;
    if (result.find(*i)==result.end())
      result.insert(*i);
    else
      result.erase(*i);
  }
  return result;
}
UnitGroup UnitGroup::operator-(const UnitGroup& other) const
{
  UnitGroup result=*this;
  for(set<Unit*>::const_iterator i=other.begin();i!=other.end();i++)
  {
    result.erase(*i);
  }
  return result;
}

UnitGroup UnitGroup::operator()(int f1) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (passesint(*i,f1))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::operator()(int f1, int f2) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (passesint(*i,f1) || passesint(*i,f2))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::operator()(int f1, int f2, int f3) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (passesint(*i,f1) || passesint(*i,f2) || passesint(*i,f3))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::operator()(int f1, int f2, int f3, int f4) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (passesint(*i,f1) || passesint(*i,f2) || passesint(*i,f3) || passesint(*i,f4))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::operator()(int f1, int f2, int f3, int f4, int f5) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (passesint(*i,f1) || passesint(*i,f2) || passesint(*i,f3) || passesint(*i,f4) || passesint(*i,f5))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::not(int f1) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (!(passesint(*i,f1)))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::not(int f1, int f2) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (!(passesint(*i,f1) || passesint(*i,f2)))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::not(int f1, int f2, int f3) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (!(passesint(*i,f1) || passesint(*i,f2) || passesint(*i,f3)))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::not(int f1, int f2, int f3, int f4) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (!(passesint(*i,f1) || passesint(*i,f2) || passesint(*i,f3) || passesint(*i,f4)))
      result.insert(*i);
  }
  return result;
}

UnitGroup UnitGroup::not(int f1, int f2, int f3, int f4, int f5) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if (!(passesint(*i,f1) || passesint(*i,f2) || passesint(*i,f3) || passesint(*i,f4) || passesint(*i,f5)))
      result.insert(*i);
  }
  return result;
}

Position UnitGroup::getCenter() const
{
  if (this->empty())
    return Positions::None;
  if (this->size()==1)
    return ((*this->begin())->getPosition());
  int count=0;
  double x=0;
  double y=0;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    Position p((*i)->getPosition());
    if (p!=Positions::None && p!=Positions::Unknown)
    {
      count++;
      x+=p.x();
      y+=p.y();
    }
  }
  if (count==0)
  {
    return Positions::None;
  }
  return Position((int)(x/count),(int)(y/count));
}

UnitGroup UnitGroup::withinRadius(double radius,BWAPI::Position position) const
{
  UnitGroup result;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    if ((*i)->getDistance(position)<=radius)
      result.insert(*i);
  }
  return result;
}

bool UnitGroup::attackMove(Position position) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->attackMove(position);
  }
  return retval;
}
bool UnitGroup::attackUnit(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->attackUnit(target);
  }
  return retval;
}
bool UnitGroup::rightClick(Position position) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->rightClick(position);
  }
  return retval;
}
bool UnitGroup::rightClick(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->rightClick(target);
  }
  return retval;
}
bool UnitGroup::train(UnitType type) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->train(type);
  }
  return retval;
}
bool UnitGroup::build(TilePosition position, UnitType type) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->build(position,type);
  }
  return retval;
}
bool UnitGroup::buildAddon(UnitType type) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->buildAddon(type);
  }
  return retval;
}
bool UnitGroup::research(TechType tech) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->research(tech);
  }
  return retval;
}
bool UnitGroup::upgrade(UpgradeType upgrade) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->upgrade(upgrade);
  }
  return retval;
}
bool UnitGroup::stop() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->stop();
  }
  return retval;
}
bool UnitGroup::holdPosition() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->holdPosition();
  }
  return retval;
}
bool UnitGroup::patrol(Position position) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->patrol(position);
  }
  return retval;
}
bool UnitGroup::follow(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->follow(target);
  }
  return retval;
}
bool UnitGroup::setRallyPosition(Position target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->setRallyPosition(target);
  }
  return retval;
}
bool UnitGroup::setRallyUnit(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->setRallyUnit(target);
  }
  return retval;
}
bool UnitGroup::repair(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->repair(target);
  }
  return retval;
}
bool UnitGroup::morph(UnitType type) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->morph(type);
  }
  return retval;
}
bool UnitGroup::burrow() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->burrow();
  }
  return retval;
}
bool UnitGroup::unburrow() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->unburrow();
  }
  return retval;
}
bool UnitGroup::siege() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->siege();
  }
  return retval;
}
bool UnitGroup::unsiege() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->unsiege();
  }
  return retval;
}
bool UnitGroup::cloak() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cloak();
  }
  return retval;
}
bool UnitGroup::decloak() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->decloak();
  }
  return retval;
}
bool UnitGroup::lift() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->lift();
  }
  return retval;
}
bool UnitGroup::land(TilePosition position) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->land(position);
  }
  return retval;
}
bool UnitGroup::load(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->load(target);
  }
  return retval;
}
bool UnitGroup::unload(Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->unload(target);
  }
  return retval;
}
bool UnitGroup::unloadAll() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->unloadAll();
  }
  return retval;
}
bool UnitGroup::unloadAll(Position position) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->unloadAll(position);
  }
  return retval;
}
bool UnitGroup::cancelConstruction() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelConstruction();
  }
  return retval;
}
bool UnitGroup::haltConstruction() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->haltConstruction();
  }
  return retval;
}
bool UnitGroup::cancelMorph() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelMorph();
  }
  return retval;
}
bool UnitGroup::cancelTrain() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelTrain();
  }
  return retval;
}
bool UnitGroup::cancelTrain(int slot) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelTrain(slot);
  }
  return retval;
}
bool UnitGroup::cancelAddon() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelAddon();
  }
  return retval;
}
bool UnitGroup::cancelResearch() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelResearch();
  }
  return retval;
}
bool UnitGroup::cancelUpgrade() const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->cancelUpgrade();
  }
  return retval;
}
bool UnitGroup::useTech(TechType tech) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->useTech(tech);
  }
  return retval;
}
bool UnitGroup::useTech(TechType tech, Position position) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->useTech(tech,position);
  }
  return retval;
}
bool UnitGroup::useTech(TechType tech, Unit* target) const
{
  bool retval=true;
  for(set<Unit*>::const_iterator i=this->begin();i!=this->end();i++)
  {
    retval = retval && (*i)->useTech(tech,target);
  }
  return retval;
}