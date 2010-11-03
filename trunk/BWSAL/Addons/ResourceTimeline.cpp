#include "ResourceTimeline.h"
#include <BWAPI.h>
#include <math.h>
using namespace std;
using namespace BWAPI;
ResourceTimeline::ResourceTimeline()
{
  currentResources = Resources();
  resourceEvents.clear();
  supplyIncreaseEvents.clear();
  mineralGatherRate = 0;
  gasGatherRate = 0;
  lastError = None;
}
void ResourceTimeline::reset(const Resources &r, int supplyTotal, double mgr, double ggr)
{
  currentResources = r;
  currentSupplyTotal = supplyTotal;
  resourceEvents.clear();
  supplyIncreaseEvents.clear();
  mineralGatherRate = mgr;
  gasGatherRate = ggr;
  lastError = None;
}

Resources ResourceTimeline::getActualResourcesAtTime(int frame)
{
  if (resourceEvents.find(frame)==resourceEvents.end())
    resourceEvents.insert(std::make_pair(frame,Resources()));
  int lastFrame = Broodwar->getFrameCount();
  Resources res = currentResources;
  for(std::map<int, Resources>::iterator i=resourceEvents.begin();i!=resourceEvents.end();i++)
  {
    int currentFrame = i->first;
    int duration = currentFrame - lastFrame;
    res.addMinerals((int)(duration*mineralGatherRate));
    res.addGas((int)(duration*gasGatherRate));
    Resources resi=i->second;
    if (resi.getMinerals()<0) resi.setMinerals(0);
    if (resi.getGas()<0) resi.setGas(0);
    if (resi.getSupply()<0) resi.setSupply(0);
    res+=resi;
    if (currentFrame == frame)
      break;
    lastFrame = currentFrame;
  }
  return res;
}
Resources ResourceTimeline::getAvailableResourcesAtTime(int frame)
{
  if (resourceEvents.find(frame)==resourceEvents.end())
    resourceEvents.insert(std::make_pair(frame,Resources()));
  int lastFrame = Broodwar->getFrameCount();
  Resources res = currentResources;
  Resources availRes;
  for(std::map<int, Resources>::iterator i=resourceEvents.begin();i!=resourceEvents.end();i++)
  {
    int currentFrame = i->first;
    int duration = currentFrame - lastFrame;
    res.addMinerals((int)(duration*mineralGatherRate));
    res.addGas((int)(duration*gasGatherRate));
    res+=i->second;
    if (currentFrame == frame)
    {
      availRes=res;
    }
    else if (currentFrame>frame)
    {
      if (res.getMinerals()<availRes.getMinerals())
        availRes.setMinerals(res.getMinerals());
      if (res.getGas()<availRes.getGas())
        availRes.setGas(res.getGas());
      if (res.getSupply()<availRes.getSupply())
        availRes.setSupply(res.getSupply());
    }
    lastFrame = currentFrame;
  }
  return availRes;
}
int ResourceTimeline::getSupplyTotalAtTime(int frame)
{
  int supplyTotal = currentSupplyTotal;
  for(std::map<int, int>::iterator i=supplyIncreaseEvents.begin();i!=supplyIncreaseEvents.end();i++)
  {
    if (i->first > frame) break;
    supplyTotal+=i->second;
  }
  return supplyTotal;
}
int ResourceTimeline::getFinalSupplyAvailable()
{
  Resources res = currentResources;
  int supply = (int)(currentResources.getSupply());
  for(std::map<int, Resources>::iterator i=resourceEvents.begin();i!=resourceEvents.end();i++)
  {
    supply+=(int)(i->second.getSupply());
  }
  return supply;
}
int ResourceTimeline::getFinalSupplyTotal()
{
  int supplyTotal = currentSupplyTotal;
  for(std::map<int, int>::iterator i=supplyIncreaseEvents.begin();i!=supplyIncreaseEvents.end();i++)
  {
    supplyTotal+=i->second;
  }
  return supplyTotal;
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
    lastFrame = currentFrame;
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
  bool isValid = false;
  int validEventFrame = -1;
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
      isValid = false;
    }
    else
    {
      if (!isValid)
        validEventFrame = currentFrame;
      isValid = true;
    }
    lastFrame = currentFrame;
  }
  resourceEvents[frame]+=r;

  if (lastInvalidEventFrame==-1)
    return frame;
  double t=lastInvalidEventFrame;
  if (invalidRes.getSupply()<-0.0001 || 
     (invalidRes.getGas()<-0.0001 && fabs(gasGatherRate)<0.0001) ||
     (invalidRes.getMinerals()<-0.0001 && fabs(mineralGatherRate)<0.0001))
  {
    if (validEventFrame>lastInvalidEventFrame)
      return validEventFrame;
    if (invalidRes.getSupply()<-0.0001)
      lastError = Insufficient_Supply;
    else if (invalidRes.getGas()<-0.0001 && fabs(gasGatherRate)<0.0001)
      lastError = Insufficient_Gas;
    else if (invalidRes.getMinerals()<-0.0001 && fabs(mineralGatherRate)<0.0001)
      lastError = Insufficient_Minerals;
    return -1;
  }
  if (invalidRes.getMinerals()<-0.001)
  {
    double t2=lastInvalidEventFrame-(invalidRes.getMinerals())/mineralGatherRate;
    if (t2>t)
    {
      t=t2;
      lastError = Insufficient_Minerals;
    }
  }
  if (invalidRes.getGas()<-0.001)
  {
    double t2=lastInvalidEventFrame-(invalidRes.getGas())/gasGatherRate;
    if (t2>t)
    {
      t=t2;
      lastError = Insufficient_Gas;
    }
  }
  int ti=(int)t;
  if (t>(int)t) ti++;
  return ti;
}
int ResourceTimeline::getFirstTimeWhenSupplyIsNoGreaterThan(int supplyAmount)
{
  int frame = Broodwar->getFrameCount();
  Resources res = currentResources;
  int supply = (int)(currentResources.getSupply());
  if (supply<=supplyAmount) return frame;
  for(std::map<int, Resources>::iterator i=resourceEvents.begin();i!=resourceEvents.end();i++)
  {
    frame = i->first;
    supply+=(int)(i->second.getSupply());
    if (supply<=supplyAmount) return frame;
  }
  return -1;
}
ResourceTimeline::ErrorCode ResourceTimeline::getLastError() const
{
  return lastError;
}
bool ResourceTimeline::registerSupplyIncrease(int frame, int supply)
{
  if (supply<0) return false;
  int currentTotal=getSupplyTotalAtTime(frame);
  int newTotal = currentTotal + supply;
  if (newTotal > 400) newTotal = 400;
  supply = newTotal-currentTotal;  
  resourceEvents[frame].addSupply(supply);
  supplyIncreaseEvents[frame]+=supply;
  return true;
}
bool ResourceTimeline::unregisterSupplyIncrease(int frame, int supply)
{
  if (supply<0) return false;
  if (supplyIncreaseEvents[frame]<supply)
    return false;
  resourceEvents[frame].addSupply(-supply);
  supplyIncreaseEvents[frame]-=supply;
  return true;
}