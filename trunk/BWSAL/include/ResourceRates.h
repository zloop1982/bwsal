#pragma once
#include "Resources.h"
#include <map>
class ResourceRates
{
  public:
    static ResourceRates* create();
    ResourceRates();
    ~ResourceRates();
    Resources getGatherRate() const;
    void update();
  private:
    Resources gatherRate;
    int gatherRateStepSize;
    std::list<Resources> resourceHistory;
};
extern ResourceRates* TheResourceRates;