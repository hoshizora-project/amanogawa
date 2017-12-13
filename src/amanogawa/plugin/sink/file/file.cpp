#include <string>
#include <vector>
#include <fstream>
#include "amanogawa/include/sink.h"

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SinkFilePlugin: SinkPlugin {
  void drain(const std::string &file_name, const std::vector<std::string> &data) const {
    printf("sink is called\n");
    std::ofstream output(file_name);
    for (const auto &row: data) {
      output << row;
    }
    output.close();
  }
};

extern "C" get_sink_plugin_return_t get_sink_plugin(){
  return std::make_unique<SinkFilePlugin>();
}
}
}
}
}