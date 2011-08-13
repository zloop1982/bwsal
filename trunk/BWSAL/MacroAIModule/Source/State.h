class State
{
public:
  int supply;
  double minerals;
  double gas;
  int time;
  map <MetaUnit*, int> availableSince;
  map<UnitType, int> completedUnitTypeCounts;
};