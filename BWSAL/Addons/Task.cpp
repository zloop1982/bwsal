#include <Task.h>
using namespace BWAPI;

Task::Task(const BWAPI::UnitType t,    const BWAPI::TilePosition p)
{
  type                       = TaskTypes::Unit;
  id                         = t.getID();
  position                   = p;
  startFrame                 = -1;
  spentResources             = false;
  reservedResourcesThisFrame = false;
  createdSupplyThisFrame     = false;
  completed                  = false;
}
Task::Task(const BWAPI::TechType t,    const BWAPI::TilePosition p)
{
  type     = TaskTypes::Tech;
  id       = t.getID();
  position = p;
  startFrame                 = -1;
  spentResources             = false;
  reservedResourcesThisFrame = false;
  createdSupplyThisFrame     = false;
  completed                  = false;
}
Task::Task(const BWAPI::UpgradeType t, const BWAPI::TilePosition p)
{
  type     = TaskTypes::Upgrade;
  id       = t.getID();
  position = p;
  startFrame                 = -1;
  spentResources             = false;
  reservedResourcesThisFrame = false;
  createdSupplyThisFrame     = false;
  completed                  = false;
}
Task& Task::operator=(const Task t)
{
  type                       = t.type;
  id                         = t.id;
  position                   = t.position;
  startFrame                 = t.startFrame;
  spentResources             = t.spentResources;
  reservedResourcesThisFrame = t.reservedResourcesThisFrame;
  createdSupplyThisFrame     = t.createdSupplyThisFrame;
  completed                  = t.completed;
  return *this;
}
Task& Task::setType(const BWAPI::UnitType t)
{
  type     = TaskTypes::Unit;
  id       = t.getID();
  return *this;
}
Task& Task::setType(const BWAPI::TechType t)
{
  type     = TaskTypes::Tech;
  id       = t.getID();
  return *this;
}
Task& Task::setType(const BWAPI::UpgradeType t)
{
  type     = TaskTypes::Upgrade;
  id       = t.getID();
  return *this;
}
Task& Task::setTilePosition(const BWAPI::TilePosition p)
{
  position = p;
  return *this;
}
bool Task::operator==(void* ptr) const
{
  if (ptr) return false;
  if (*this == Task())
    return true;
  return false;
}
bool Task::operator==(const Task &t) const
{
  if (type                       != t.type) return false;
  if (id                         != t.id) return false;
  if (position                   != t.position) return false;
  if (startFrame                 != t.startFrame) return false;
  if (spentResources             != t.spentResources) return false;
  if (reservedResourcesThisFrame != t.reservedResourcesThisFrame) return false;
  if (createdSupplyThisFrame     != t.createdSupplyThisFrame) return false;
  if (completed                  != t.completed) return false;
  return true;
}
bool Task::operator==(const BWAPI::UnitType &t) const
{
  return (type == TaskTypes::Unit && id == t.getID());
}
bool Task::operator==(const BWAPI::TechType &t) const
{
  return (type == TaskTypes::Tech && id == t.getID());
}
bool Task::operator==(const BWAPI::UpgradeType &t) const
{
  return (type == TaskTypes::Upgrade && id == t.getID());
}
bool Task::operator==(const BWAPI::TilePosition &p) const
{
  return position==p;
}

TaskTypes::Enum Task::getType() const
{
  return type;
}
BWAPI::UnitType Task::getUnit() const
{
  if (type==TaskTypes::Unit)
    return UnitType(id);
  return UnitTypes::None;
}
BWAPI::TechType Task::getTech() const
{
  if (type==TaskTypes::Tech)
    return TechType(id);
  return TechTypes::None;
}
BWAPI::UpgradeType Task::getUpgrade() const
{
  if (type==TaskTypes::Upgrade)
    return UpgradeType(id);
  return UpgradeTypes::None;
}
BWAPI::TilePosition Task::getTilePosition() const
{
  return position;
}
BWAPI::UnitType Task::getWorkerType() const
{
  if (type == TaskTypes::Unit)
    return UnitType(id).whatBuilds().first;
  if (type == TaskTypes::Tech)
    return TechType(id).whatResearches();
  if (type == TaskTypes::Upgrade)
    return UpgradeType(id).whatUpgrades();
  return UnitTypes::None;
}
Resources Task::getResources(BWAPI::Player* player) const
{
  if (type == TaskTypes::Unit)
    return Resources(UnitType(id));
  if (type == TaskTypes::Tech)
    return Resources(TechType(id));
  if (type == TaskTypes::Upgrade)
  {
    if (player == NULL) // assume self() if the user doesn't specify a player
      player = Broodwar->self();
    int level = player->getUpgradeLevel(UpgradeType(id)) + 1;
    if (player->isUpgrading(UpgradeType(id)))
      level++;
    return Resources(UpgradeType(id),level);
  }
  return Resources();
}
int Task::getTime(BWAPI::Player* player) const
{
  if (type == TaskTypes::Unit)
    return UnitType(id).buildTime();
  if (type == TaskTypes::Tech)
    return TechType(id).researchTime();
  if (type == TaskTypes::Upgrade)
  {
    if (player == NULL) // assume self() if the user doesn't specify a player
      player = Broodwar->self();
    int level = player->getUpgradeLevel(UpgradeType(id)) + 1;
    if (player->isUpgrading(UpgradeType(id)))
      level++;
    return UpgradeType(id).upgradeTimeBase()+UpgradeType(id).upgradeTimeFactor()*(level-1);
  }
  return 0;
}
std::string Task::getName() const
{
  if (type == TaskTypes::Unit)
    return UnitType(id).getName();
  if (type == TaskTypes::Tech)
    return TechType(id).getName();
  if (type == TaskTypes::Upgrade)
    return UpgradeType(id).getName();
  return "None";
}
std::string Task::getVerb() const
{
  if (type == TaskTypes::Unit)
    return "Build";
  if (type == TaskTypes::Tech)
    return "Research";
  if (type == TaskTypes::Upgrade)
    return "Upgrade";
  return "";
}
bool Task::isBeingExecutedBy(const BWAPI::Unit* unit) const
{
  if (type == TaskTypes::Unit)
  {
    UnitType ut=getUnit();
    if (ut.getRace()==Races::Zerg && ut.isBuilding()==ut.whatBuilds().first.isBuilding())
      return (unit->isMorphing() && unit->getBuildType() == ut);
    UnitType buildType = unit->getBuildType();
    if (buildType == UnitTypes::None && unit->getBuildUnit()!=NULL)
      buildType = unit->getBuildUnit()->getType();

    if (ut.isBuilding())
      return buildType == ut;
    else
      return (unit->isTraining() && unit->getTrainingQueue().front() == ut);
  }
  else if (type == TaskTypes::Tech)
    return (unit->isResearching() && unit->getTech() == getTech());
  else if (type == TaskTypes::Upgrade)
    return (unit->isUpgrading() && unit->getUpgrade() == getUpgrade());
  return false;
}
void Task::setStartFrame(int frame)
{
  startFrame = frame;
}
int Task::getStartFrame() const
{
  return startFrame;
}

int Task::getRemainingTime(BWAPI::Player* player) const
{
  if (startFrame<0) return -1;
  int t=getTime()-(Broodwar->getFrameCount()-startFrame);
  if (t<0) return 0;
  return t;
}

void Task::setSpentResources(bool spent)
{
  spentResources = spent;
}
bool Task::hasSpentResources() const
{
  return spentResources;
}
void Task::setReservedResourcesThisFrame(bool reserved)
{
  reservedResourcesThisFrame = reserved;
}
bool Task::hasReservedResourcesThisFrame() const
{
  return reservedResourcesThisFrame;
}
void Task::setCreatedSupplyThisFrame(bool created)
{
  createdSupplyThisFrame = created;
}
bool Task::hasCreatedSupplyThisFrame() const
{
  return createdSupplyThisFrame;
}
void Task::setCompleted(bool c)
{
  completed = c;

}
bool Task::isCompleted() const
{
  return completed;
}