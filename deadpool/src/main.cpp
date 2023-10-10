#include <request_processor.h>

int main()
{
  Stopper globalStopper;
  RequestProcessor processor(ProcessRequest);

  auto start = std::chrono::high_resolution_clock::now();
  auto now = start;
  constexpr auto delay = 30;

  while(std::chrono::duration_cast<std::chrono::seconds>(now - start).count() <
        delay) {
    try {
      Request *req = GetRequest(globalStopper);
      if(nullptr != req) {
        processor.process(req);
      }
    } catch(const std::exception &e) {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
    now = std::chrono::high_resolution_clock::now();
  }

  globalStopper.stopSignal->store(true);
  processor.stop();

  return 0;
}