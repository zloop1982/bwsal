#include <BuildingPlacer.h>
BuildingPlacer::BuildingPlacer()
{
  reserveMap.resize(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight());
  reserveMap.setTo(false);
}
bool BuildingPlacer::canBuildHere(BWAPI::TilePosition position, BWAPI::UnitType type) const
{
  if (!BWAPI::Broodwar->canBuildHere(NULL, position, type))
    return false;
  for(int x = position.x(); x < position.x() + type.tileWidth(); x++)
    for(int y = position.y(); y < position.y() + type.tileHeight(); y++)
      if (reserveMap[x][y])
        return false;
  return true;
}
bool BuildingPlacer::canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type) const
{
  if (!this->canBuildHere(position, type))
    return false;
  if (position.x() < 2 || position.y() < 2 || position.x() >= BWAPI::Broodwar->mapWidth() - 2 || position.y() >= BWAPI::Broodwar->mapHeight() - 2)
    return false;
  for(int x = position.x() - 1; x < position.x() + type.tileWidth() + 1; x++)
    for(int y = position.y() - 1; y < position.y() + type.tileHeight() + 1; y++)
    {
      std::set<BWAPI::Unit*> units = BWAPI::Broodwar->unitsOnTile(x, y);
      std::set<BWAPI::Unit*> buildings;
      for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
        if ((*i)->getType().isBuilding() && !(*i)->isLifted())
          buildings.insert(*i);
      if (!type.isRefinery() && (!BWAPI::Broodwar->buildable(x, y) || !buildings.empty()))
        return false;
    }
  return true;
}
BWAPI::TilePosition BuildingPlacer::getBuildLocation(BWAPI::UnitType type) const
{
  for(int x = 0; x < BWAPI::Broodwar->mapWidth(); x++)
    for(int y = 0; y < BWAPI::Broodwar->mapHeight(); y++)
      if (this->canBuildHere(BWAPI::TilePosition(x, y), type))
        return BWAPI::TilePosition(x, y);
  return BWAPI::TilePositions::None;
}
BWAPI::TilePosition BuildingPlacer::getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type) const
{
  int x      = position.x();
  int y      = position.y();
  int length = 1;
  int j      = 0;
  bool first = true;
  int dx     = 0;
  int dy     = 1;
  while (length < BWAPI::Broodwar->mapWidth())
  {
    if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
      if (this->canBuildHereWithSpace(BWAPI::TilePosition(x, y), type))
        return BWAPI::TilePosition(x, y);
    x = x + dx;
    y = y + dy;
    j++;
    if (j == length)
    {
      j = 0;
      if (!first)
        length++;
      first =! first;
      if (dx == 0)
      {
        dx = dy;
        dy = 0;
      }
      else
      {
        dy = -dx;
        dx = 0;
      }
    }
  }
  return BWAPI::TilePositions::None;
}
void BuildingPlacer::reserveTiles(BWAPI::TilePosition position, int width, int height)
{
  for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); x++)
    for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); y++)
      reserveMap[x][y] = true;
}
void BuildingPlacer::freeTiles(BWAPI::TilePosition position, int width, int height)
{
  for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); x++)
    for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); y++)
      reserveMap[x][y] = false;
}