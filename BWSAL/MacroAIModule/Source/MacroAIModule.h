#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <Arbitrator.h>
#include <WorkerManager.h>
#include <MacroManager.h>

class MacroAIModule : public BWAPI::AIModule
{
  public:
  Arbitrator::Arbitrator<BWAPI::Unit*,double> arbitrator;
};