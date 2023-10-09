#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <vector>
#include <memory>
#include <stdexcept>

// Dummy Request class
class Request
{
};

// Dummy GetRequest function
Request *GetRequest(std::atomic_bool &stopSignal) noexcept
{
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return stopSignal ? nullptr : new Request();
}

// Dummy ProcessRequest function
void ProcessRequest(std::unique_ptr<Request> request, std::atomic_bool &stopSignal) noexcept
{
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

class Stopper
{
public:
  std::atomic_bool stopSignal{false};
};

class ThreadPool
{
private:
  std::queue<std::unique_ptr<Request>> requestQueue;
  std::vector<std::thread> workerThreads;
  std::mutex queueMutex;
  std::condition_variable queueCondVar;
  Stopper stopper;
  size_t threadCount;

public:
  ThreadPool(size_t initialThreadCount = 2)
      : threadCount(initialThreadCount)
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
            while (!stopper.stopSignal) {
                std::unique_ptr<Request> request;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    queueCondVar.wait(lock, [this]() {
                        return !requestQueue.empty() || stopper.stopSignal;
                    });

                    if (!requestQueue.empty()) {
                        request = std::move(requestQueue.front());
                        requestQueue.pop();
                    }
                }

                if (request) {
                    try {
                        ProcessRequest(std::move(request), stopper.stopSignal);
                    } catch (const std::exception& e) {
                        // Handle exceptions from ProcessRequest
                        std::cerr << "Exception: " << e.what() << std::endl;
                    }
                }
            } });
  }

  void receive(Request *rawRequest) noexcept
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
      // Handle exceptions, if any
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
    stopper.stopSignal = true;
    queueCondVar.notify_all();

    for (auto &worker : workerThreads)
    {
      if (worker.joinable())
      {
        worker.join();
      }
    }
  }

  ~ThreadPool()
  {
    stop();
  }
};

int main()
{
  ThreadPool pool;

  // Run for 30 seconds
  auto start = std::chrono::high_resolution_clock::now();
  auto now = start;
  Stopper stopper;

  while (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() < 30)
  {
    try
    {
      Request *req = GetRequest(stopper.stopSignal);
      if (req)
      {
        pool.receive(req);
      }
    }
    catch (const std::exception &e)
    {
      // Handle exceptions from GetRequest
      std::cerr << "Exception: " << e.what() << std::endl;
    }
    now = std::chrono::high_resolution_clock::now();
  }

  stopper.stopSignal = true;
  pool.stop();

  return 0;
}
