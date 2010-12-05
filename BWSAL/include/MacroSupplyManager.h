#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <MacroManager.h>
class MacroSupplyManager
{
  public:
    static MacroSupplyManager* create();
    MacroSupplyManager();
    ~MacroSupplyManager();
    void update();
    int lastFrameCheck;
    int initialSupplyTotal;
    int initialSupplyProviderCount;
};
extern MacroSupplyManager* TheMacroSupplyManager;