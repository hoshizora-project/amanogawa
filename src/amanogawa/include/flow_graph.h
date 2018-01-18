#ifndef AMANOGAWA_FLOW_GRAPH_H
#define AMANOGAWA_FLOW_GRAPH_H

#include "amanogawa/core/dl.h"
#include "amanogawa/include/api.h"
#include "amanogawa/include/config.h"
#include <algorithm>
#include <deque>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace amanogawa {
// represent node
struct Component {
  int index = -1; // topological
  const std::string id;
  const std::string type;
  const std::string clazz;
  const std::shared_ptr<plugin::Plugin> plugin;
  std::vector<Component *> prev;
  std::vector<Component *> next;
  std::unordered_set<Component *> deps; // TODO

  // We delete all components at `FlowGraph`
  ~Component() = default;

  Component(std::string id, std::string type, std::string clazz,
            std::shared_ptr<plugin::Plugin> plugin)
      : id(std::move(id)), type(std::move(type)), clazz(std::move(clazz)),
        plugin(std::move(plugin)) {}
};

// represent DAG-ish
struct FlowGraph {
  ~FlowGraph() {
    for (auto &component : all) {
      delete component.second;
    }
  }

  // need to be destructed *AFTER* ~FlowGraph()
  std::vector<std::unique_ptr<core::DL>> libs;

  std::map<std::string, Component *> all;
  std::vector<std::pair<std::string, Component *>> sorted_all;

  std::vector<Component *> sources;
  std::vector<Component *> flows;
  std::vector<Component *> branches;
  std::vector<Component *> confluents;
  std::vector<Component *> sinks;

  void topological_indexing() {
    std::unordered_set<Component *> indexed;
    indexed.reserve(all.size());
    std::deque<Component *> starting_points{sources.begin(), sources.end()};
    uint32_t index = 0;
    while (!starting_points.empty()) {
      const auto starting_point = starting_points.front();
      starting_point->index = index++;
      indexed.emplace(starting_point);
      for (const auto &next : starting_point->next) {
        if (next->prev.empty()         // no incoming edge
            || !indexed.count(next)) { // all incoming edges have been indexed
          starting_points.emplace_back(next);
        }
      }
      starting_points.pop_front();
    }

    if (index != all.size()) {
      throw std::runtime_error("Not DAG");
    }
  }

  // FIXME: Too dirty (cast, code repetition), no validation
  static std::unique_ptr<FlowGraph> make(const config_t &config) {
    const auto table = config->table;
    auto flow_graph = std::make_unique<FlowGraph>();

    const std::vector<std::string> clazzes = {
        string::clazz::_source, string::clazz::_flow, string::clazz::_branch,
        string::clazz::_confluence, string::clazz::_sink};

    // Instantiating components
    for (const auto &clazz : clazzes) {
      const auto class_table = table->get_table(clazz);
      if (class_table == nullptr) {
        continue;
      }
      const auto ids = class_table->get_keys();
      for (const auto &id : ids) {
        const auto id_table = class_table->get_table(id);
        const auto type = *id_table->get_as<std::string>(string::keyword::type);

        if (!flow_graph->all.count(type)) {
          flow_graph->libs.emplace_back(core::DL::open(clazz + "_" + type));
        }

        if (clazz == string::clazz::_source) {
          const auto get_source_plugin =
              flow_graph->libs.back()->sym<plugin::get_source_plugin_t>(
                  string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_source_plugin(id, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->sources.emplace_back(component);
        } else if (clazz == string::clazz::_flow) {
          const auto from =
              *id_table->get_as<std::string>(string::keyword::from);
          const auto get_flow_plugin =
              flow_graph->libs.back()->sym<plugin::get_flow_plugin_t>(
                  string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_flow_plugin(id, from, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->flows.emplace_back(component);
        } else if (clazz == string::clazz::_branch) {

        } else if (clazz == string::clazz::_confluence) {

        } else if (clazz == string::clazz::_sink) {
          const auto from =
              *id_table->get_as<std::string>(string::keyword::from);
          const auto get_sink_plugin =
              flow_graph->libs.back()->sym<plugin::get_sink_plugin_t>(
                  string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_sink_plugin(id, from, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->sinks.emplace_back(component);
        } else {
        }
      }
    }

    // Construct doubly linked flow graph
    for (const auto &component_pair : flow_graph->all) {
      const auto component = component_pair.second;
      const auto clazz = component->clazz;

      if (clazz == string::clazz::_source) {
      } else if (clazz == string::clazz::_flow) {
        const auto flow_plugin =
            std::dynamic_pointer_cast<plugin::FlowPlugin>(component->plugin);
        const auto prev_component = flow_graph->all[flow_plugin->from];
        prev_component->next.emplace_back(component);
        component->prev.emplace_back(prev_component);
      } else if (clazz == string::clazz::_branch) {

      } else if (clazz == string::clazz::_confluence) {

      } else if (clazz == string::clazz::_sink) {
        const auto sink_plugin =
            std::dynamic_pointer_cast<plugin::SinkPlugin>(component->plugin);
        const auto prev_component = flow_graph->all[sink_plugin->from];
        prev_component->next.emplace_back(component);
        component->prev.emplace_back(prev_component);
      } else {
      }
    }

    // Set all dependencies
    flow_graph->topological_indexing();
    std::copy(flow_graph->all.begin(), flow_graph->all.end(),
              std::back_inserter(flow_graph->sorted_all));
    std::sort(flow_graph->sorted_all.begin(), flow_graph->sorted_all.end(),
              [](const auto &l, const auto &r) {
                return l.second->index < r.second->index;
              });

    for (const auto &component_pair : flow_graph->sorted_all) {
      const auto component = component_pair.second;
      const auto clazz = component->clazz;

      if (clazz == string::clazz::_source) {
      } else if (clazz == string::clazz::_flow) {
        const auto prev_component =
            component->prev.front(); // source has only 1 prev component
        component->deps.emplace(prev_component);
        std::copy(prev_component->deps.begin(), prev_component->deps.end(),
                  std::inserter(component->deps, component->deps.end()));
      } else if (clazz == string::clazz::_branch) {

      } else if (clazz == string::clazz::_confluence) {

      } else if (clazz == string::clazz::_sink) {
        const auto prev_component =
            component->prev.front(); // sink has only 1 prev component
        component->deps.emplace(prev_component);
        std::copy(prev_component->deps.begin(), prev_component->deps.end(),
                  std::inserter(component->deps, component->deps.end()));
      } else {
      }
    }

    return flow_graph;
  }
};
} // namespace amanogawa

#endif // AMANOGAWA_FLOW_GRAPH_H
