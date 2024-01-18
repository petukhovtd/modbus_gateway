#include <common/rtu_creator.h>

#include <common/logger.h>

#include <csignal>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace test {

void RtuCreator::Run() {
  pid_ = fork();
  if (0 == pid_) {
    const std::string cmd = "/usr/bin/socat";
    std::string in = "pty,raw,echo=0,link=" + deviceIn;
    std::string out = "pty,raw,echo=0,link=" + deviceOut;
    char *const args[] = {(char *) "-d", (char *) "-d", in.data(), out.data(), nullptr};
    MG_TRACE("RtuCreator::Run: start socat in ({})", getpid());
    int res = execv(cmd.c_str(), args);
    if (-1 == res) {
      MG_ERROR("RtuCreator::Run: start socat in ({}) error:", getpid(), std::strerror(errno));
    } else {
      MG_TRACE("RtuCreator::Run: stop socat in ({})", getpid());
    }
    exit(res);
  } else if (-1 == pid_) {
    MG_CRIT("RtuCreator::Run: fork error: {}", std::strerror(errno));
    throw std::runtime_error("fork error");
  }
  MG_INFO("RtuCreator::Run: child {}", pid_);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void RtuCreator::Stop() {
  if (pid_) {
    MG_INFO("RtuCreator::Stop: send  SIGINT to {}", pid_);
    int res = kill(pid_, SIGINT);
    if (-1 == res) {
      MG_CRIT("RtuCreator::Stop: send  SIGINT to {} error: ", pid_, std::strerror(errno));
    }
    res = waitpid(pid_, nullptr, 0);
    if (-1 == res) {
      MG_CRIT("RtuCreator::Stop: waitpid {} error: {}", pid_, std::strerror(errno));
    }
  }
}

}// namespace test
