#pragma once
#include <BWAPI.h>
#include <map>
class UpgradeTimeline
{
  public:
    UpgradeTimeline();
    void reset();
    int getFirstTime(BWAPI::UpgradeType t, int level);
    int getFinalLevel(BWAPI::UpgradeType t);
    void registerUpgradeLevelIncrease(int frame, BWAPI::UpgradeType t);
    std::map< BWAPI::UpgradeType, std::map< int, int > > upgradeEvents;
};