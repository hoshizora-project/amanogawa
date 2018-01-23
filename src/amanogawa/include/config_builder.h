#ifndef AMANOGAWA_CONFIG_BUILDER_H
#define AMANOGAWA_CONFIG_BUILDER_H

#include "amanogawa/include/api.h"
#include <cpptoml.h>
#include <pybind11/stl.h>
#include <string>
#include <unordered_map>

namespace amanogawa {
// TODO: Add helper function which supports nested and hetero configs
struct ConfigBuilder {
  std::string curr_id; // FIXME: Type as state
  std::unordered_map<std::string, std::string> id_clazz;

  Config::table_t config;

  ConfigBuilder() : config(cpptoml::make_table()) {}

  config_t build() { return std::make_shared<Config>(config); }

  Config::table_t curr_table() {
    return config->get_table(id_clazz.at(curr_id))->get_table(curr_id);
  }

  ConfigBuilder *source(const std::string &id, const std::string &type) {
    auto id_table = cpptoml::make_table();
    id_table->insert(string::keyword::type, type);

    if (config->contains(string::clazz::source)) {
      config->get_table(string::clazz::source)->insert(id, id_table);
    } else {
      auto source_table = cpptoml::make_table();
      source_table->insert(id, id_table);
      config->insert(string::clazz::source, source_table);
    }

    id_clazz.emplace(id, string::clazz::source);
    curr_id = id;
    return this;
  }

  ConfigBuilder *flow(const std::string &id, const std::string &type,
                      const std::string &from) {
    auto id_table = cpptoml::make_table();
    id_table->insert(string::keyword::type, type);
    id_table->insert(string::keyword::from, from);

    if (config->contains(string::clazz::flow)) {
      config->get_table(string::clazz::flow)->insert(id, id_table);
    } else {
      auto flow_table = cpptoml::make_table();
      flow_table->insert(id, id_table);
      config->insert(string::clazz::flow, flow_table);
    }

    id_clazz.emplace(id, string::clazz::flow);
    curr_id = id;
    return this;
  }

  ConfigBuilder *branch(const std::string &id, const std::string &type,
                        const std::string &from) {
    auto id_table = cpptoml::make_table();
    id_table->insert(string::keyword::type, type);
    id_table->insert(string::keyword::from, from);

    if (config->contains(string::clazz::branch)) {
      config->get_table(string::clazz::branch)->insert(id, id_table);
    } else {
      auto branch_table = cpptoml::make_table();
      branch_table->insert(id, id_table);
      config->insert(string::clazz::branch, branch_table);
    }

    id_clazz.emplace(id, string::clazz::branch);
    curr_id = id;
    return this;
  }

  ConfigBuilder *confluence(const std::string &id, const std::string &type,
                            const std::string &from_left,
                            const std::string &from_right) {
    auto id_table = cpptoml::make_table();
    id_table->insert(string::keyword::type, type);
    // id_table->insert(string::keyword::from + "_left", from_left);
    // id_table->insert(string::keyword::from + "_right", from_right);

    if (config->contains(string::clazz::confluence)) {
      config->get_table(string::clazz::confluence)->insert(id, id_table);
    } else {
      auto confluence_table = cpptoml::make_table();
      confluence_table->insert(id, id_table);
      config->insert(string::clazz::confluence, confluence_table);
    }

    id_clazz.emplace(id, string::clazz::confluence);
    curr_id = id;
    return this;
  }

  ConfigBuilder *sink(const std::string &id, const std::string &type,
                      const std::string &from) {
    auto id_table = cpptoml::make_table();
    id_table->insert(string::keyword::type, type);
    id_table->insert(string::keyword::from, from);

    if (config->contains(string::clazz::sink)) {
      config->get_table(string::clazz::sink)->insert(id, id_table);
    } else {
      auto sink_table = cpptoml::make_table();
      sink_table->insert(id, id_table);
      config->insert(string::clazz::sink, sink_table);
    }

    id_clazz.emplace(id, string::clazz::sink);
    curr_id = id;
    return this;
  }

  template <class T>
  ConfigBuilder *set(const std::string &key, const T &value) {
    curr_table()->insert(key, value);
    return this;
  }

  template <class T>
  ConfigBuilder *set_array(const std::string &key,
                           const std::vector<T> &values) {
    auto array = cpptoml::make_array();
    for (const auto &value : values) {
      array->push_back(value);
    }
    curr_table()->insert(key, array);
    return this;
  }

  ConfigBuilder *set_array_bool(const std::string &key,
                                const std::vector<bool> &values) {
    auto array = cpptoml::make_array();
    for (const auto &value : values) {
      array->push_back((int)value);
    }
    curr_table()->insert(key, array);
    return this;
  }

  template <class T>
  ConfigBuilder *
  set_array_map(const std::string &key,
                const std::vector<std::unordered_map<std::string, T>> &values) {
    auto array = cpptoml::make_table_array();
    for (const auto &map : values) {
      auto table = cpptoml::make_table();
      for (const auto &kv : map) {
        table->insert(kv.first, kv.second);
      }
      array->push_back(table);
    }
    curr_table()->insert(key, array);
    return this;
  }

  template <class T>
  ConfigBuilder *set_map(const std::string &key,
                         const std::unordered_map<std::string, T> &values) {
    auto table = cpptoml::make_table();
    for (const auto &kv : values) {
      table->insert(kv.first, kv.second);
    }
    curr_table()->insert(key, table);
    return this;
  }

  template <class T>
  ConfigBuilder *set_(const std::vector<std::string> &keys, const T &value) {
    auto table = curr_table()->get_table(keys.at(0));
    for (size_t i = 1, end = keys.size(); i < end; ++i) {
      if (i + 1 != end) {
        table = table->get_table(keys.at(i));
      } else {
        table->insert(keys.at(i), value);
      }
    }
    return this;
  }

  template <class T>
  ConfigBuilder *set_array_(const std::vector<std::string> &keys,
                            const std::vector<T> &values) {
    auto array = cpptoml::make_array();
    for (const auto &value : values) {
      array->push_back(value);
    }

    auto table = curr_table()->get_table(keys.at(0));
    for (size_t i = 1, end = keys.size(); i < end; ++i) {
      if (i + 1 != end) {
        table = table->get_table(keys.at(i));
      } else {
        table->insert(keys.at(i), array);
      }
    }
    return this;
  }

  ConfigBuilder *set_array_bool_(const std::vector<std::string> &keys,
                                 const std::vector<bool> &values) {
    auto array = cpptoml::make_array();
    for (const auto &value : values) {
      array->push_back((int)value);
    }

    auto table = curr_table()->get_table(keys.at(0));
    for (size_t i = 1, end = keys.size(); i < end; ++i) {
      if (i + 1 != end) {
        table = table->get_table(keys.at(i));
      } else {
        table->insert(keys.at(i), array);
      }
    }
    return this;
  }

  template <class T>
  ConfigBuilder *set_array_map_(
      const std::vector<std::string> &keys,
      const std::vector<std::unordered_map<std::string, T>> &values) {
    auto array = cpptoml::make_table_array();
    for (const auto &map : values) {
      auto table = cpptoml::make_table();
      for (const auto &kv : map) {
        table->insert(kv.first, kv.second);
      }
      array->push_back(table);
    }

    auto table = curr_table()->get_table(keys.at(0));
    for (size_t i = 1, end = keys.size(); i < end; ++i) {
      if (i + 1 != end) {
        table = table->get_table(keys.at(i));
      } else {
        table->insert(keys.at(i), array);
      }
    }
    return this;
  }

  template <class T>
  ConfigBuilder *set_map_(const std::vector<std::string> &keys,
                          const std::unordered_map<std::string, T> &values) {
    auto _table = cpptoml::make_table();
    for (const auto &kv : values) {
      _table->insert(kv.first, kv.second);
    }

    auto table = curr_table()->get_table(keys.at(0));
    for (size_t i = 1, end = keys.size(); i < end; ++i) {
      if (i + 1 != end) {
        table = table->get_table(keys.at(i));
      } else {
        table->insert(keys.at(i), _table);
      }
    }
    return this;
  }
};
} // namespace amanogawa

#endif // AMANOGAWA_CONFIG_BUILDER_H
