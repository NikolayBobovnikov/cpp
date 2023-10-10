#include <stopper.h>

Stopper::Stopper()
    : stopSignal(std::make_shared<std::atomic_bool>(false))
{
}