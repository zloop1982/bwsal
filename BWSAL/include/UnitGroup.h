#pragma once
#include <BWAPI.h>
#include <BWTA.h>
enum Filter
{
  exists = 1,
  isAccelerating,
  isBeingConstructed,
  isBeingGathered,
  isBeingHealed,
  isBlind,
  isBraking,
  isBurrowed,
  isCarryingGas,
  isCarryingMinerals,
  isCloaked,
  isCompleted,
  isConstructing,
  isDefenseMatrixed,
  isEnsnared,
  isFollowing,
  isGatheringGas,
  isGatheringMinerals,
  isHallucination,
  isIdle,
  isIrradiated,
  isLifted,
  isLoaded,
  isLockedDown,
  isMaelstrommed,
  isMorphing,
  isMoving,
  isParasited,
  isPatrolling,
  isPlagued,
  isRepairing,
  isResearching,
  isSelected,
  isSieged,
  isStartingAttack,
  isStasised,
  isStimmed,
  isTraining,
  isUnderStorm,
  isUnpowered,
  isUpgrading,
  isVisible,
  canProduce,
  canAttack,
  canMove,
  isFlyer,
  regeneratesHP,
  isSpellcaster,
  hasPermanentCloak,
  isInvincible,
  isOrganic,
  isMechanical,
  isRobotic,
  isDetector,
  isResourceContainer,
  isResourceDepot,
  isRefinery,
  isWorker,
  requiresPsi,
  requiresCreep,
  isTwoUnitsInOneEgg,
  isBurrowable,
  isCloakable,
  isBuilding,
  isAddon,
  isFlyingBuilding,
  isNeutral,
  Terran_Marine,
  Terran_Ghost,
  Terran_Vulture,
  Terran_Goliath,
  Terran_Siege_Tank_Tank_Mode,
  Terran_SCV,
  Terran_Wraith,
  Terran_Science_Vessel,
  Terran_Dropship,
  Terran_Battlecruiser,
  Terran_Vulture_Spider_Mine,
  Terran_Nuclear_Missile,
  Terran_Siege_Tank_Siege_Mode,
  Terran_Firebat,
  Spell_Scanner_Sweep,
  Terran_Medic,
  Zerg_Larva,
  Zerg_Egg,
  Zerg_Zergling,
  Zerg_Hydralisk,
  Zerg_Ultralisk,
  Zerg_Broodling,
  Zerg_Drone,
  Zerg_Overlord,
  Zerg_Mutalisk,
  Zerg_Guardian,
  Zerg_Queen,
  Zerg_Defiler,
  Zerg_Scourge,
  Zerg_Infested_Terran,
  Terran_Valkyrie,
  Zerg_Cocoon,
  Protoss_Corsair,
  Protoss_Dark_Templar,
  Zerg_Devourer,
  Protoss_Dark_Archon,
  Protoss_Probe,
  Protoss_Zealot,
  Protoss_Dragoon,
  Protoss_High_Templar,
  Protoss_Archon,
  Protoss_Shuttle,
  Protoss_Scout,
  Protoss_Arbiter,
  Protoss_Carrier,
  Protoss_Interceptor,
  Protoss_Reaver,
  Protoss_Observer,
  Protoss_Scarab,
  Critter_Rhynadon,
  Critter_Bengalaas,
  Critter_Scantid,
  Critter_Kakaru,
  Critter_Ragnasaur,
  Critter_Ursadon,
  Zerg_Lurker_Egg,
  Zerg_Lurker,
  Spell_Disruption_Web,
  Terran_Command_Center,
  Terran_Comsat_Station,
  Terran_Nuclear_Silo,
  Terran_Supply_Depot,
  Terran_Refinery,
  Terran_Barracks,
  Terran_Academy,
  Terran_Factory,
  Terran_Starport,
  Terran_Control_Tower,
  Terran_Science_Facility,
  Terran_Covert_Ops,
  Terran_Physics_Lab,
  Terran_Machine_Shop,
  Terran_Engineering_Bay,
  Terran_Armory,
  Terran_Missile_Turret,
  Terran_Bunker,
  Special_Crashed_Norad_II,
  Special_Ion_Cannon,
  Zerg_Infested_Command_Center,
  Zerg_Hatchery,
  Zerg_Lair,
  Zerg_Hive,
  Zerg_Nydus_Canal,
  Zerg_Hydralisk_Den,
  Zerg_Defiler_Mound,
  Zerg_Greater_Spire,
  Zerg_Queens_Nest,
  Zerg_Evolution_Chamber,
  Zerg_Ultralisk_Cavern,
  Zerg_Spire,
  Zerg_Spawning_Pool,
  Zerg_Creep_Colony,
  Zerg_Spore_Colony,
  Zerg_Sunken_Colony,
  Special_Overmind_With_Shell,
  Special_Overmind,
  Zerg_Extractor,
  Special_Mature_Chrysalis,
  Special_Cerebrate,
  Special_Cerebrate_Daggoth,
  Protoss_Nexus,
  Protoss_Robotics_Facility,
  Protoss_Pylon,
  Protoss_Assimilator,
  Protoss_Observatory,
  Protoss_Gateway,
  Protoss_Photon_Cannon,
  Protoss_Citadel_of_Adun,
  Protoss_Cybernetics_Core,
  Protoss_Templar_Archives,
  Protoss_Forge,
  Protoss_Stargate,
  Special_Stasis_Cell_Prison,
  Protoss_Fleet_Beacon,
  Protoss_Arbiter_Tribunal,
  Protoss_Robotics_Support_Bay,
  Protoss_Shield_Battery,
  Special_Khaydarin_Crystal_Form,
  Special_Protoss_Temple,
  Special_XelNaga_Temple,
  Resource_Mineral_Field,
  Resource_Vespene_Geyser,
  Special_Warp_Gate,
  Special_Psi_Disrupter,
  Special_Power_Generator,
  Special_Overmind_Cocoon,
  Spell_Dark_Swarm,
  None,
  Unknown
};

class UnitGroup : public std::set<BWAPI::Unit*>
{
  public:
    UnitGroup operator+(const UnitGroup& other) const;//union
    UnitGroup operator*(const UnitGroup& other) const;//intersection
    UnitGroup operator^(const UnitGroup& other) const;//symmetric difference
    UnitGroup operator-(const UnitGroup& other) const;//difference
    BWAPI::Position getCenter() const;
    UnitGroup withinRadius(double radius,BWAPI::Position position) const;
    UnitGroup withinRegion(BWTA::Region* region) const;
    UnitGroup operator()(int f1) const;
    UnitGroup operator()(int f1, int f2) const;
    UnitGroup operator()(int f1, int f2, int f3) const;
    UnitGroup operator()(int f1, int f2, int f3, int f4) const;
    UnitGroup operator()(int f1, int f2, int f3, int f4, int f5) const;
    UnitGroup not(int f1) const;
    UnitGroup not(int f1, int f2) const;
    UnitGroup not(int f1, int f2, int f3) const;
    UnitGroup not(int f1, int f2, int f3, int f4) const;
    UnitGroup not(int f1, int f2, int f3, int f4, int f5) const;
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

