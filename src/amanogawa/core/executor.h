#ifndef AMANOGAWA_EXECUTOR_CPP
#define AMANOGAWA_EXECUTOR_CPP

#include "amanogawa/core/confing.h"
#include "amanogawa/include/common.h"
#include "amanogawa/include/flow.h"
#include "amanogawa/include/sink.h"
#include "amanogawa/include/source.h"
#include "arrow/api.h"
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <vector>
#ifdef PYTHON
#include <pybind11/pybind11.h>
#endif

namespace amanogawa {
namespace core {

void execute(const std::string &config_file) {
  auto conf = load_config(config_file);

#ifdef __APPLE__
  constexpr auto mode = RTLD_LOCAL; // mac
  const std::string ext = "dylib";
#elif __linux__
  constexpr auto mode = RTLD_LAZY; // linux
  const std::string ext = "so";
#endif
  const auto lib_source = dlopen(("libsource_file." + ext).c_str(), mode);
  // const auto lib_format = dlopen(("libformat_csv." + ext).c_str(), mode);
  const auto lib_flow = dlopen(("libflow_example_add." + ext).c_str(), mode);
  const auto lib_sink = dlopen(("libsink_file." + ext).c_str(), mode);

  if (lib_source == nullptr) {
    printf("%s\n", dlerror());
  }
  if (lib_flow == nullptr) {
    printf("%s\n", dlerror());
  }
  if (lib_sink == nullptr) {
    printf("%s\n", dlerror());
  }

  const auto get_source_plugin = (amanogawa::plugin::get_source_plugin_t)dlsym(
      lib_source, func_name::get_source_plugin);
  const auto get_flow_plugin = (amanogawa::plugin::get_flow_plugin_t)dlsym(
      lib_flow, func_name::get_flow_plugin);
  const auto get_sink_plugin = (amanogawa::plugin::get_sink_plugin_t)dlsym(
      lib_sink, func_name::get_sink_plugin);

  if (get_source_plugin == nullptr) {
    printf("%s\n", dlerror());
  }
  if (get_flow_plugin == nullptr) {
    printf("%s\n", dlerror());
  }
  if (get_sink_plugin == nullptr) {
    printf("%s\n", dlerror());
  }

  const auto &source_plugin = get_source_plugin();
  const auto &flow_plugin = get_flow_plugin();
  const auto &sink_plugin = get_sink_plugin();

  auto data = source_plugin->spring("raw.csv");
  printf("a\n");
  auto transformed = flow_plugin->flow(data);
  printf("a\n");
  sink_plugin->drain("result", transformed);
  printf("a\n");

  dlclose(lib_source);
  dlclose(lib_flow);
  dlclose(lib_sink);
}

#ifdef PYTHON
PYBIND11_MODULE(amanogawa, m) {
  m.doc() = "pybind11 exampleuuuuu";
  m.def("execute", &execute, "executeeeeeee");
}
#endif
} // namespace core
} // namespace amanogawa

#endif