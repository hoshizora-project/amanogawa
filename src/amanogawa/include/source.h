#ifndef AMANOGAWA_SOURCE_H
#define AMANOGAWA_SOURCE_H

#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SourcePlugin {
  virtual std::vector<std::string>
  spring(const std::string &file_name) const = 0;
};

using get_source_plugin_return_t = std::unique_ptr<SourcePlugin>;
using get_source_plugin_t = get_source_plugin_return_t (*)();
} // namespace plugin
} // namespace amanogawa

#endif