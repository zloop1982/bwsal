#pragma once
#include <BWAPI.h>
#include <UnitGroup.h>
class UnitGroupManager
{
  public:
    void onUnitShow(BWAPI::Unit* unit);
    void onUnitHide(BWAPI::Unit* unit);
    void onUnitMorph(BWAPI::Unit* unit);
    void onUnitRenegade(BWAPI::Unit* unit);
};
UnitGroup AllUnits();
UnitGroup SelectAll(BWAPI::UnitType type);
UnitGroup SelectAllEnemy(BWAPI::UnitType type);
UnitGroup SelectAll(BWAPI::Player* player, BWAPI::UnitType type);