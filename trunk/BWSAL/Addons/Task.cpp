#include <Task.h>
using namespace BWAPI;

Task::Task(const BWAPI::UnitType &t,    const BWAPI::TilePosition &p)
{
  type     = TaskTypes::Unit;
  id       = t.getID();
  position = p;
}
Task::Task(const BWAPI::TechType &t,    const BWAPI::TilePosition &p)
{
  type     = TaskTypes::Tech;
  id       = t.getID();
  position = p;
}
Task::Task(const BWAPI::UpgradeType &t, const BWAPI::TilePosition &p)
{
  type     = TaskTypes::Upgrade;
  id       = t.getID();
  position = p;
}
Task& Task::operator=(const Task &t)
{
  type     = t.type;
  id       = t.id;
  position = t.position;
  return *this;
}
Task& Task::operator=(const BWAPI::UnitType &t)
{
  type     = TaskTypes::Unit;
  id       = t.getID();
  return *this;
}
Task& Task::operator=(const BWAPI::TechType &t)
{
  type     = TaskTypes::Tech;
  id       = t.getID();
  return *this;
}
Task& Task::operator=(const BWAPI::UpgradeType &t)
{
  type     = TaskTypes::Upgrade;
  id       = t.getID();
  return *this;
}
Task& Task::operator=(const BWAPI::TilePosition &p)
{
  position = p;
  return *this;
}
Task& Task::setType(const BWAPI::UnitType &t)
{
  type     = TaskTypes::Unit;
  id       = t.getID();
  return *this;
}
Task& Task::setType(const BWAPI::TechType &t)
{
  type     = TaskTypes::Tech;
  id       = t.getID();
  return *this;
}
Task& Task::setType(const BWAPI::UpgradeType &t)
{
  type     = TaskTypes::Upgrade;
  id       = t.getID();
  return *this;
}
Task& Task::setTilePosition(const BWAPI::TilePosition &p)
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
  if (type     != t.type) return false;
  if (id       != t.id) return false;
  if (position != t.position) return false;
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
Resources Task::getResources(BWAPI::Player* player) const
{
  if (type == TaskTypes::Unit)
    return Resources(UnitType(id));
  if (type == TaskTypes::Tech)
    return Resources(TechType(id));
  //type == TaskTypes::Upgrade
  if (player == NULL) // assume self() if the user doesn't specify a player
    player = Broodwar->self();
  int level = player->getUpgradeLevel(UpgradeType(id)) + 1;
  if (player->isUpgrading(UpgradeType(id)))
    level++;
  return Resources(UpgradeType(id),level);
}
int Task::getTime(BWAPI::Player* player) const
{
  if (type == TaskTypes::Unit)
    return UnitType(id).buildTime();
  if (type == TaskTypes::Tech)
    return TechType(id).researchTime();
  //type == TaskTypes::Upgrade
  if (player == NULL) // assume self() if the user doesn't specify a player
    player = Broodwar->self();
  int level = player->getUpgradeLevel(UpgradeType(id)) + 1;
  if (player->isUpgrading(UpgradeType(id)))
    level++;
  return UpgradeType(id).upgradeTimeBase()+UpgradeType(id).upgradeTimeFactor()*(level-1);
}
std::string Task::getName() const
{
  if (type == TaskTypes::Unit)
    return UnitType(id).getName();
  if (type == TaskTypes::Tech)
    return TechType(id).getName();
  //type == TaskTypes::Upgrade
  return UpgradeType(id).getName();
}
std::string Task::getVerb() const
{
  if (type == TaskTypes::Unit)
    return "Build";
  if (type == TaskTypes::Tech)
    return "Research";
  //type == TaskTypes::Upgrade
  return "Upgrade";
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