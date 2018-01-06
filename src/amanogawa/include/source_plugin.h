#ifndef AMANOGAWA_SOURCE_H
#define AMANOGAWA_SOURCE_H

#include "amanogawa/core/confing.h"
#include "amanogawa/include/plugin.h"
#include <arrow/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SourcePlugin : Plugin {
  const core::Config::config_map source_config;
  std::string plugin_full_name() const override {
    return "source_" + plugin_name();
  }

  virtual std::shared_ptr<arrow::Table> spring() const = 0;

  SourcePlugin(const core::Config &config)
      : Plugin(config), source_config(config.source) {}
};

using get_source_plugin_return_t = std::unique_ptr<SourcePlugin>;
using get_source_plugin_t =
    get_source_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif