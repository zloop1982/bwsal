#include <SpiralBuildingPlacer.h>
using namespace BWAPI;
SpiralBuildingPlacer* instance = NULL;
SpiralBuildingPlacer* SpiralBuildingPlacer::getInstance()
{
  if (instance==NULL)
    instance = new SpiralBuildingPlacer();
  return instance;
}
SpiralBuildingPlacer::SpiralBuildingPlacer()
{
  reserveMap.resize(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight());
  reserveMap.setTo(false);
}
void SpiralBuildingPlacer::attached(TaskStream* ts)
{
  if (ts->getTask().getTilePosition().isValid()==false)
    ts->getTask().setTilePosition(Broodwar->self()->getStartLocation());
  taskStreams[ts].isRelocatable   = true;
  taskStreams[ts].buildDistance   = 1;
  taskStreams[ts].reservePosition = ts->getTask().getTilePosition();
  taskStreams[ts].reserveWidth    = 0;
  taskStreams[ts].reserveHeight   = 0;
}
void SpiralBuildingPlacer::detached(TaskStream* ts)
{
  taskStreams.erase(ts);
}
void SpiralBuildingPlacer::newStatus(TaskStream* ts)
{
}
void SpiralBuildingPlacer::completedTask(TaskStream* ts, const Task &t)
{
  freeTiles(taskStreams[ts].reservePosition,taskStreams[ts].reserveWidth,taskStreams[ts].reserveHeight);
  taskStreams[ts].reserveWidth  = 0;
  taskStreams[ts].reserveHeight = 0;
}
void SpiralBuildingPlacer::update(TaskStream* ts)
{
  if (ts->getTask().getType()!=TaskTypes::Unit) return;

  int width = ts->getTask().getUnit().tileWidth();
  UnitType type = ts->getTask().getUnit();
  if (type.isAddon()) type=type.whatBuilds().first;

  if (ts->getStatus()==TaskStream::Error_Location_Blocked || ts->getStatus()==TaskStream::Error_Location_Not_Specified)
  {
    if (ts->getTask().getTilePosition().isValid()==false)
      ts->getTask().setTilePosition(Broodwar->self()->getStartLocation());
    if (taskStreams[ts].isRelocatable)
    {
      TilePosition tp(ts->getTask().getTilePosition());
      TilePosition newtp(getBuildLocationNear(tp,type,taskStreams[ts].buildDistance));
      Broodwar->printf("(%d,%d) -> (%d,%d)",tp.x(),tp.y(),newtp.x(),newtp.y());
      ts->getTask().setTilePosition(newtp);
    }
  }
  if (type==BWAPI::UnitTypes::Terran_Command_Center ||
    type==BWAPI::UnitTypes::Terran_Factory || 
    type==BWAPI::UnitTypes::Terran_Starport ||
    type==BWAPI::UnitTypes::Terran_Science_Facility)
  {
    width+=2;
  }

  if (taskStreams[ts].reserveWidth    != width ||
      taskStreams[ts].reserveHeight   != ts->getTask().getUnit().tileHeight() ||
      taskStreams[ts].reservePosition != ts->getTask().getTilePosition())
  {
    freeTiles(taskStreams[ts].reservePosition,taskStreams[ts].reserveWidth,taskStreams[ts].reserveHeight);
    taskStreams[ts].reserveWidth    = width;
    taskStreams[ts].reserveHeight   = ts->getTask().getUnit().tileHeight();
    taskStreams[ts].reservePosition = ts->getTask().getTilePosition();
    reserveTiles(taskStreams[ts].reservePosition,taskStreams[ts].reserveWidth,taskStreams[ts].reserveHeight);
  }
}
void SpiralBuildingPlacer::setTilePosition(TaskStream* ts, BWAPI::TilePosition p)
{
  ts->getTask().setTilePosition(p);
}
void SpiralBuildingPlacer::setRelocatable(TaskStream* ts, bool isRelocatable)
{
  taskStreams[ts].isRelocatable = isRelocatable;
}
void SpiralBuildingPlacer::setBuildDistance(TaskStream* ts, int distance)
{
  taskStreams[ts].buildDistance = distance;
}

BWAPI::TilePosition SpiralBuildingPlacer::getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const
{
  //returns a valid build location near the specified tile position.
  //searches outward in a spiral.
  if (type.isAddon()) type=type.whatBuilds().first;
  int x      = position.x();
  int y      = position.y();
  int length = 1;
  int j      = 0;
  bool first = true;
  int dx     = 0;
  int dy     = 1;
  while (length < BWAPI::Broodwar->mapWidth()) //We'll ride the spiral to the end
  {
    //if we can build here, return this tile position
    if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
      if (this->canBuildHereWithSpace(BWAPI::TilePosition(x, y), type, buildDist))
        return BWAPI::TilePosition(x, y);

    //otherwise, move to another position
    x = x + dx;
    y = y + dy;
    //count how many steps we take in this direction
    j++;
    if (j == length) //if we've reached the end, its time to turn
    {
      //reset step counter
      j = 0;

      //Spiral out. Keep going.
      if (!first)
        length++; //increment step counter if needed

      //first=true for every other turn so we spiral out at the right rate
      first =! first;

      //turn counter clockwise 90 degrees:
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
    //Spiral out. Keep going.
  }
  return BWAPI::TilePositions::None;
}

bool SpiralBuildingPlacer::canBuildHere(BWAPI::TilePosition position, BWAPI::UnitType type) const
{
  if (type.isAddon()) type=type.whatBuilds().first;
  //returns true if we can build this type of unit here. Takes into account reserved tiles.
  if (!BWAPI::Broodwar->canBuildHere(NULL, position, type))
    return false;
  for(int x = position.x(); x < position.x() + type.tileWidth(); x++)
    for(int y = position.y(); y < position.y() + type.tileHeight(); y++)
      if (reserveMap[x][y])
        return false;
  return true;
}

bool SpiralBuildingPlacer::canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const
{
  if (type.isAddon()) type=type.whatBuilds().first;
  //returns true if we can build this type of unit here with the specified amount of space.
  //space value is stored in this->buildDistance.

  //if we can't build here, we of course can't build here with space
  if (!this->canBuildHere(position, type))
    return false;

  int width=type.tileWidth();
  int height=type.tileHeight();

  //make sure we leave space for add-ons. These types of units can have addons:
  if (type==BWAPI::UnitTypes::Terran_Command_Center ||
    type==BWAPI::UnitTypes::Terran_Factory || 
    type==BWAPI::UnitTypes::Terran_Starport ||
    type==BWAPI::UnitTypes::Terran_Science_Facility)
  {
    width+=2;
  }
  int startx = position.x() - buildDist;
  if (startx<0) return false;
  int starty = position.y() - buildDist;
  if (starty<0) return false;
  int endx = position.x() + width + buildDist;
  if (endx>BWAPI::Broodwar->mapWidth()) return false;
  int endy = position.y() + height + buildDist;
  if (endy>BWAPI::Broodwar->mapHeight()) return false;

  if (!type.isRefinery())
  {
    for(int x = startx; x < endx; x++)
      for(int y = starty; y < endy; y++)
          if (!buildable(x, y) || reserveMap[x][y])
            return false;
  }
  if (position.x()>3)
  {
    int startx2=startx-2;
    if (startx2<0) startx2=0;
    for(int x = startx2; x < startx; x++)
      for(int y = starty; y < endy; y++)
      {
        std::set<BWAPI::Unit*> units = BWAPI::Broodwar->unitsOnTile(x, y);
        for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
        {
          if (!(*i)->isLifted())
          {
            BWAPI::UnitType type=(*i)->getType();
            if (type==BWAPI::UnitTypes::Terran_Command_Center ||
              type==BWAPI::UnitTypes::Terran_Factory || 
              type==BWAPI::UnitTypes::Terran_Starport ||
              type==BWAPI::UnitTypes::Terran_Science_Facility)
            {
              return false;
            }
          }
        }
      }
  }
  return true;
}

bool SpiralBuildingPlacer::buildable(int x, int y) const
{
  //returns true if this tile is currently buildable, takes into account units on tile
  if (!BWAPI::Broodwar->isBuildable(x,y)) return false;
  std::set<BWAPI::Unit*> units = BWAPI::Broodwar->unitsOnTile(x, y);
  for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
    if ((*i)->getType().isBuilding() && !(*i)->isLifted())
      return false;
  return true;
}

void SpiralBuildingPlacer::reserveTiles(BWAPI::TilePosition position, int width, int height)
{
  for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); x++)
    for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); y++)
      reserveMap[x][y] = true;
}

void SpiralBuildingPlacer::freeTiles(BWAPI::TilePosition position, int width, int height)
{
  for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); x++)
    for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); y++)
      reserveMap[x][y] = false;
}
