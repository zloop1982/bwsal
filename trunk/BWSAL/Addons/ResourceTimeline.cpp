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
int ResourceTimeline::getFirstValidTime(const Resources &r)
{
  int frame = Broodwar->getFrameCount();
  resourceEvents[frame]-=r;
  int lastFrame = Broodwar->getFrameCount();
  int lastInvalidEventFrame = -1;
  Resources invalidRes;
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
      invalidRes=res;
      lastInvalidEventFrame = currentFrame;
    }
  }
  resourceEvents[frame]+=r;

  if (lastInvalidEventFrame==-1)
    return frame;
  double t=lastInvalidEventFrame;
  if (invalidRes.getSupply()<0 || 
     (invalidRes.getMinerals()<0 && mineralGatherRate==0) ||
     (invalidRes.getGas()<0 && gasGatherRate==0))
    return -1;
  if (invalidRes.getMinerals()<0)
  {
    double t2=lastInvalidEventFrame-(invalidRes.getMinerals())/mineralGatherRate;
    if (t2>t)
      t=t2;
  }
  if (invalidRes.getGas()<0)
  {
    double t2=lastInvalidEventFrame-(invalidRes.getGas())/gasGatherRate;
    if (t2>t)
      t=t2;
  }
  if (t>(int)t) t=(int)t+1;
  return t;
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