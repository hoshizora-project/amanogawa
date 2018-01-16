#ifndef AMANOGAWA_EXECUTOR_H
#define AMANOGAWA_EXECUTOR_H

#include "amanogawa/core/dl.h"
#include "amanogawa/include/api.h"
#include "arrow/api.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace amanogawa {
namespace core {
void *execute(const config_t &config) {
  const auto logger = get_logger("executor");

  const auto plugins_meta = config->read_plugins();

  std::vector<std::unique_ptr<DL>> libs;
  libs.reserve(plugins_meta.size());
  std::vector<plugin::get_source_plugin_return_t> source_plugins;
  std::vector<plugin::get_flow_plugin_return_t> flow_plugins;
  // std::vector<plugin::get_branch_plugin_return_t> branch_plugins;
  // std::vector<plugin::get_confluence_plugin_return_t> confluence_plugins;
  std::vector<plugin::get_sink_plugin_return_t> sink_plugins;
  for (const auto &meta : plugins_meta) {
    const auto clazz = meta.first;
    for (const auto &plugin : meta.second) {
      const auto type = plugin.first;
      libs.emplace_back(DL::open(clazz + "_" + type));
      if (clazz == string::clazz::_source) {
        const auto get_source_plugin =
            libs.back()->sym<plugin::get_source_plugin_t>(
                string::func::get_plugin);
        for (const auto &id_from : plugin.second) {
          source_plugins.emplace_back(get_source_plugin(id_from.first, config));
        }
      } else if (clazz == string::clazz::_flow) {
        const auto get_flow_plugin =
            libs.back()->sym<plugin::get_flow_plugin_t>(
                string::func::get_plugin);
        for (const auto &id_from : plugin.second) {
          flow_plugins.emplace_back(
              get_flow_plugin(id_from.first, id_from.second.at(0), config));
        }
      } else if (clazz == string::clazz::_branch) {

      } else if (clazz == string::clazz::_confluence) {

      } else if (clazz == string::clazz::_sink) {
        const auto get_sink_plugin =
            libs.back()->sym<plugin::get_sink_plugin_t>(
                string::func::get_plugin);
        for (const auto &id_from : plugin.second) {
          sink_plugins.emplace_back(
              get_sink_plugin(id_from.first, id_from.second.at(0), config));
        }
      }
    }
  }

  auto data = source_plugins.at(0)->spring();
  logger->info("Source-phase finished");
  auto transformed = flow_plugins.at(0)->flow(data);
  logger->info("Flow-phase finished");
  auto res = sink_plugins.at(0)->drain(transformed);
  logger->info("Sink-phase finished");

  return res;
}
} // namespace core
} // namespace amanogawa

#endif