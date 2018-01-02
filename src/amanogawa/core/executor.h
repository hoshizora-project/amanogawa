#ifndef AMANOGAWA_EXECUTOR_CPP
#define AMANOGAWA_EXECUTOR_CPP

#include "amanogawa/core/confing.h"
#include "amanogawa/core/dl.h"
#include "amanogawa/core/util.h"
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
  const auto logger = get_logger("executor");
  auto conf = Config::load_from_file(config_file);

  const auto lib_source = DL::open("source_file");
  // const auto lib_format = DL::open("format_csv");
  const auto lib_flow = DL::open("flow_example_add");
  const auto lib_sink = DL::open("sink_file");

  const auto get_source_plugin = lib_source->sym<plugin::get_source_plugin_t>(
      func_name::get_source_plugin);
  // const auto get_format_plugin =
  //    lib_format->sym<plugin::get_format_plugin_t>(func_name::get_format_plugin);
  const auto get_flow_plugin =
      lib_flow->sym<plugin::get_flow_plugin_t>(func_name::get_flow_plugin);
  const auto get_sink_plugin =
      lib_sink->sym<plugin::get_sink_plugin_t>(func_name::get_sink_plugin);

  const auto &source_plugin = get_source_plugin(conf);
  // const auto &format_plugin = get_format_plugin(conf);
  const auto &flow_plugin = get_flow_plugin(conf);
  const auto &sink_plugin = get_sink_plugin(conf);

  auto data = source_plugin->spring("raw.csv");
  logger->info("Source-phase finished");
  auto transformed = flow_plugin->flow(data);
  logger->info("Flow-phase finished");
  sink_plugin->drain("result", transformed);
  logger->info("Sink-phase finished");
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