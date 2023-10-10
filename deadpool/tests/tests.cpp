#include <gtest/gtest.h>

// Assuming the previous code is in a header "task_processor.h"
#include <external_api.h>
#include <request_processor.h>

void LongProcessRequest(Request* req, Stopper s) noexcept
{
  constexpr auto pollTimeout = 100;
  constexpr auto processingTimeout = 10;
  auto start_time = std::chrono::system_clock::now();
  auto end_time = start_time + std::chrono::seconds(processingTimeout);

  while(std::chrono::system_clock::now() < end_time) {
    if(*s.stopSignal) {
      return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(pollTimeout));
  }
}

void IncorrectSignature(Request* req) {}

// Uncommenting this would break the compilation, as intended
// // NOLINTNEXTLINE
// TEST(RequestProcessorTest, IncorrectSignatureTest) {
//     RequestProcessor processor(IncorrectSignature);
// }

// NOLINTNEXTLINE
TEST(RequestProcessorTest, StopSignalTest)
{
  Stopper stopper;
  RequestProcessor processor(LongProcessRequest);
  std::unique_ptr<Request> req(new Request);

  auto start = std::chrono::high_resolution_clock::now();

  try {
    processor.process(req.release());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    *stopper.stopSignal = true;  // send stop signal
  } catch(const std::exception& e) {
    FAIL() << "Unexpected exception: " << e.what();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  EXPECT_LT(duration, 10) << "The task should have been interrupted and exited "
                             "in less than 10 seconds.";
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
