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

    if (component->clazz == string::clazz::_source) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::SourcePlugin>(component->plugin);
      data_slot[id].emplace(id, plugin->spring());

      logger->info("slotted: ({}) as ({})", id, id);
    } else if (component->clazz == string::clazz::_flow) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::FlowPlugin>(component->plugin);
      // `component->prev.front()`: flow has only 1 prev component
      data_slot[id].emplace(
          id,
          plugin->flow(data_slot[component->prev.front()->id][plugin->from]));

      logger->info("slotted: ({}) as ({})", id, id);
    } else if (component->clazz == string::clazz::_branch) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::BranchPlugin>(component->plugin);
      // `component->prev.front()`: branch has only 1 prev component
      const auto results =
          plugin->branch(data_slot[component->prev.front()->id][plugin->from]);
      for (const auto &result : *results) {
        data_slot[id].emplace(result);

        logger->info("slotted: ({}) as ({})", id, result.first);
      }
    } else if (component->clazz == string::clazz::_confluence) {
      const auto plugin = plugin::as_confluence(component->plugin);
      const auto result = plugin->join(
          data_slot[component->prev.at(0)->id][plugin->from_left],
          data_slot[component->prev.at(1)->id][plugin->from_right]);
      data_slot[id].emplace(id, result);

      logger->info("slotted: ({}) as ({})", id, id);
    } else if (component->clazz == string::clazz::_sink) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::SinkPlugin>(component->plugin);
      // `component->prev.front()`: sink has only 1 prev component
      plugin->drain(data_slot[component->prev.front()->id][plugin->from]);
    } else {
    }
  }

  return nullptr;
}
} // namespace core
} // namespace amanogawa

#endif