#ifndef AMANOGAWA_EXECUTOR_CPP
#define AMANOGAWA_EXECUTOR_CPP

#include <string>
#include <vector>
#include <dlfcn.h>
#include <iostream>
#include "arrow/api.h"
#include "amanogawa/core/confing.h"
#include "amanogawa/include/common.h"
#include "amanogawa/include/source.h"
#include "amanogawa/include/flow.h"
#include "amanogawa/include/sink.h"
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
  //const auto lib_format = dlopen(("libformat_csv." + ext).c_str(), mode);
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

  const auto spring = (decltype(amanogawa::spring) *)
      dlsym(lib_source, source_spring);
  const auto flow = (decltype(amanogawa::flow) *)
      dlsym(lib_flow, flow_flow);
  const auto drain = (decltype(amanogawa::drain) *)
      dlsym(lib_sink, sink_drain);

  if (spring == nullptr) {
    printf("%s\n", dlerror());
  }
  if (flow == nullptr) {
    printf("%s\n", dlerror());
  }
  if (drain == nullptr) {
    printf("%s\n", dlerror());
  }

  auto data = spring("raw.csv");
  printf("a\n");
  auto transformed = flow(data);
  printf("a\n");
  drain("result", transformed);
  printf("a\n");

  dlclose(lib_source);
  dlclose(lib_flow);
  dlclose(lib_sink);
}
}
}

#ifdef PYTHON
PYBIND11_MODULE(amanogawa, m) {
  m.doc() = "pybind11 exampleuuuuu";
  m.def("execute", &execute, "executeeeeeee");
}
#endif

#endif