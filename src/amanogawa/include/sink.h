#ifndef AMANOGAWA_SINK_H
#define AMANOGAWA_SINK_H

#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SinkPlugin {
  virtual void drain(const std::string &file_name,
                     const std::vector<std::string> &data) const = 0;
};

using get_sink_plugin_return_t = std::unique_ptr<SinkPlugin>;
using get_sink_plugin_t = get_sink_plugin_return_t (*)();
} // namespace plugin
} // namespace amanogawa

#endif