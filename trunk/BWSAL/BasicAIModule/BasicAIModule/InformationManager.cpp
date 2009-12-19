#include <InformationManager.h>
InformationManager::InformationManager()
{
}
void InformationManager::onUnitHide(BWAPI::Unit* unit)
{
  savedData[unit].player=unit->getPlayer();
  savedData[unit].type=unit->getType();
  savedData[unit].position=unit->getPosition();
}

BWAPI::Player* InformationManager::getPlayer(BWAPI::Unit* unit)
{
  if (unit->exists())
    return unit->getPlayer();
  return savedData[unit].player;
}

BWAPI::UnitType InformationManager::getType(BWAPI::Unit* unit)
{
  if (unit->exists())
    return unit->getType();
  return savedData[unit].type;
}

BWAPI::Position InformationManager::getLastPosition(BWAPI::Unit* unit)
{
  if (unit->exists())
    return unit->getPosition();
  return savedData[unit].position;
}

InformationManager::UnitData::UnitData()
{
  position=BWAPI::Positions::Unknown;
  type=BWAPI::UnitTypes::Unknown;
  player=NULL;
}