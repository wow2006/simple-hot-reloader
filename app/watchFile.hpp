#pragma once
// STL
#include <vector>
#include <memory>
#include <filesystem>
#include <string_view>
#include <initializer_list>

using INotifyHandler = int;

class WorkingThread;

class WatchFile {
public:
  explicit WatchFile(std::initializer_list<std::string_view> files) noexcept;

  ~WatchFile() noexcept;

private:
  INotifyHandler mHandler = 0;
  std::unique_ptr<WorkingThread> mWorkingThread;
  std::vector<std::pair<std::filesystem::path, int>> mFilesToWatch;

};

