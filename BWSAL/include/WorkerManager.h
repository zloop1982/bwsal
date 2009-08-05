#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BaseManager.h>
class WorkerManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    class WorkerData
    {
      public:
        WorkerData() {mineral = NULL; lastFrameSpam = 0;}
        BWAPI::Unit* mineral;
        BWAPI::Unit* newMineral;
        int lastFrameSpam;
    };
    WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BaseManager* baseManager);
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    void onRemoveUnit(BWAPI::Unit* unit);
    virtual std::string getName();
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
    BaseManager* baseManager;
    std::map<BWAPI::Unit*,WorkerData> workers;
    std::map<BWAPI::Unit*, std::set<BWAPI::Unit*> > currentWorkers;
    std::map<BWAPI::Unit*, Base*> mineralBase;
    std::map<BWAPI::Unit*, int> desiredWorkerCount;
    std::vector<std::pair<BWAPI::Unit*, int> > mineralOrder;
    int mineralOrderIndex;
    int lastSCVBalance;
    std::set<Base*> basesCache;
};