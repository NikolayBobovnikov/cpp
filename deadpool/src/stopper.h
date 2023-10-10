#ifndef STOPPER_H
#define STOPPER_H

#include <atomic>
#include <memory>

class Stopper
{
 public:
  std::shared_ptr<std::atomic_bool> stopSignal;
  Stopper();
};

#endif /* STOPPER_H */
