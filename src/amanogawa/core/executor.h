#ifndef AMANOGAWA_EXECUTOR_H
#define AMANOGAWA_EXECUTOR_H

#include "amanogawa/core/dl.h"
#include "amanogawa/include/api.h"
#include "amanogawa/include/flow_graph.h"
#include "arrow/api.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace amanogawa {
namespace core {
void *execute(const config_t &config) {
#ifdef SPDLOG_DEBUG_ON
  spdlog::set_level(spdlog::level::debug); // should not be necessary...
#endif

  const auto logger = get_logger("executor");

  const auto flow_graph = FlowGraph::make(config);

  // [real id, [maybe synonym, data]]
  std::unordered_map<
      std::string,
      std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>
      data_slot;
  for (const auto &component_pair : flow_graph->sorted_all_wo_synonym) {
    const auto id = component_pair.first;
    const auto component = component_pair.second;

    if (component->clazz == string::clazz::source) {
      const auto plugin = plugin::as_source(component->plugin);
      data_slot[id].emplace(id, plugin->spring());

      SPDLOG_DEBUG(logger, "slotted: ({}) as ({})", id, id);
    } else if (component->clazz == string::clazz::flow) {
      const auto plugin = plugin::as_flow(component->plugin);
      // `component->prev.at(0)`: flow has only 1 prev component
      const auto results =
          plugin->flow(data_slot[component->prev.at(0)->id][plugin->from]);
      for (const auto &result : *results) {
        data_slot[id].emplace(result);

        SPDLOG_DEBUG(logger, "slotted: ({}) as ({})", id, result.first);
      }
    } else if (component->clazz == string::clazz::branch) {
      const auto plugin = plugin::as_branch(component->plugin);
      // `component->prev.at(0)`: branch has only 1 prev component
      const auto results =
          plugin->branch(data_slot[component->prev.at(0)->id][plugin->from]);
      for (const auto &result : *results) {
        data_slot[id].emplace(result);

        SPDLOG_DEBUG(logger, "slotted: ({}) as ({})", id, result.first);
      }
    } else if (component->clazz == string::clazz::confluence) {
      const auto plugin = plugin::as_confluence(component->plugin);
      const auto result = plugin->join(
          data_slot[component->prev.at(0)->id][plugin->from_left],
          data_slot[component->prev.at(1)->id][plugin->from_right]);
      data_slot[id].emplace(id, result);

      SPDLOG_DEBUG(logger, "slotted: ({}) as ({})", id, id);
    } else if (component->clazz == string::clazz::sink) {
      const auto plugin = plugin::as_sink(component->plugin);
      // `component->prev.at(0)`: sink has only 1 prev component
      plugin->drain(data_slot[component->prev.at(0)->id][plugin->from]);
    } else {
    }
  }

  return nullptr;
}
} // namespace core
} // namespace amanogawa

#endif