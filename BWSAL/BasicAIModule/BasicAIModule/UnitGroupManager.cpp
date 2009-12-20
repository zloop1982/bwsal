#include <UnitGroupManager.h>

std::map<BWAPI::Unit*,BWAPI::Player*> unitOwner;
std::map<BWAPI::Unit*,BWAPI::UnitType> unitType;
std::map<BWAPI::Player*, std::map<BWAPI::UnitType,UnitGroup > > data;
UnitGroup allUnits;

void UnitGroupManager::onUnitShow(BWAPI::Unit* unit)
{
  unitOwner[unit]=unit->getPlayer();
  unitType[unit]=unit->getType();
  data[unit->getPlayer()][unit->getType()].insert(unit);
  allUnits.insert(unit);
}
void UnitGroupManager::onUnitHide(BWAPI::Unit* unit)
{
  unitOwner[unit]=unit->getPlayer();
  unitType[unit]=unit->getType();
  data[unit->getPlayer()][unit->getType()].erase(unit);
  allUnits.erase(unit);
}
void UnitGroupManager::onUnitMorph(BWAPI::Unit* unit)
{
  data[unitOwner[unit]][unitType[unit]].erase(unit);
  unitType[unit]=unit->getType();
  unitOwner[unit]=unit->getPlayer();
  data[unit->getPlayer()][unit->getType()].insert(unit);
}
void UnitGroupManager::onUnitRenegade(BWAPI::Unit* unit)
{
  data[unitOwner[unit]][unitType[unit]].erase(unit);
  unitType[unit]=unit->getType();
  unitOwner[unit]=unit->getPlayer();
  data[unit->getPlayer()][unit->getType()].insert(unit);
}
UnitGroup AllUnits()
{
  return allUnits;
}
UnitGroup SelectAll(BWAPI::UnitType type)
{
  return data[BWAPI::Broodwar->self()][type];
}
UnitGroup SelectAllEnemy(BWAPI::UnitType type)
{
  return data[BWAPI::Broodwar->enemy()][type];
}
UnitGroup SelectAll(BWAPI::Player* player, BWAPI::UnitType type)
{
  return data[player][type];
}