#pragma once
#include <BWAPI.h>
#include <Resources.h>
namespace TaskTypes
{
  enum Enum
  {
    Unit,
    Tech,
    Upgrade
  };
}
class Task
{
  public:
    Task(const BWAPI::UnitType &t = BWAPI::UnitTypes::None,    const BWAPI::TilePosition &p = BWAPI::TilePositions::None);
    Task(const BWAPI::TechType &t,                             const BWAPI::TilePosition &p = BWAPI::TilePositions::None);
    Task(const BWAPI::UpgradeType &t,                          const BWAPI::TilePosition &p = BWAPI::TilePositions::None);

    Task& operator=(const Task &t);
    Task& operator=(const BWAPI::UnitType &t);
    Task& operator=(const BWAPI::TechType &t);
    Task& operator=(const BWAPI::UpgradeType &t);
    Task& operator=(const BWAPI::TilePosition &p);

    bool operator==(void* ptr) const;
    bool operator==(const Task &t) const;
    bool operator==(const BWAPI::UnitType &t) const;
    bool operator==(const BWAPI::TechType &t) const;
    bool operator==(const BWAPI::UpgradeType &t) const;
    bool operator==(const BWAPI::TilePosition &p) const;

    TaskTypes::Enum getType() const;
    BWAPI::UnitType getUnit() const;
    BWAPI::TechType getTech() const;
    BWAPI::UpgradeType getUpgrade() const;
    BWAPI::TilePosition getTilePosition() const;
    Resources getResources(BWAPI::Player* player = NULL) const;
    std::string getName() const;
    std::string getVerb() const;

  private:
    TaskTypes::Enum type;
    int id;
    BWAPI::TilePosition position;
};

