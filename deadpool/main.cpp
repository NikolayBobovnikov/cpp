#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

// Dummy Request class
class Request
{
};

// Forward declaration of Stopper
class Stopper;

// Dummy GetRequest function
// NOLINTNEXTLINE (readability-identifier-naming)
Request *GetRequest(Stopper stopSignal) noexcept;

// Dummy ProcessRequest function
// NOLINTNEXTLINE (readability-identifier-naming)
void ProcessRequest(Request *request, Stopper stopSignal) noexcept;

class Stopper
{
 public:
  std::shared_ptr<std::atomic_bool> stopSignal;

  Stopper()
      : stopSignal(std::make_shared<std::atomic_bool>(false))
  {
  }
};

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

// Corrected FunctionTraits definition, include the case for noexcept functions
template <typename T>
struct FunctionTraits;

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...) noexcept> {
  // NOLINTNEXTLINE (readability-identifier-naming)
  using return_type = ReturnType;
  // NOLINTNEXTLINE (readability-identifier-naming)
  using args_tuple = std::tuple<Args...>;
};

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...)> {
  // NOLINTNEXTLINE (readability-identifier-naming)
  using return_type = ReturnType;
  // NOLINTNEXTLINE (readability-identifier-naming)
  using args_tuple = std::tuple<Args...>;
};

// Correct the static_asserts to properly refer to the FunctionTraits
class ProcessRequestFunc
{
 public:
  using FuncType = decltype(&ProcessRequest);

  ProcessRequestFunc(FuncType func)
      : m_func(func)
  {
    using Traits = FunctionTraits<FuncType>;
    static_assert(std::is_same_v<typename Traits::return_type, void>,
                  "Return type must be void");
    static_assert(
        std::is_same_v<std::tuple_element_t<0, typename Traits::args_tuple>,
                       Request *>,
        "First argument must be Request*");
    static_assert(
        std::is_same_v<std::tuple_element_t<1, typename Traits::args_tuple>,
                       Stopper>,
        "Second argument must be Stopper");
  }

  // Remaining code is the same
  void operator()(Request *req, Stopper stopper) const noexcept
  {
    m_func(req, std::move(stopper));
  }

 private:
  FuncType m_func;
};

class RequestProcessor
{
 public:
  explicit RequestProcessor(ProcessRequestFunc func,
                            size_t initialThreadCount = 2)
      : m_processFunction(func)
      , m_threadsCount(initialThreadCount)
  {
    for(size_t i = 0; i < initialThreadCount; ++i) {
      spawnThread();
    }
  }
  ~RequestProcessor() { stop(); }

  RequestProcessor(const RequestProcessor &) = delete;
  RequestProcessor &operator=(const RequestProcessor &) = delete;
  RequestProcessor(RequestProcessor &&) = delete;
  RequestProcessor &operator=(const RequestProcessor &&) = delete;

  void process(Request *rawRequest) noexcept
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

  void addThreads(size_t additionalThreads)
  {
    for(size_t i = 0; i < additionalThreads; ++i) {
      spawnThread();
    }
    m_threadsCount += additionalThreads;
  }

  void stop()
  {
    m_stopper.stopSignal->store(true);
    m_cv.notify_all();

    for(auto &worker : m_threads) {
      if(worker.joinable()) {
        worker.join();
      }
    }
  }

 private:
  void spawnThread()
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

  std::queue<std::unique_ptr<Request>> m_requests;
  std::vector<std::thread> m_threads;
  std::mutex m_mtx;
  std::condition_variable m_cv;
  Stopper m_stopper;
  ProcessRequestFunc m_processFunction;
  size_t m_threadsCount;
};

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