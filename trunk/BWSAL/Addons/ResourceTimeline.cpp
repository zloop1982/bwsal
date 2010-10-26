#include "ResourceTimeline.h"
#include <BWAPI.h>
using namespace BWAPI;
ResourceTimeline::ResourceTimeline()
{
  currentResources = Resources();
  resourceEvents.clear();
  mineralGatherRate = 0;
  gasGatherRate = 0;
  lastError = None;
}
void ResourceTimeline::reset(const Resources &r, double mgr, double ggr)
{
  currentResources = r;
  resourceEvents.clear();
  mineralGatherRate = mgr;
  gasGatherRate = ggr;
  lastError = None;
}
bool ResourceTimeline::reserveResources(int frame, const Resources &r)
{
  resourceEvents[frame]-=r;
  int lastFrame = Broodwar->getFrameCount();
  Resources res = currentResources;
  bool isValid = true;
  for(std::map<int, Resources>::iterator i=resourceEvents.begin();i!=resourceEvents.end();i++)
  {
    int currentFrame = i->first;
    int duration = currentFrame - lastFrame;
    res.addMinerals((int)(duration*mineralGatherRate));
    res.addGas((int)(duration*gasGatherRate));
    res+=i->second;
    if (res.isValid()==false)
    {
      isValid = false;
      break;
    }
  }
  if (isValid)
    lastError = None;
  else
  {
    if (res.getSupply()<0)
      lastError = Insufficient_Supply;
    else if (res.getGas()<0)
      lastError = Insufficient_Gas;
    else
      lastError = Insufficient_Minerals;
    resourceEvents[frame]+=r;
  }
  return isValid;
}
ResourceTimeline::ErrorCode ResourceTimeline::getLastError() const
{
  return lastError;
}
bool ResourceTimeline::registerSupplyIncrease(int frame, int supply)
{
  if (supply<0) return false;
  resourceEvents[frame].addSupply(supply);
  return true;
}
bool ResourceTimeline::unregisterSupplyIncrease(int frame, int supply)
{
  if (supply<0) return false;
  if (resourceEvents[frame].getSupply()<supply)
    return false;
  resourceEvents[frame].addSupply(-supply);
  return true;
}