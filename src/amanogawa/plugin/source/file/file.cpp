#include "amanogawa/core/confing.h"
#include "amanogawa/include/source.h"
#include "amanogawa/plugin/source/file/csv.h"
#include <amanogawa/core/confing.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace source {
namespace file {
using amanogawa::plugin::SourcePlugin;

struct SourceFilePlugin : SourcePlugin {
  const core::Config config;

  explicit SourceFilePlugin(const core::Config &config) : config(config) {}

  std::vector<std::string> spring(const std::string &file_name) const {
    printf("source is called\n");
    io::CSVReader<2> in(file_name);
    in.read_header(io::ignore_extra_column, "id", "name");
    uint32_t id;
    std::string name;
    std::vector<std::string> result;
    while (in.read_row(id, name)) {
      result.emplace_back(std::to_string(id) + "|" + name);
    }

    return result;
  }
};

extern "C" get_source_plugin_return_t
get_source_plugin(const core::Config &config) {
  return std::make_unique<SourceFilePlugin>(config);
}
} // namespace file
} // namespace source
} // namespace plugin
} // namespace amanogawa