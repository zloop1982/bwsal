#pragma once
#include "Resources.h"
#include <map>
class ResourceTimeline
{
  public:
    ResourceTimeline();
    void reset(const Resources &r, double mgr, double ggr);
    bool reserveResources(int frame, const Resources &r);
    
    enum ErrorCode
    {
      None,
      Insufficient_Minerals,
      Insufficient_Gas,
      Insufficient_Supply
    };
    ErrorCode getLastError() const;
    bool registerSupplyIncrease(int frame, int supply);
    bool unregisterSupplyIncrease(int frame, int supply);
  private:
    ErrorCode lastError;
    double mineralGatherRate;
    double gasGatherRate;
    Resources currentResources;
    std::map<int, Resources > resourceEvents;
};