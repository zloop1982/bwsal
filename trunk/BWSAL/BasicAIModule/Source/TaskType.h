#pragma once
#include <BWAPI.h>

namespace BWAPI
{
  class TaskType
  {
    public:
      TaskType();
      TaskType(int id);
      TaskType(const TaskType& other);
      TaskType(const TechType& other);
      TaskType(const UnitType& other);
      TaskType(const UpgradeType& other);
      TaskType& operator=(const TaskType& other);
      TaskType& operator=(const TechType& other);
      TaskType& operator=(const UnitType& other);
      TaskType& operator=(const UpgradeType& other);
      bool operator==(const TaskType& other) const;
      bool operator==(const TechType& other) const;
      bool operator==(const UnitType& other) const;
      bool operator==(const UpgradeType& other) const;
      bool operator!=(const TaskType& other) const;
      bool operator!=(const TechType& other) const;
      bool operator!=(const UnitType& other) const;
      bool operator!=(const UpgradeType& other) const;
      bool operator<(const TaskType& other) const;
      bool isTechType() const;
      bool isUnitType() const;
      bool isUpgradeType() const;
      TechType toTechType() const;
      UnitType toUnitType() const;
      UpgradeType toUpgradeType() const;

      /** Returns a unique ID for this task type. */
      int getID() const;

      /** Returns the name of the task. */
      std::string getName() const;

      /** Returns the race that the task type belongs to. For example TaskTypes::Terran_SCV.getRace() will return
       * Races::Terran. */
      Race getRace() const;

      /** Returns what builds this task type. The second number will usually be 1 unless the unit type is
       * Protoss_Archon or Protoss_Dark_Archon. Task types that cannot be created, such as critters and mineral
       * fields, will return a pair where the task type is TaskTypes::None, and the second component is 0.
       *
       * Example: TaskTypes::Terran_Marine.whatBuilds() will return an std::pair, where the first component
       * is TaskTypes::Terran_Barracks. */
      const std::pair< TaskType, int > whatExecutes() const;

      /** Returns the task types the player is required to have before it can perform the given task type.
       *
       * Example: TaskTypes::Terran_Battlecruiser.requiredUnits() will return a map of three keys:
       * TaskTypes::Terran_Starport, TaskTypes::Terran_Control_Tower, and TaskTypes::Terran_Physics_Lab. */
      const std::map< TaskType, int >& requiredTypes() const;

      /** Returns the mineral price of the task.
       *
       * Example: TaskTypes::Siege_Tank_Tank_Mode.mineralPrice() returns 150. */
      int mineralPrice(int level = 1) const;

      /** UnitTypes::Siege_Tank_Tank_Mode.gasPrice() returns 100. */
      int gasPrice(int level = 1) const;

      /** Returns the number of frames needed to execute this task type. */
      int executionTime(int level = 1) const;

      /** Returns the amount of supply used by this unit. Supply counts returned by BWAPI are double what you
       *  would expect to see from playing the game. This is because zerglings take up 0.5 in-game supply. */
      int supplyRequired() const;

      /** Returns the amount of supply produced by this unit (i.e. for a Protoss_Pylon). Supply counts
       * returned by BWAPI are double what you would expect to see from playing the game. This is because
       * zerglings take up 0.5 in-game supply. */
      int supplyProvided() const;
    private:
      int id;
  };

namespace TaskTypes
{
    extern const TaskType Terran_Marine;
    extern const TaskType Hero_Jim_Raynor_Marine;
    extern const TaskType Terran_Ghost;
    extern const TaskType Hero_Sarah_Kerrigan;
    extern const TaskType Hero_Samir_Duran;
    extern const TaskType Hero_Infested_Duran;
    extern const TaskType Hero_Alexei_Stukov;
    extern const TaskType Terran_Vulture;
    extern const TaskType Hero_Jim_Raynor_Vulture;
    extern const TaskType Terran_Goliath;
    extern const TaskType Hero_Alan_Schezar;
    extern const TaskType Terran_Siege_Tank_Tank_Mode;
    extern const TaskType Hero_Edmund_Duke_Tank_Mode;
    extern const TaskType Terran_SCV;
    extern const TaskType Terran_Wraith;
    extern const TaskType Hero_Tom_Kazansky;
    extern const TaskType Terran_Science_Vessel;
    extern const TaskType Hero_Magellan;
    extern const TaskType Terran_Dropship;
    extern const TaskType Terran_Battlecruiser;
    extern const TaskType Hero_Arcturus_Mengsk;
    extern const TaskType Hero_Hyperion;
    extern const TaskType Hero_Norad_II;
    extern const TaskType Hero_Gerard_DuGalle;
    extern const TaskType Terran_Vulture_Spider_Mine;
    extern const TaskType Terran_Nuclear_Missile;
    extern const TaskType Terran_Siege_Tank_Siege_Mode;
    extern const TaskType Hero_Edmund_Duke_Siege_Mode;
    extern const TaskType Terran_Firebat;
    extern const TaskType Hero_Gui_Montag;
    extern const TaskType Spell_Scanner_Sweep;
    extern const TaskType Terran_Medic;
    extern const TaskType Terran_Civilian;
    extern const TaskType Zerg_Larva;
    extern const TaskType Zerg_Egg;
    extern const TaskType Zerg_Zergling;
    extern const TaskType Hero_Devouring_One;
    extern const TaskType Hero_Infested_Kerrigan;
    extern const TaskType Zerg_Hydralisk;
    extern const TaskType Hero_Hunter_Killer;
    extern const TaskType Zerg_Ultralisk;
    extern const TaskType Hero_Torrasque;
    extern const TaskType Zerg_Broodling;
    extern const TaskType Zerg_Drone;
    extern const TaskType Zerg_Overlord;
    extern const TaskType Hero_Yggdrasill;
    extern const TaskType Zerg_Mutalisk;
    extern const TaskType Hero_Kukulza_Mutalisk;
    extern const TaskType Zerg_Guardian;
    extern const TaskType Hero_Kukulza_Guardian;
    extern const TaskType Zerg_Queen;
    extern const TaskType Hero_Matriarch;
    extern const TaskType Zerg_Defiler;
    extern const TaskType Hero_Unclean_One;
    extern const TaskType Zerg_Scourge;
    extern const TaskType Zerg_Infested_Terran;
    extern const TaskType Terran_Valkyrie;
    extern const TaskType Zerg_Cocoon;
    extern const TaskType Protoss_Corsair;
    extern const TaskType Hero_Raszagal;
    extern const TaskType Protoss_Dark_Templar;
    extern const TaskType Hero_Dark_Templar;
    extern const TaskType Hero_Zeratul;
    extern const TaskType Zerg_Devourer;
    extern const TaskType Protoss_Dark_Archon;
    extern const TaskType Protoss_Probe;
    extern const TaskType Protoss_Zealot;
    extern const TaskType Hero_Fenix_Zealot;
    extern const TaskType Protoss_Dragoon;
    extern const TaskType Hero_Fenix_Dragoon;
    extern const TaskType Protoss_High_Templar;
    extern const TaskType Hero_Tassadar;
    extern const TaskType Hero_Aldaris;
    extern const TaskType Protoss_Archon;
    extern const TaskType Hero_Tassadar_Zeratul_Archon;
    extern const TaskType Protoss_Shuttle;
    extern const TaskType Protoss_Scout;
    extern const TaskType Hero_Mojo;
    extern const TaskType Hero_Artanis;
    extern const TaskType Protoss_Arbiter;
    extern const TaskType Hero_Danimoth;
    extern const TaskType Protoss_Carrier;
    extern const TaskType Hero_Gantrithor;
    extern const TaskType Protoss_Interceptor;
    extern const TaskType Protoss_Reaver;
    extern const TaskType Hero_Warbringer;
    extern const TaskType Protoss_Observer;
    extern const TaskType Protoss_Scarab;
    extern const TaskType Critter_Rhynadon;
    extern const TaskType Critter_Bengalaas;
    extern const TaskType Critter_Scantid;
    extern const TaskType Critter_Kakaru;
    extern const TaskType Critter_Ragnasaur;
    extern const TaskType Critter_Ursadon;
    extern const TaskType Zerg_Lurker_Egg;
    extern const TaskType Zerg_Lurker;
    extern const TaskType Spell_Disruption_Web;
    extern const TaskType Terran_Command_Center;
    extern const TaskType Terran_Comsat_Station;
    extern const TaskType Terran_Nuclear_Silo;
    extern const TaskType Terran_Supply_Depot;
    extern const TaskType Terran_Refinery;
    extern const TaskType Terran_Barracks;
    extern const TaskType Terran_Academy;
    extern const TaskType Terran_Factory;
    extern const TaskType Terran_Starport;
    extern const TaskType Terran_Control_Tower;
    extern const TaskType Terran_Science_Facility;
    extern const TaskType Terran_Covert_Ops;
    extern const TaskType Terran_Physics_Lab;
    extern const TaskType Terran_Machine_Shop;
    extern const TaskType Terran_Engineering_Bay;
    extern const TaskType Terran_Armory;
    extern const TaskType Terran_Missile_Turret;
    extern const TaskType Terran_Bunker;
    extern const TaskType Special_Crashed_Norad_II;
    extern const TaskType Special_Ion_Cannon;
    extern const TaskType Zerg_Infested_Command_Center;
    extern const TaskType Zerg_Hatchery;
    extern const TaskType Zerg_Lair;
    extern const TaskType Zerg_Hive;
    extern const TaskType Zerg_Nydus_Canal;
    extern const TaskType Zerg_Hydralisk_Den;
    extern const TaskType Zerg_Defiler_Mound;
    extern const TaskType Zerg_Greater_Spire;
    extern const TaskType Zerg_Queens_Nest;
    extern const TaskType Zerg_Evolution_Chamber;
    extern const TaskType Zerg_Ultralisk_Cavern;
    extern const TaskType Zerg_Spire;
    extern const TaskType Zerg_Spawning_Pool;
    extern const TaskType Zerg_Creep_Colony;
    extern const TaskType Zerg_Spore_Colony;
    extern const TaskType Zerg_Sunken_Colony;
    extern const TaskType Special_Overmind_With_Shell;
    extern const TaskType Special_Overmind;
    extern const TaskType Zerg_Extractor;
    extern const TaskType Special_Mature_Chrysalis;
    extern const TaskType Special_Cerebrate;
    extern const TaskType Special_Cerebrate_Daggoth;
    extern const TaskType Protoss_Nexus;
    extern const TaskType Protoss_Robotics_Facility;
    extern const TaskType Protoss_Pylon;
    extern const TaskType Protoss_Assimilator;
    extern const TaskType Protoss_Observatory;
    extern const TaskType Protoss_Gateway;
    extern const TaskType Protoss_Photon_Cannon;
    extern const TaskType Protoss_Citadel_of_Adun;
    extern const TaskType Protoss_Cybernetics_Core;
    extern const TaskType Protoss_Templar_Archives;
    extern const TaskType Protoss_Forge;
    extern const TaskType Protoss_Stargate;
    extern const TaskType Special_Stasis_Cell_Prison;
    extern const TaskType Protoss_Fleet_Beacon;
    extern const TaskType Protoss_Arbiter_Tribunal;
    extern const TaskType Protoss_Robotics_Support_Bay;
    extern const TaskType Protoss_Shield_Battery;
    extern const TaskType Special_Khaydarin_Crystal_Form;
    extern const TaskType Special_Protoss_Temple;
    extern const TaskType Special_XelNaga_Temple;
    extern const TaskType Resource_Mineral_Field;
    extern const TaskType Resource_Vespene_Geyser;
    extern const TaskType Special_Warp_Gate;
    extern const TaskType Special_Psi_Disrupter;
    extern const TaskType Special_Power_Generator;
    extern const TaskType Special_Overmind_Cocoon;
    extern const TaskType Special_Zerg_Beacon;
    extern const TaskType Special_Terran_Beacon;
    extern const TaskType Special_Protoss_Beacon;
    extern const TaskType Special_Zerg_Flag_Beacon;
    extern const TaskType Special_Terran_Flag_Beacon;
    extern const TaskType Special_Protoss_Flag_Beacon;
    extern const TaskType Spell_Dark_Swarm;
    extern const TaskType Powerup_Uraj_Crystal;
    extern const TaskType Powerup_Khalis_Crystal;
    extern const TaskType Powerup_Flag;
    extern const TaskType Powerup_Young_Chrysalis;
    extern const TaskType Powerup_Psi_Emitter;
    extern const TaskType Powerup_Data_Disk;
    extern const TaskType Powerup_Khaydarin_Crystal;
    extern const TaskType Terran_Infantry_Armor;
    extern const TaskType Terran_Vehicle_Plating;
    extern const TaskType Terran_Ship_Plating;
    extern const TaskType Zerg_Carapace;
    extern const TaskType Zerg_Flyer_Carapace;
    extern const TaskType Protoss_Ground_Armor;
    extern const TaskType Protoss_Air_Armor;
    extern const TaskType Terran_Infantry_Weapons;
    extern const TaskType Terran_Vehicle_Weapons;
    extern const TaskType Terran_Ship_Weapons;
    extern const TaskType Zerg_Melee_Attacks;
    extern const TaskType Zerg_Missile_Attacks;
    extern const TaskType Zerg_Flyer_Attacks;
    extern const TaskType Protoss_Ground_Weapons;
    extern const TaskType Protoss_Air_Weapons;
    extern const TaskType Protoss_Plasma_Shields;
    extern const TaskType U_238_Shells;
    extern const TaskType Ion_Thrusters;
    extern const TaskType Titan_Reactor;
    extern const TaskType Ocular_Implants;
    extern const TaskType Moebius_Reactor;
    extern const TaskType Apollo_Reactor;
    extern const TaskType Colossus_Reactor;
    extern const TaskType Ventral_Sacs;
    extern const TaskType Antennae;
    extern const TaskType Pneumatized_Carapace;
    extern const TaskType Metabolic_Boost;
    extern const TaskType Adrenal_Glands;
    extern const TaskType Muscular_Augments;
    extern const TaskType Grooved_Spines;
    extern const TaskType Gamete_Meiosis;
    extern const TaskType Metasynaptic_Node;
    extern const TaskType Singularity_Charge;
    extern const TaskType Leg_Enhancements;
    extern const TaskType Scarab_Damage;
    extern const TaskType Reaver_Capacity;
    extern const TaskType Gravitic_Drive;
    extern const TaskType Sensor_Array;
    extern const TaskType Gravitic_Boosters;
    extern const TaskType Khaydarin_Amulet;
    extern const TaskType Apial_Sensors;
    extern const TaskType Gravitic_Thrusters;
    extern const TaskType Carrier_Capacity;
    extern const TaskType Khaydarin_Core;
    extern const TaskType Argus_Jewel;
    extern const TaskType Argus_Talisman;
    extern const TaskType Caduceus_Reactor;
    extern const TaskType Chitinous_Plating;
    extern const TaskType Anabolic_Synthesis;
    extern const TaskType Charon_Boosters;
    extern const TaskType Stim_Packs;
    extern const TaskType Lockdown;
    extern const TaskType EMP_Shockwave;
    extern const TaskType Spider_Mines;
    extern const TaskType Scanner_Sweep;
    extern const TaskType Tank_Siege_Mode;
    extern const TaskType Defensive_Matrix;
    extern const TaskType Irradiate;
    extern const TaskType Yamato_Gun;
    extern const TaskType Cloaking_Field;
    extern const TaskType Personnel_Cloaking;
    extern const TaskType Burrowing;
    extern const TaskType Infestation;
    extern const TaskType Spawn_Broodlings;
    extern const TaskType Dark_Swarm;
    extern const TaskType Plague;
    extern const TaskType Consume;
    extern const TaskType Ensnare;
    extern const TaskType Parasite;
    extern const TaskType Psionic_Storm;
    extern const TaskType Hallucination;
    extern const TaskType Recall;
    extern const TaskType Stasis_Field;
    extern const TaskType Archon_Warp;
    extern const TaskType Restoration;
    extern const TaskType Disruption_Web;
    extern const TaskType Mind_Control;
    extern const TaskType Dark_Archon_Meld;
    extern const TaskType Feedback;
    extern const TaskType Optical_Flare;
    extern const TaskType Maelstrom;
    extern const TaskType Lurker_Aspect;
    extern const TaskType Healing;
    extern const TaskType None;
    extern const TaskType Unknown;
    extern const TaskType Nuclear_Strike;
}