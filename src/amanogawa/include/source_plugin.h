#ifndef AMANOGAWA_SOURCE_H
#define AMANOGAWA_SOURCE_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <memory>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SourcePlugin : Plugin {
  std::string plugin_full_name() const override {
    return "source_" + plugin_name();
  }

  // TODO: Make `format` a standalone plugin
  const Config::table_t format_config;

  virtual std::shared_ptr<arrow::Table> spring() const = 0;

  SourcePlugin(const std::string &id, const config_t &config)
      : Plugin(id, config), format_config(this->config->get_table("format")) {}
};

using source_plugin_t = std::shared_ptr<SourcePlugin>;
using get_source_plugin_t = source_plugin_t (*)(const std::string &,
                                                const config_t &);
source_plugin_t as_source(const plugin_t &plugin) {
  return std::dynamic_pointer_cast<SourcePlugin>(plugin);
}
} // namespace plugin
} // namespace amanogawa

#endif