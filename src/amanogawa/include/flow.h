#ifndef AMANOGAWA_FLOW_H
#define AMANOGAWA_FLOW_H

#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct FlowPlugin {
  virtual std::vector<core::Row> flow(std::vector<core::Row> &data) const = 0;
};

using get_flow_plugin_return_t = std::unique_ptr<FlowPlugin>;
using get_flow_plugin_t = get_flow_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif