#ifndef AMANOGAWA_FLOW_H
#define AMANOGAWA_FLOW_H

#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include "amanogawa/include/plugin.h"
#include <arrow/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct FlowPlugin : Plugin {
  const core::Config::config_map flow_config;
  std::string plugin_full_name() const override {
    return "flow_" + plugin_name();
  }

  virtual std::shared_ptr<arrow::Table>
  flow(const std::shared_ptr<arrow::Table> &) const = 0;

  FlowPlugin(const core::Config &config)
      : Plugin(config), flow_config(config.flow) {}
};

using get_flow_plugin_return_t = std::unique_ptr<FlowPlugin>;
using get_flow_plugin_t = get_flow_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif