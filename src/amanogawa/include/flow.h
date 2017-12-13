#ifndef AMANOGAWA_FLOW_H
#define AMANOGAWA_FLOW_H

#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct FlowPlugin {
  virtual std::vector<std::string>
  flow(std::vector<std::string> &data) const = 0;
};

using get_flow_plugin_return_t = std::unique_ptr<FlowPlugin>;
using get_flow_plugin_t = get_flow_plugin_return_t (*)();
} // namespace plugin
} // namespace amanogawa

#endif