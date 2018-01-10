#ifndef AMANOGAWA_EXECUTOR_H
#define AMANOGAWA_EXECUTOR_H

#include "arrow/api.h"
#include "amanogawa/include/api.h"
#include "amanogawa/core/dl.h"
#include <iostream>
#include <string>
#include <vector>

namespace amanogawa {
namespace core {
void execute(const core::Config &config) {
  const auto logger = get_logger("executor");

  const auto lib_source =
      DL::open("source_" + *(config.source->get_as<std::string>("type")));
  // const auto lib_source_format = DL::open(
  //    "source_format_" +
  //    *(config.source->get_qualified_as<std::string>("format.type")));
  const auto lib_flow =
      DL::open("flow_" + *(config.flow->get_as<std::string>("type")));
  const auto lib_sink =
      DL::open("sink_" + *(config.sink->get_as<std::string>("type")));

  const auto get_source_plugin = lib_source->sym<plugin::get_source_plugin_t>(
      func_name::get_source_plugin);
  // const auto get_format_plugin =
  //    lib_format->sym<plugin::get_format_plugin_t>(func_name::get_format_plugin);
  const auto get_flow_plugin =
      lib_flow->sym<plugin::get_flow_plugin_t>(func_name::get_flow_plugin);
  const auto get_sink_plugin =
      lib_sink->sym<plugin::get_sink_plugin_t>(func_name::get_sink_plugin);

  const auto &source_plugin = get_source_plugin(config);
  // const auto &format_plugin = get_format_plugin(config);
  const auto &flow_plugin = get_flow_plugin(config);
  const auto &sink_plugin = get_sink_plugin(config);

  auto data = source_plugin->spring();
  logger->info("Source-phase finished");
  auto transformed = flow_plugin->flow(data);
  logger->info("Flow-phase finished");
  sink_plugin->drain(transformed);
  logger->info("Sink-phase finished");
}
} // namespace core
} // namespace amanogawa

#endif