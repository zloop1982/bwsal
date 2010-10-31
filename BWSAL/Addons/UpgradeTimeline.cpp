#include "UpgradeTimeline.h"
#include <BWAPI.h>
using namespace BWAPI;
UpgradeTimeline::UpgradeTimeline()
{
  upgradeEvents.clear();
}
void UpgradeTimeline::reset()
{
  upgradeEvents.clear();
}
int UpgradeTimeline::getFirstTime(BWAPI::UpgradeType t, int level)
{
  int l = Broodwar->self()->getUpgradeLevel(t);
  int frame = Broodwar->getFrameCount();
  if (l >= level)
    return frame;
  if (upgradeEvents.find(t)==upgradeEvents.end())
    return -1;
  std::map<int, int>* events = &(upgradeEvents.find(t)->second);
  for(std::map<int, int>::iterator i=events->begin();i!=events->end();i++)
  {
    frame = (*i).first;
    l+=(*i).second;
    if ( l>= level)
      return frame;
  }
  return -1;
}
void UpgradeTimeline::registerUpgradeLevelIncrease(int frame, BWAPI::UpgradeType t)
{
  upgradeEvents[t][frame]+=1;
}
int UpgradeTimeline::getFinalLevel(BWAPI::UpgradeType t)
{
  int l = Broodwar->self()->getUpgradeLevel(t);
  if (upgradeEvents.find(t)==upgradeEvents.end())
    return l;
  std::map<int, int>* events = &(upgradeEvents.find(t)->second);
  for(std::map<int, int>::iterator i=events->begin();i!=events->end();i++)
    l+=(*i).second;
  return l;
}