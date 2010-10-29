#include "UnitCountTimeline.h"
#include <BWAPI.h>
using namespace BWAPI;
UnitCountTimeline::UnitCountTimeline()
{
  unitEvents.clear();
}
void UnitCountTimeline::reset()
{
  unitEvents.clear();
}
int UnitCountTimeline::getFirstTime(BWAPI::UnitType t, int count)
{
  int c = Broodwar->self()->completedUnitCount(t);
  int frame = Broodwar->getFrameCount();
  if (c >= count)
    return frame;
  std::map<int, int>* events = &(unitEvents.find(t)->second);
  for(std::map<int, int>::iterator i=events->begin();i!=events->end();i++)
  {
    frame = (*i).first;
    c+=(*i).second;
    if ( c>= count)
      return frame;
  }
  return -1;
}
void UnitCountTimeline::registerUnitCountChange(int frame, BWAPI::UnitType t, int count)
{
  unitEvents[t][frame]+=count;
}