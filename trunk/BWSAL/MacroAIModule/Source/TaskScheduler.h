#pragma once
class TaskScheduler
{
  public:
  void schedule(Task* task);
  private:
    Timeline* t;
};