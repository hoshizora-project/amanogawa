#include "amanogawa/core/confing.h"
#include "amanogawa/include/flow.h"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace example_add {
struct FlowExampleAddPlugin : FlowPlugin {
  const core::Config config;

  explicit FlowExampleAddPlugin(const core::Config &config) : config(config) {}

  std::vector<std::string> flow(std::vector<std::string> &data) const {
    printf("flow is called\n");
    for (auto &row : data) {
      row += "!";
    }
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
