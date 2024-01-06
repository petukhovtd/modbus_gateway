#include <common/context_runner.h>

#include <common/logger.h>

namespace test {

ContextRunner::ContextRunner(size_t threads)
    : context_(std::make_shared<modbus_gateway::ContextPtr::element_type>(threads)), work_(context_->get_executor()), contextThread_() {
}

ContextRunner::~ContextRunner() {
  Stop();
}

void ContextRunner::Run() {
  contextThread_ = std::thread([this]() {
    MG_INFO("ContextRunner::Run: context run")
    context_->run();
    MG_INFO("ContextRunner::Run: context run end")
  });
}

modbus_gateway::ContextPtr ContextRunner::GetContext() const {
  return context_;
}

void ContextRunner::Stop() {
  MG_INFO("ContextRunner::Stop: context stop")
  context_->stop();
  if (contextThread_.joinable()) {
    MG_DEBUG("ContextRunner::Stop: context join")
    contextThread_.join();
    MG_DEBUG("ContextRunner::Stop: context join end")
  }
}

}// namespace test