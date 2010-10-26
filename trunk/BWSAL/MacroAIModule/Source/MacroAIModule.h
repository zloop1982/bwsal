#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <Arbitrator.h>
#include <WorkerManager.h>
#include <MacroManager.h>

class MacroAIModule : public BWAPI::AIModule
{
  public:
    virtual void onStart();
    virtual void onEnd(bool isWinner);
    virtual void onFrame();
  private:
    Arbitrator::Arbitrator<BWAPI::Unit*,double> arbitrator;
    MacroManager* macroManager;
};