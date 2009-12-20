#pragma once
#include <BWAPI.h>

class UnitGroup : public std::set<BWAPI::Unit*>
{
  public:
    UnitGroup operator+(const UnitGroup& other) const;//union
    UnitGroup operator*(const UnitGroup& other) const;//intersection
    UnitGroup operator^(const UnitGroup& other) const;//symmetric difference
    UnitGroup operator-(const UnitGroup& other) const;//difference
    bool attackMove(BWAPI::Position position);
    bool attackUnit(BWAPI::Unit* target);
    bool rightClick(BWAPI::Position position);
    bool rightClick(BWAPI::Unit* target);
    bool train(BWAPI::UnitType type);
    bool build(BWAPI::TilePosition position, BWAPI::UnitType type);
    bool buildAddon(BWAPI::UnitType type);
    bool research(BWAPI::TechType tech);
    bool upgrade(BWAPI::UpgradeType upgrade);
    bool stop();
    bool holdPosition();
    bool patrol(BWAPI::Position position);
    bool follow(BWAPI::Unit* target);
    bool setRallyPosition(BWAPI::Position target);
    bool setRallyUnit(BWAPI::Unit* target);
    bool repair(BWAPI::Unit* target);
    bool morph(BWAPI::UnitType type);
    bool burrow();
    bool unburrow();
    bool siege();
    bool unsiege();
    bool cloak();
    bool decloak();
    bool lift();
    bool land(BWAPI::TilePosition position);
    bool load(BWAPI::Unit* target);
    bool unload(BWAPI::Unit* target);
    bool unloadAll();
    bool unloadAll(BWAPI::Position position);
    bool cancelConstruction();
    bool haltConstruction();
    bool cancelMorph();
    bool cancelTrain();
    bool cancelTrain(int slot);
    bool cancelAddon();
    bool cancelResearch();
    bool cancelUpgrade();
    bool useTech(BWAPI::TechType tech);
    bool useTech(BWAPI::TechType tech, BWAPI::Position position);
    bool useTech(BWAPI::TechType tech, BWAPI::Unit* target);
};

