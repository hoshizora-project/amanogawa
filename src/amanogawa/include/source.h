#ifndef AMANOGAWA_SOURCE_H
#define AMANOGAWA_SOURCE_H

#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SourcePlugin {
  virtual std::vector<core::Row> spring(const std::string &file_name) const = 0;
};

using get_source_plugin_return_t = std::unique_ptr<SourcePlugin>;
using get_source_plugin_t =
    get_source_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif