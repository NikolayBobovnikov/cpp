#ifndef REQUEST_PROCESSOR_H
#define REQUEST_PROCESSOR_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <process_request_func.h>

class RequestProcessor
{
 public:
  // implicit intended
  RequestProcessor(ProcessRequestFunc func, size_t initialThreadCount = 2);
  ~RequestProcessor();

  RequestProcessor(const RequestProcessor &) = delete;
  RequestProcessor &operator=(const RequestProcessor &) = delete;
  RequestProcessor(RequestProcessor &&) = delete;
  RequestProcessor &operator=(const RequestProcessor &&) = delete;

  void process(Request *rawRequest) noexcept;

  void addThreads(size_t additionalThreads);

  void stop();

 private:
  void spawnThread();

  std::queue<std::unique_ptr<Request>> m_requests;
  std::vector<std::thread> m_threads;
  std::mutex m_mtx;
  std::condition_variable m_cv;
  Stopper m_stopper;
  ProcessRequestFunc m_processFunction;
  size_t m_threadsCount;
};

#endif /* REQUEST_PROCESSOR_H */
