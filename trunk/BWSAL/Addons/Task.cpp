#include <Task.h>
using namespace BWAPI;

Task::Task(const BWAPI::UnitType &t,    const BWAPI::TilePosition &p)
{
  type     = TaskType::Unit;
  id       = t.getID();
  position = p;
}
Task::Task(const BWAPI::TechType &t,    const BWAPI::TilePosition &p)
{
  type     = TaskType::Tech;
  id       = t.getID();
  position = p;
}
Task::Task(const BWAPI::UpgradeType &t, const BWAPI::TilePosition &p)
{
  type     = TaskType::Upgrade;
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
  type     = TaskType::Unit;
  id       = t.getID();
  return *this;
}
Task& Task::operator=(const BWAPI::TechType &t)
{
  type     = TaskType::Tech;
  id       = t.getID();
  return *this;
}
Task& Task::operator=(const BWAPI::UpgradeType &t)
{
  type     = TaskType::Upgrade;
  id       = t.getID();
  return *this;
}
Task& Task::operator=(const BWAPI::TilePosition &p)
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
  return (type == TaskType::Unit && id == t.getID());
}
bool Task::operator==(const BWAPI::TechType &t) const
{
  return (type == TaskType::Tech && id == t.getID());
}
bool Task::operator==(const BWAPI::UpgradeType &t) const
{
  return (type == TaskType::Upgrade && id == t.getID());
}
bool Task::operator==(const BWAPI::TilePosition &p) const
{
  return position==p;
}

TaskType::Enum Task::getType() const
{
  return type;
}
BWAPI::UnitType Task::getUnit() const
{
  if (type==TaskType::Unit)
    return UnitType(id);
  return UnitTypes::None;
}
BWAPI::TechType Task::getTech() const
{
  if (type==TaskType::Tech)
    return TechType(id);
  return TechTypes::None;
}
BWAPI::UpgradeType Task::getUpgrade() const
{
  if (type==TaskType::Upgrade)
    return UpgradeType(id);
  return UpgradeTypes::None;
}
BWAPI::TilePosition Task::getTilePosition() const
{
  return position;
}