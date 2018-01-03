#include "amanogawa/core/confing.h"
#include "amanogawa/include/flow_plugin.h"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace example_add {
struct FlowExampleAddPlugin : FlowPlugin {
  std::string plugin_name() const {
    return "example_add";
  }
  const core::Config config;

  explicit FlowExampleAddPlugin(const core::Config &config) : config(config) {}

  std::vector<core::Row> flow(std::vector<core::Row> &data) const {
    printf("flow is called\n");
    // for (auto &row : data) {
    // row += "!";
    //}
    return data;
  }
};

extern "C" get_flow_plugin_return_t
get_flow_plugin(const core::Config &config) {
  return std::make_unique<FlowExampleAddPlugin>(config);
}
} // namespace example_add
} // namespace flow
} // namespace plugin
} // namespace amanogawa
