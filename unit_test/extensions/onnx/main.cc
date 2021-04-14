#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Main
#include <boost/test/unit_test.hpp>


#include "global_fixture.h"

#include <memory>

#include <core/session/onnxruntime_cxx_api.h>

#include "onnx_extension.h"

GlobalConfig::GlobalConfig()
{
  instance() = this;
  reinforcement_learning::onnx::register_onnx_factory();
}

GlobalConfig*& GlobalConfig::instance() {
    static GlobalConfig* s_inst = nullptr;
    return s_inst;
}

const Ort::MemoryInfo& GlobalConfig::get_memory_info()
{
    if (TestMemoryInfo == nullptr) {
        TestMemoryInfo = std::unique_ptr<Ort::MemoryInfo>(new Ort::MemoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault).release()));
    }
    return *TestMemoryInfo;
}

BOOST_GLOBAL_FIXTURE(GlobalConfig);
