#ifndef AMANOGAWA_FORMAT_PLUGIN_H
#define AMANOGAWA_FORMAT_PLUGIN_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <memory>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct FormatPlugin : Plugin {
  std::string plugin_full_name() const override {
    return "format_" + plugin_name();
  }

  // TMP: Passing path, should pass stream or sth to support s3 and remote files
  virtual std::shared_ptr<arrow::Table>
  parse(const std::string &path) const = 0;
  virtual void *format(const std::string &path,
                       const std::shared_ptr<arrow::Table> &) const = 0;

  FormatPlugin(const std::string &id, const config_t &config)
      : Plugin(id + ".format", config) {}
};

using format_plugin_t = std::shared_ptr<FormatPlugin>;
using get_format_plugin_t = format_plugin_t (*)(const std::string &,
                                                const config_t &);
format_plugin_t as_format(const plugin_t &plugin) {
  return std::dynamic_pointer_cast<FormatPlugin>(plugin);
}
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_FORMAT_PLUGIN_H
