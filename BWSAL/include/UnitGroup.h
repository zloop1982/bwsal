#pragma once
#include <BWAPI.h>

class UnitGroup : public std::set<BWAPI::Unit*>
{
  public:
    UnitGroup operator+(const UnitGroup& other) const;//union
    UnitGroup operator*(const UnitGroup& other) const;//intersection
    UnitGroup operator^(const UnitGroup& other) const;//symmetric difference
    UnitGroup operator-(const UnitGroup& other) const;//difference
    BWAPI::Position getCenter() const;
    UnitGroup withinRadius(double radius,BWAPI::Position position) const;
    bool attackMove(BWAPI::Position position) const;
    bool attackUnit(BWAPI::Unit* target) const;
    bool rightClick(BWAPI::Position position) const;
    bool rightClick(BWAPI::Unit* target) const;
    bool train(BWAPI::UnitType type) const;
    bool build(BWAPI::TilePosition position, BWAPI::UnitType type) const;
    bool buildAddon(BWAPI::UnitType type) const;
    bool research(BWAPI::TechType tech) const;
    bool upgrade(BWAPI::UpgradeType upgrade) const;
    bool stop() const;
    bool holdPosition() const;
    bool patrol(BWAPI::Position position) const;
    bool follow(BWAPI::Unit* target) const;
    bool setRallyPosition(BWAPI::Position target) const;
    bool setRallyUnit(BWAPI::Unit* target) const;
    bool repair(BWAPI::Unit* target) const;
    bool morph(BWAPI::UnitType type) const;
    bool burrow() const;
    bool unburrow() const;
    bool siege() const;
    bool unsiege() const;
    bool cloak() const;
    bool decloak() const;
    bool lift() const;
    bool land(BWAPI::TilePosition position) const;
    bool load(BWAPI::Unit* target) const;
    bool unload(BWAPI::Unit* target) const;
    bool unloadAll() const;
    bool unloadAll(BWAPI::Position position) const;
    bool cancelConstruction() const;
    bool haltConstruction() const;
    bool cancelMorph() const;
    bool cancelTrain() const;
    bool cancelTrain(int slot) const;
    bool cancelAddon() const;
    bool cancelResearch() const;
    bool cancelUpgrade() const;
    bool useTech(BWAPI::TechType tech) const;
    bool useTech(BWAPI::TechType tech, BWAPI::Position position) const;
    bool useTech(BWAPI::TechType tech, BWAPI::Unit* target) const;
};

