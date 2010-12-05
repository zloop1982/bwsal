#pragma once
#include <BWAPI.h>
#include <map>
#include <Task.h>

//Timeline of the planned amount of each UnitType
class LarvaTimeline
{
  public:
    LarvaTimeline();
    void reset();
    std::pair<int,int> getFirstFreeInterval(BWAPI::Unit* worker, int earliestStartTime = -1);
    bool reserveLarva(BWAPI::Unit* worker, int startFrame, Task* task);
    int getPlannedLarvaCount(BWAPI::Unit* worker, int frame);
  private:
    std::map<BWAPI::Unit*, std::map<int, int> > larvaEventTimes;
};