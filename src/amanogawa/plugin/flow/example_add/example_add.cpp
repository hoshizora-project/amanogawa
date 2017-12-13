//#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#include <string>
#include <vector>
#include "amanogawa/include/flow.h"

namespace amanogawa {
namespace plugin {
namespace flow {
namespace example_add {
struct FlowExampleAddPlugin: FlowPlugin {
  std::vector<std::string> flow(std::vector<std::string> &data) const {
    printf("flow is called\n");
    for (auto &row: data) {
      row += "!";
    }
    return data;
  }
};

extern "C" get_flow_plugin_return_t get_flow_plugin() {
  return std::make_unique<FlowExampleAddPlugin>();
}
}
}
}
}
