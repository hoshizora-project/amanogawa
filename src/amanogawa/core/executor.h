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
  std::unordered_map<std::string, std::vector<std::shared_ptr<arrow::Table>>>
      data_slot;
  for (const auto &component_pair : flow_graph->sorted_all_wo_synonym) {
    const auto id = component_pair.first;
    const auto component = component_pair.second;

    if (component->clazz == string::clazz::_source) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::SourcePlugin>(component->plugin);
      data_slot[id].emplace_back(plugin->spring());
    } else if (component->clazz == string::clazz::_flow) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::FlowPlugin>(component->plugin);
      data_slot[id].emplace_back(
          plugin->flow(data_slot[component->prev.front()->id].front()));
    } else if (component->clazz == string::clazz::_branch) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::BranchPlugin>(component->plugin);
      const auto results =
          plugin->branch(data_slot[component->prev.front()->id].front());
      for (const auto &result : *results) {
        data_slot[id].emplace_back(result); // synonymを使う
      }
    } else if (component->clazz == string::clazz::_confluence) {
    } else if (component->clazz == string::clazz::_sink) {
      const auto plugin =
          std::dynamic_pointer_cast<plugin::SinkPlugin>(component->plugin);
      auto x = component->prev.front()->id; // id_name_blood
      auto y = data_slot[component->prev.front()->id];
      auto z = y.front(); // ここをfrontではなくsynonym指定で取るようにする
      // std::unordered_map<std::string,
      //   std::unordered_map<std::string, std::shared_ptr<arrow::Table>>> data_slot
      plugin->drain(data_slot[component->prev.front()->id].front());
    } else {
    }
  }

  return nullptr;
}
} // namespace core
} // namespace amanogawa

#endif