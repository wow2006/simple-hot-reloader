// OS
#include <unistd.h>
// inotify
#include <sys/inotify.h>
// fmt
#include <fmt/printf.h>
// Internal
#include "watchFile.hpp"
#include "workingThread.hpp"

constexpr auto EVENT_SIZE = sizeof(inotify_event);
constexpr auto EVENT_BUF_LENGTH = (1024 * (EVENT_SIZE + 16));

WatchFile::WatchFile(std::initializer_list<std::string_view> files) noexcept {
  mHandler = inotify_init1(IN_NONBLOCK);
  if (mHandler < 0) {
    fmt::print("ERROR: Can not initialize 'inotify'\n");
    return;
  }

  for (auto file : files) {
    const auto filePath = std::filesystem::path(file);
    if (!std::filesystem::exists(filePath)) {
      fmt::print("ERROR: %s file is not exists\n", filePath.string());
      continue;
    }
    const auto wd = inotify_add_watch(mHandler, filePath.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
    mFilesToWatch.emplace_back(filePath, wd);
  }

  using namespace std::chrono_literals;

  mWorkingThread = std::make_unique<WorkingThread>([this]() {
      std::array<char, EVENT_BUF_LENGTH> buffer;
      const auto length = read(mHandler, buffer.data(), EVENT_BUF_LENGTH);
      for(int index = 0; index < length;) {
        const auto pEvent = reinterpret_cast<inotify_event *>(&buffer[index]);
        if(!(pEvent->mask & IN_ISDIR) && pEvent->mask & IN_MODIFY) {
          const auto wd = pEvent->wd;
          const auto itr = std::find_if(
              std::cbegin(mFilesToWatch), std::cend(mFilesToWatch), [wd](const auto& item) {
                return item.second == wd;
              }
          );
          fmt::print(R"(File {} updated)", itr->first);
          fmt::print("\n");
        }
        index += EVENT_SIZE + pEvent->len;
      }
  }, 500ms);
}

WatchFile::~WatchFile() noexcept {
  mWorkingThread.reset();
  for (const auto &[filePath, wd] : mFilesToWatch) {
    inotify_rm_watch(mHandler, wd);
  }
}
