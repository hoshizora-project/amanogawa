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
    for (auto &component : all_wo_synonym) {
      delete component.second;
    }
  }

  // need to be destructed *AFTER* ~FlowGraph()
  std::unordered_map<std::string, std::unique_ptr<core::DL>> libs;

  std::map<std::string, Component *> all; // [id (including synonym), component]
  std::map<std::string, Component *> all_wo_synonym; // [id, component]
  std::vector<std::pair<std::string, Component *>>
      sorted_all_wo_synonym; // sorted [id, component]

  std::vector<Component *> sources;
  std::vector<Component *> flows;
  std::vector<Component *> branches;
  std::vector<Component *> confluences;
  std::vector<Component *> sinks;

  void topological_indexing() {
    std::deque<Component *> starting_points{sources.begin(), sources.end()};
    uint32_t index = 0;
    while (!starting_points.empty()) {
      const auto starting_point = starting_points.at(0);
      starting_point->index = index++;
      for (const auto &next : starting_point->next) {
        if (next->prev.empty() // no incoming edge
            || std::all_of(next->prev.begin(), next->prev.end(),
                           [](const auto ngh) {
                             return ngh->index >= 0;
                           })) { // all incoming edges have been indexed
          starting_points.emplace_back(next);
        }
      }
      starting_points.pop_front();
    }

    if (index != all_wo_synonym.size()) {
      throw std::runtime_error("Not DAG");
    }
  }

  // FIXME: Too dirty (cast, code repetition), no validation
  static std::unique_ptr<FlowGraph> make(const config_t &config) {
    const logger_t logger = get_logger("flow_graph::make");

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

        const auto plugin_type = clazz + "_" + type;
        if (!flow_graph->libs.count(plugin_type)) {
          flow_graph->libs.emplace(plugin_type, core::DL::open(plugin_type));
        }

        if (clazz == string::clazz::_source) {
          const auto get_source_plugin =
              flow_graph->libs.at(plugin_type)
                  ->sym<plugin::get_source_plugin_t>(string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_source_plugin(id, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->all_wo_synonym.emplace(id, component);
          flow_graph->sources.emplace_back(component);
        } else if (clazz == string::clazz::_flow) {
          const auto from =
              *id_table->get_as<std::string>(string::keyword::from);
          const auto get_flow_plugin =
              flow_graph->libs.at(plugin_type)
                  ->sym<plugin::get_flow_plugin_t>(string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_flow_plugin(id, from, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->all_wo_synonym.emplace(id, component);
          flow_graph->flows.emplace_back(component);
        } else if (clazz == string::clazz::_branch) {
          const auto from =
              *id_table->get_as<std::string>(string::keyword::from);
          const auto get_branch_plugin =
              flow_graph->libs.at(plugin_type)
                  ->sym<plugin::get_branch_plugin_t>(string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_branch_plugin(id, from, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->all_wo_synonym.emplace(id, component);
          flow_graph->branches.emplace_back(component);

          // Register branch synonym
          const auto tos =
              config->get_by_id(id)->get_table_array(string::keyword::to);
          for (const auto &to : *tos) {
            const auto synonym = *to->get_as<std::string>("name");
            flow_graph->all.emplace(synonym, component);
          }
        } else if (clazz == string::clazz::_confluence) {
          const auto froms = id_table->get_table_array(string::keyword::from);
          const auto from_left =
              *(*froms->begin())->get_as<std::string>("name");
          const auto from_right =
              *(*(froms->begin() + 1))->get_as<std::string>("name");
          const auto get_confluence_plugin =
              flow_graph->libs.at(plugin_type)
                  ->sym<plugin::get_confluence_plugin_t>(
                      string::func::get_plugin);
          const auto component = new Component(
              id, type, clazz,
              std::static_pointer_cast<plugin::Plugin>(
                  get_confluence_plugin(id, from_left, from_right, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->all_wo_synonym.emplace(id, component);
          flow_graph->confluences.emplace_back(component);
        } else if (clazz == string::clazz::_sink) {
          const auto from =
              *id_table->get_as<std::string>(string::keyword::from);
          const auto get_sink_plugin =
              flow_graph->libs.at(plugin_type)
                  ->sym<plugin::get_sink_plugin_t>(string::func::get_plugin);
          const auto component =
              new Component(id, type, clazz,
                            std::static_pointer_cast<plugin::Plugin>(
                                get_sink_plugin(id, from, config)));
          flow_graph->all.emplace(id, component);
          flow_graph->all_wo_synonym.emplace(id, component);
          flow_graph->sinks.emplace_back(component);
        } else {
        }
      }
    }

    const auto log_edge = [&](const auto &from, const auto &from_synonym,
                              const auto to) {
      if (from == from_synonym) {
        logger->info("{} -> {}", from, to);
      } else {
        logger->info("{} -> ({}) -> {}", from, from_synonym, to);
      }
    };

    // `flow_graph->{prev,next}` has NOT been set here
    // Construct doubly linked flow graph
    for (const auto &component_pair : flow_graph->all_wo_synonym) {
      const auto component = component_pair.second;
      const auto clazz = component->clazz;

      if (clazz == string::clazz::_source) {
      } else if (clazz == string::clazz::_flow) {
        const auto plugin = plugin::as_flow(component->plugin);
        const auto prev_component = flow_graph->all[plugin->from];
        prev_component->next.emplace_back(component);
        component->prev.emplace_back(prev_component);

        log_edge(prev_component->id, plugin->from, component->id);
      } else if (clazz == string::clazz::_branch) {
        const auto plugin = plugin::as_branch(component->plugin);
        const auto prev_component = flow_graph->all[plugin->from];
        prev_component->next.emplace_back(component);
        component->prev.emplace_back(prev_component);

        log_edge(prev_component->id, plugin->from, component->id);
      } else if (clazz == string::clazz::_confluence) {
        const auto plugin = plugin::as_confluence(component->plugin);
        const std::vector<Component *> prev_components = {
            flow_graph->all[plugin->from_left],
            flow_graph->all[plugin->from_right]};
        for (const auto &prev_component : prev_components) {
          prev_component->next.emplace_back(component);
          component->prev.emplace_back(prev_component);
        }

        log_edge(prev_components.at(0)->id, plugin->from_left, component->id);
        log_edge(prev_components.at(1)->id, plugin->from_right, component->id);
      } else if (clazz == string::clazz::_sink) {
        const auto plugin = plugin::as_sink(component->plugin);
        const auto prev_component = flow_graph->all[plugin->from];
        prev_component->next.emplace_back(component);
        component->prev.emplace_back(prev_component);

        log_edge(prev_component->id, plugin->from, component->id);
      } else {
      }
    }

    // Set all dependencies
    flow_graph->topological_indexing();
    std::copy(flow_graph->all_wo_synonym.begin(),
              flow_graph->all_wo_synonym.end(),
              std::back_inserter(flow_graph->sorted_all_wo_synonym));
    std::sort(flow_graph->sorted_all_wo_synonym.begin(),
              flow_graph->sorted_all_wo_synonym.end(),
              [](const auto &l, const auto &r) {
                return l.second->index < r.second->index;
              });

    for (const auto &component_pair : flow_graph->sorted_all_wo_synonym) {
      const auto component = component_pair.second;
      const auto clazz = component->clazz;

      if (clazz == string::clazz::_source) {
      } else if (clazz == string::clazz::_flow) {
        const auto prev_component =
            component->prev.at(0); // source has only 1 prev component
        component->deps.emplace(prev_component);
        std::copy(prev_component->deps.begin(), prev_component->deps.end(),
                  std::inserter(component->deps, component->deps.end()));
      } else if (clazz == string::clazz::_branch) {
        const auto prev_component =
            component->prev.at(0); // branch has only 1 prev component
        component->deps.emplace(prev_component);
        std::copy(prev_component->deps.begin(), prev_component->deps.end(),
                  std::inserter(component->deps, component->deps.end()));
      } else if (clazz == string::clazz::_confluence) {
        const auto prev_components = component->prev;
        assert(prev_components.size() == 2);
        for (const auto prev_component : prev_components) {
          component->deps.emplace(prev_component);
          std::copy(prev_component->deps.begin(), prev_component->deps.end(),
                    std::inserter(component->deps, component->deps.end()));
        }
      } else if (clazz == string::clazz::_sink) {
        const auto prev_component =
            component->prev.at(0); // sink has only 1 prev component
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
