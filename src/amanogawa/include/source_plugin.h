#ifndef AMANOGAWA_SOURCE_H
#define AMANOGAWA_SOURCE_H

#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include "amanogawa/include/plugin.h"
#include <arrow/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SourcePlugin : Plugin {
  std::string plugin_full_name() const override {
    return "source_" + plugin_name();
  }

  virtual std::shared_ptr<arrow::Table> spring() const = 0;
};

using get_source_plugin_return_t = std::unique_ptr<SourcePlugin>;
using get_source_plugin_t =
    get_source_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif