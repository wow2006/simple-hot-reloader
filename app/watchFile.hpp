#pragma once
// STL
#include <memory>
#include <functional>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <initializer_list>

using INotifyHandler = int;

class WorkingThread;
using Callback = std::function<void()>;

class WatchFile {
public:
  explicit WatchFile(std::initializer_list<std::pair<std::string_view, Callback>> files) noexcept;

  ~WatchFile() noexcept;

private:
  INotifyHandler mHandler = 0;
  std::unique_ptr<WorkingThread> mWorkingThread;
  std::unordered_map<int, std::pair<std::filesystem::path, Callback>> mFilesToWatch;

};

