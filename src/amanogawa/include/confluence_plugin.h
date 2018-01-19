#ifndef AMANOGAWA_CONFLUENCE_PLUGIN_H
#define AMANOGAWA_CONFLUENCE_PLUGIN_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <memory>
#include <string>

namespace amanogawa {
namespace plugin {
struct ConfluencePlugin : Plugin {
  std::string plugin_full_name() const override {
    return "confluence_" + plugin_name();
  }
  const std::string from_left;
  const std::string from_right;

  ConfluencePlugin(const std::string &id, const std::string &from_left,
                   const std::string &from_right, const config_t &config)
      : Plugin(id, config), from_left(from_left), from_right(from_right) {}

  virtual std::shared_ptr<arrow::Table>
  join(const std::shared_ptr<arrow::Table> &,
       const std::shared_ptr<arrow::Table> &) const = 0;
};

using confluence_plugin_t = std::shared_ptr<ConfluencePlugin>;
using get_confluence_plugin_t = confluence_plugin_t (*)(const std::string &,
                                                        const std::string &,
                                                        const std::string &,
                                                        const config_t &);
confluence_plugin_t as_confluence(const plugin_t &plugin) {
  return std::dynamic_pointer_cast<ConfluencePlugin>(plugin);
}
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_CONFLUENCE_PLUGIN_H
