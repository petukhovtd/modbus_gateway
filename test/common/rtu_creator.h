#pragma once

#include <string>

namespace test {

static const std::string deviceIn = "port1";
static const std::string deviceOut = "port2";

class RtuCreator {
public:

  void Run();

  void Stop();


private:
  pid_t pid_ = 0;

};
}

