#include <chrono>
#include <thread>

#include <external_api.h>

// NOLINTNEXTLINE (readability-identifier-naming)
Request *GetRequest(Stopper stopSignal) noexcept
{
  constexpr auto delay = 100;
  std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  return *(stopSignal.stopSignal) ? nullptr : new Request();
}

// NOLINTNEXTLINE (readability-identifier-naming)
void ProcessRequest(Request *request, Stopper stopSignal) noexcept
{
  constexpr auto delay = 200;
  std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}