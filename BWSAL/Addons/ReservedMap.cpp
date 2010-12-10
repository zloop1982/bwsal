#include <ReservedMap.h>
using namespace BWAPI;
ReservedMap* TheReservedMap = NULL;
ReservedMap* ReservedMap::create()
{
  if (TheReservedMap) return TheReservedMap;
  return new ReservedMap();
}
void ReservedMap::destroy()
{
  if (TheReservedMap)
    delete TheReservedMap;
}
ReservedMap::ReservedMap()
{
  TheReservedMap = this;
  reserveMap.resize(Broodwar->mapWidth(),Broodwar->mapHeight());
  reserveMap.setTo(false);
}
ReservedMap::~ReservedMap()
{
  TheReservedMap = NULL;
}
void ReservedMap::reserveTiles(BWAPI::TilePosition position, int width, int height)
{
  for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); x++)
    for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); y++)
      reserveMap[x][y] = true;
}
void ReservedMap::freeTiles(BWAPI::TilePosition position, int width, int height)
{
  for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); x++)
    for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); y++)
      reserveMap[x][y] = false;
}
bool ReservedMap::isReserved(int x, int y)
{
  return reserveMap[x][y];
}
bool ReservedMap::isReserved(TilePosition p)
{
  return reserveMap[p.x()][p.y()];
}