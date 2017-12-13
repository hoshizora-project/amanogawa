#include "amanogawa/core/confing.h"
#include "amanogawa/include/sink.h"
#include <fstream>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SinkFilePlugin : SinkPlugin {
  const core::Config config;

  explicit SinkFilePlugin(const core::Config &config) : config(config) {}

  void drain(const std::string &file_name,
             const std::vector<std::string> &data) const {
    printf("sink is called\n");
    std::ofstream output(file_name);
    for (const auto &row : data) {
      output << row;
    }
    output.close();
  }
};

extern "C" get_sink_plugin_return_t
get_sink_plugin(const core::Config &config) {
  return std::make_unique<SinkFilePlugin>(config);
}
} // namespace file
} // namespace sink
} // namespace plugin
} // namespace amanogawa