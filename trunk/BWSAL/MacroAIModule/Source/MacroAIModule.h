#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <Arbitrator.h>
#include <WorkerManager.h>
#include <MacroManager.h>
#include <ResourceRates.h>

class MacroAIModule : public BWAPI::AIModule
{
  public:
    MacroAIModule();
    ~MacroAIModule();
    virtual void onStart();
    virtual void onEnd(bool isWinner);
    virtual void onFrame();
    virtual void onSendText(std::string text);
  private:
    Arbitrator::Arbitrator<BWAPI::Unit*,double> arbitrator;
};