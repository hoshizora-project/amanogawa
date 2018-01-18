#ifndef AMANOGAWA_FLOW_H
#define AMANOGAWA_FLOW_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct FlowPlugin : Plugin {
  std::string plugin_full_name() const override {
    return "flow_" + plugin_name();
  }
  const std::string from; // need???

  virtual std::shared_ptr<arrow::Table>
  flow(const std::shared_ptr<arrow::Table> &) const = 0;

  FlowPlugin(const std::string &id, const std::string &from,
             const config_t &config)
      : Plugin(id, config), from(from) {}
};

using flow_plugin_t = std::shared_ptr<FlowPlugin>;
using get_flow_plugin_t = flow_plugin_t (*)(const std::string &,
                                            const std::string &,
                                            const config_t &);
} // namespace plugin
} // namespace amanogawa

#endif