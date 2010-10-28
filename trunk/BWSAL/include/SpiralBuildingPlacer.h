#pragma once
#include <MacroManager/TaskStreamObserver.h>
#include <MacroManager/TaskStream.h>
#include <RectangleArray.h>
class SpiralBuildingPlacer : public TaskStreamObserver
{
  public:
    static SpiralBuildingPlacer* getInstance();
    SpiralBuildingPlacer();
    virtual void attached(TaskStream* ts);
    virtual void detached(TaskStream* ts);
    virtual void newStatus(TaskStream* ts);
    virtual void completedTask(TaskStream* ts, const Task &t);
    virtual void update(TaskStream* ts);
    void setTilePosition(TaskStream* ts, BWAPI::TilePosition p);
    void setRelocatable(TaskStream* ts, bool isRelocatable);
    void setBuildDistance(TaskStream* ts, int distance);
  private:
    BWAPI::TilePosition getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const;
    bool canBuildHere(BWAPI::TilePosition position, BWAPI::UnitType type) const;
    bool canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const;
    bool buildable(int x, int y) const;
    void reserveTiles(BWAPI::TilePosition position, int width, int height);
    void freeTiles(BWAPI::TilePosition position, int width, int height);
    struct data
    {
      bool isRelocatable;
      int buildDistance;
      BWAPI::TilePosition reservePosition;
      int reserveWidth;
      int reserveHeight;
    };
    std::map< TaskStream*, data > taskStreams;
    Util::RectangleArray<bool> reserveMap;

};