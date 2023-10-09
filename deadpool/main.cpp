#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <atomic>

// Dummy Request class
class Request
{
};

// Forward declaration of Stopper
class Stopper;

// Dummy GetRequest function
Request *GetRequest(Stopper stopSignal) noexcept;

// Dummy ProcessRequest function
void ProcessRequest(Request *request, Stopper stopSignal) noexcept;

class Stopper
{
public:
  std::shared_ptr<std::atomic_bool> stopSignal;

  Stopper()
      : stopSignal(std::make_shared<std::atomic_bool>(false)) {}

  Stopper(const Stopper &other)
      : stopSignal(other.stopSignal) {}
};

Request *GetRequest(Stopper stopSignal) noexcept
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return *(stopSignal.stopSignal) ? nullptr : new Request();
}

void ProcessRequest(Request *request, Stopper stopSignal) noexcept
{
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

template <typename Func>
class TaskProcessor
{
private:
  std::queue<std::unique_ptr<Request>> requestQueue;
  std::vector<std::thread> workerThreads;
  std::mutex queueMutex;
  std::condition_variable queueCondVar;
  Stopper stopper;
  Func processFunction;
  size_t threadCount;

public:
  TaskProcessor(Func func, size_t initialThreadCount = 2)
      : processFunction(func), threadCount(initialThreadCount)
  {
    for (size_t i = 0; i < initialThreadCount; ++i)
    {
      spawnThread();
    }
  }

  void spawnThread()
  {
    workerThreads.emplace_back([this]()
                               {
            while (!stopper.stopSignal->load()) {
                std::unique_ptr<Request> request;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    queueCondVar.wait(lock, [this]() {
                        return !requestQueue.empty() || stopper.stopSignal->load();
                    });

                    if (!requestQueue.empty()) {
                        request = std::move(requestQueue.front());
                        requestQueue.pop();
                    }
                }

                if (request) {
                    try {
                        processFunction(request.get(), stopper);
                    } catch (const std::exception& e) {
                        std::cerr << "Exception: " << e.what() << std::endl;
                    }
                }
            } });
  }

  void runTask(Request *rawRequest) noexcept
  {
    try
    {
      std::unique_ptr<Request> request(rawRequest);
      std::unique_lock<std::mutex> lock(queueMutex);
      requestQueue.push(std::move(request));
      queueCondVar.notify_one();
    }
    catch (const std::exception &e)
    {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
  }

  void increaseThreadCount(size_t additionalThreads)
  {
    for (size_t i = 0; i < additionalThreads; ++i)
    {
      spawnThread();
    }
    threadCount += additionalThreads;
  }

  void stop()
  {
    stopper.stopSignal->store(true);
    queueCondVar.notify_all();

    for (auto &worker : workerThreads)
    {
      if (worker.joinable())
      {
        worker.join();
      }
    }
  }

  ~TaskProcessor()
  {
    stop();
  }
};

int main()
{
  Stopper globalStopper;
  TaskProcessor<decltype(ProcessRequest) *> processor(ProcessRequest);

  auto start = std::chrono::high_resolution_clock::now();
  auto now = start;

  while (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() < 30)
  {
    try
    {
      Request *req = GetRequest(globalStopper);
      if (req)
      {
        processor.runTask(req);
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
    now = std::chrono::high_resolution_clock::now();
  }

  globalStopper.stopSignal->store(true);
  processor.stop();

  return 0;
}