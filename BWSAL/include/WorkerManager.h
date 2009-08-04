#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
class WorkerManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
  WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
  virtual void onOffer(std::set<BWAPI::Unit*> units);
  virtual void onRevoke(BWAPI::Unit* unit, double bid);
  virtual void update();
  virtual std::string getName();
  Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
  std::set<BWAPI::Unit*> workers;
};