#pragma once

#include <memory>

namespace Ort {
  struct MemoryInfo;
}

struct GlobalConfig {
  GlobalConfig();
  ~GlobalConfig() = default;

  static GlobalConfig*& instance();
  const Ort::MemoryInfo& get_memory_info();

private:
  std::unique_ptr<Ort::MemoryInfo> TestMemoryInfo;
};
