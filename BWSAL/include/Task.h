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
    Task(const BWAPI::UpgradeType t, int l = -1,              const BWAPI::TilePosition p = BWAPI::TilePositions::None);

    Task& operator=(const Task t);
    Task& setType(const BWAPI::UnitType t,    const BWAPI::TilePosition p = BWAPI::TilePositions::None);
    Task& setType(const BWAPI::TechType t,    const BWAPI::TilePosition p = BWAPI::TilePositions::None);
    Task& setType(const BWAPI::UpgradeType t, int l = -1,    const BWAPI::TilePosition p = BWAPI::TilePositions::None);
    Task& setLevel(int l);
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
    int getLevel() const;
    BWAPI::TilePosition getTilePosition() const;
    BWAPI::UnitType getWorkerType() const;
    std::map<BWAPI::UnitType, int> getRequiredUnits() const;
    Resources getResources() const;
    int getTime() const;
    std::string getName() const;
    std::string getVerb() const;

    void setStartTime(int time);
    int getStartTime() const;
    int getFinishTime() const;

    int getRemainingTime(BWAPI::Player* player = NULL) const;

    void setSpentResources(bool spent);
    bool hasSpentResources() const;

    void setReservedResourcesThisFrame(bool reserved);
    bool hasReservedResourcesThisFrame() const;

    void setReservedFinishDataThisFrame(bool reserved);
    bool hasReservedFinishDataThisFrame() const;

    void setCompleted(bool c);
    bool isCompleted() const;

  private:
    TaskTypes::Enum type;
    int id;
    int level;
    BWAPI::TilePosition position;
    int startTime;
    bool spentResources;
    bool reservedResourcesThisFrame;
    bool reservedFinishDataThisFrame;
    bool completed;
};

