#pragma once
// STL
#include <atomic>
#include <thread>
#include <filesystem>
#include <functional>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

using WatchEventID = int;

struct WatchHandler {
  explicit WatchHandler(int handler) noexcept;

  ~WatchHandler() noexcept;

  operator bool() const {
    return mHandler != -1;
  }

  operator int() const {
    return mHandler;
  }

private:
  int mHandler = -1;

};

using fileIterator = std::vector<std::filesystem::path>::iterator;

class watch final {
public:
  watch();

  ~watch() noexcept;

  void addFileToWatch(std::filesystem::path fileName, std::function<void()> function);

  void removeWatchedFile(const std::filesystem::path& fileName);

  void createRunnerThread();

private:
  std::atomic_bool mRunning;
  WatchHandler mHandler;
  std::thread mRunningThread;
  std::vector<std::filesystem::path> mFiles = {};
  std::unordered_map<WatchEventID, std::pair<fileIterator, std::function<void()>>> mCallbacks;

};

