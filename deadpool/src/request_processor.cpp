#include <request_processor.h>

// implicit intended
RequestProcessor::RequestProcessor(ProcessRequestFunc func,
                                   size_t initialThreadCount)
    : m_processFunction(func)
    , m_threadsCount(initialThreadCount)
{
  for(size_t i = 0; i < initialThreadCount; ++i) {
    spawnThread();
  }
}

RequestProcessor::~RequestProcessor() { stop(); }

void RequestProcessor::process(Request *rawRequest) noexcept
{
  try {
    std::unique_ptr<Request> request(rawRequest);
    std::unique_lock<std::mutex> lock(m_mtx);
    m_requests.push(std::move(request));
    m_cv.notify_one();
  } catch(const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

void RequestProcessor::addThreads(size_t additionalThreads)
{
  for(size_t i = 0; i < additionalThreads; ++i) {
    spawnThread();
  }
  m_threadsCount += additionalThreads;
}

void RequestProcessor::stop()
{
  m_stopper.stopSignal->store(true);
  m_cv.notify_all();

  for(auto &worker : m_threads) {
    if(worker.joinable()) {
      worker.join();
    }
  }
}

void RequestProcessor::spawnThread()
{
  m_threads.emplace_back([this]() {
    while(!m_stopper.stopSignal->load()) {
      std::unique_ptr<Request> request;
      {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [this]() {
          return !m_requests.empty() || m_stopper.stopSignal->load();
        });

        if(!m_requests.empty()) {
          request = std::move(m_requests.front());
          m_requests.pop();
        }
      }

      if(request) {
        try {
          m_processFunction(request.get(), m_stopper);
        } catch(const std::exception &e) {
          std::cerr << "Exception: " << e.what() << std::endl;
        }
      }
    }
  });
}