#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <MacroManager.h>
class MacroDependencyResolver
{
  public:
    static MacroDependencyResolver* create();
    MacroDependencyResolver();
    ~MacroDependencyResolver();
    void update();
    int lastFrameCheck;
};
extern MacroDependencyResolver* TheMacroDependencyResolver;