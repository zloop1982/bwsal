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
    Task(const BWAPI::UnitType t = BWAPI::UnitTypes::None,    const BWAPI::TilePosition p = BWAPI::TilePositions::None);
    Task(const BWAPI::TechType t,                             const BWAPI::TilePosition p = BWAPI::TilePositions::None);
    Task(const BWAPI::UpgradeType t,                          const BWAPI::TilePosition p = BWAPI::TilePositions::None);

    Task& operator=(const Task t);
    Task& setType(const BWAPI::UnitType t);
    Task& setType(const BWAPI::TechType t);
    Task& setType(const BWAPI::UpgradeType t);
    Task& setTilePosition(const BWAPI::TilePosition p);

    bool operator==(void* ptr) const;
    bool operator==(const Task &t) const;
    bool operator==(const BWAPI::UnitType &t) const;
    bool operator==(const BWAPI::TechType &t) const;
    bool operator==(const BWAPI::UpgradeType &t) const;
    bool operator==(const BWAPI::TilePosition &p) const;
    bool isBeingExecutedBy(const BWAPI::Unit* unit) const;

    TaskTypes::Enum getType() const;
    BWAPI::UnitType getUnit() const;
    BWAPI::TechType getTech() const;
    BWAPI::UpgradeType getUpgrade() const;
    BWAPI::TilePosition getTilePosition() const;
    BWAPI::UnitType getWorkerType() const;
    Resources getResources(BWAPI::Player* player = NULL) const;
    int getTime(BWAPI::Player* player = NULL) const;
    std::string getName() const;
    std::string getVerb() const;

    void setStartFrame(int frame);
    int getStartFrame() const;

    int getRemainingTime(BWAPI::Player* player = NULL) const;

    void setSpentResources(bool spent);
    bool hasSpentResources() const;

    void setReservedResourcesThisFrame(bool reserved);
    bool hasReservedResourcesThisFrame() const;

    void setCreatedSupplyThisFrame(bool created);
    bool hasCreatedSupplyThisFrame() const;

    void setCompleted(bool c);
    bool isCompleted() const;

  private:
    TaskTypes::Enum type;
    int id;
    BWAPI::TilePosition position;
    int startFrame;
    bool spentResources;
    bool reservedResourcesThisFrame;
    bool createdSupplyThisFrame;
    bool completed;
};

