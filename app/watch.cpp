// STL
#include <cstring>
#include <iostream>
// inotify
#include <sys/inotify.h>
// OS
#include <poll.h>
#include <unistd.h>
// Internal
#include "watch.hpp"
// NOTE: https://man7.org/linux/man-pages/man7/inotify.7.html
// NOTE: https://man7.org/linux/man-pages/man3/strerror.3.html
// https://docs.microsoft.com/en-us/windows/win32/fileio/obtaining-directory-change-notifications?redirectedfrom=MSDN

WatchHandler::WatchHandler(int handler) noexcept : mHandler{handler} {}

WatchHandler::~WatchHandler() noexcept {
  if (mHandler != -1) {
    close(mHandler);
  }
}

watch::watch() : mHandler(inotify_init1(IN_NONBLOCK)) {
  if (!mHandler) {
    throw std::runtime_error("inotify_init1");
  }
}

watch::~watch() noexcept {
  mRunning = false;
  mRunningThread.join();
}

void watch::createRunnerThread() {
  if (!mRunning) {
    mRunningThread = std::thread([this]() {
      pollfd fds{.fd = mHandler, .events = POLLIN};
      nfds_t nfds = 1;
      while (mRunning) {
        const auto poll_num = poll(&fds, nfds, -1);
        if (poll_num == -1) {
          if (errno == EINTR) {
            continue;
          }
          throw std::runtime_error("Error accur while poll");
        }
        if (poll_num > 0) {
          if (fds.revents & POLLIN) {
            // handle_events();
          }
        }
      }
    });
  }
}

void watch::addFileToWatch(std::filesystem::path fileName,
                           std::function<void()> function) {
  const auto found =
      std::find(std::cbegin(mFiles), std::cend(mFiles), fileName);
  if (found != std::cend(mFiles)) {
    return;
  }
  const auto wd = inotify_add_watch(mHandler, fileName.c_str(), IN_MODIFY);
  const auto itr = mFiles.insert(std::end(mFiles), fileName);
  mCallbacks.emplace(wd, std::make_pair(itr, function));
}

void watch::removeWatchedFile(const std::filesystem::path &fileName) {}

static void handle_events(int fd, int wd) {
  const inotify_event *event;
  int i;
  ssize_t len;
  char *ptr;
  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  while (true) {
    const auto len = read(fd, buf, sizeof buf);
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    if (len <= 0) {
      break;
    }

    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *)ptr;
      if (event->mask & IN_OPEN)
        printf("IN_OPEN: ");
      if (event->mask & IN_CLOSE_NOWRITE)
        printf("IN_CLOSE_NOWRITE: ");
      if (event->mask & IN_CLOSE_WRITE)
        printf("IN_CLOSE_WRITE: ");

      if (wd == event->wd) {
        std::cout << "event ID: " << event->name << '\n';
      }

      if (event->len) {
        printf("%s", event->name);
      }

      if (event->mask & IN_ISDIR) {
        printf(" [directory]\n");
      } else {
        printf(" [file]\n");
      }
    }
  }
}

int main(int argc, char *argv[]) {
  const auto fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    std::cerr << "ERROR: inotify_init1\n";
    return EXIT_FAILURE;
  }

  const auto wd = inotify_add_watch(fd,
                                    "/home/ahussein/Documents/sourceCode/"
                                    "simpleHotReloader/src/lib/library.cpp",
                                    IN_MODIFY);
  if (wd == -1) {
    std::cerr << "Cannot watch '': " << strerror(errno) << '\n';
    return EXIT_FAILURE;
  }

  pollfd fds;
  nfds_t nfds = 1;
  fds.fd = fd;
  fds.events = POLLIN;
  while (true) {
    const auto poll_num = poll(&fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("poll");
      exit(EXIT_FAILURE);
    }
    if (poll_num > 0) {
      if (fds.revents & POLLIN) {
        handle_events(fd, wd);
      }
    }
  }

  close(fd);

  return EXIT_SUCCESS;
}
