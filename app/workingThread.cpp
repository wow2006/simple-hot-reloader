// Internal
#include "workingThread.hpp"

WorkingThread::WorkingThread(std::function<void()> func, std::chrono::microseconds internal) noexcept
    : mInternal{internal},
      mThread{std::thread(
          [this](const auto function) {
            mWorking = true;
            while (mWorking) {
              const auto t1 = std::chrono::steady_clock::now();
              function();
              const auto t2 = std::chrono::steady_clock::now();
              const auto duration =
                  std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
              if (duration < mInternal) {
                std::this_thread::sleep_for(mInternal - duration);
              }
            }
          },
          func)} {}

WorkingThread::~WorkingThread() noexcept {
  mWorking = false;
  mThread.join();
}
