// STL
#include <atomic>
#include <thread>
#include <functional>

class WorkingThread {
public:
  explicit WorkingThread(std::function<void()> func, std::chrono::microseconds internal) noexcept;

  ~WorkingThread() noexcept;

private:
  std::chrono::microseconds mInternal;
  std::atomic_bool mWorking = false;
  std::thread mThread;

};

