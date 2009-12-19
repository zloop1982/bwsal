#pragma once
#include <BWAPI.h>
#include <map>
class InformationManager
{
  public:
    InformationManager();
    void onUnitHide(BWAPI::Unit* unit);
    BWAPI::Player* getPlayer(BWAPI::Unit* unit);
    BWAPI::UnitType getType(BWAPI::Unit* unit);
    BWAPI::Position getLastPosition(BWAPI::Unit* unit);
  private:
    class UnitData
    {
      public:
        UnitData();
        BWAPI::Position position;
        BWAPI::UnitType type;
        BWAPI::Player* player;
    };
    std::map<BWAPI::Unit*, UnitData> savedData;
};