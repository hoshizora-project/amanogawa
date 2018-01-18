#ifndef AMANOGAWA_BRANCH_PLUGIN_H
#define AMANOGAWA_BRANCH_PLUGIN_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <string>
#include <unordered_map>

namespace amanogawa {
namespace plugin {
struct BranchPlugin : Plugin {
  std::string plugin_full_name() const override {
    return "branch_" + plugin_name();
  }
  const std::string from; // need???

  virtual std::shared_ptr<
      std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>
  branch(const std::shared_ptr<arrow::Table> &) const = 0;

  BranchPlugin(const std::string &id, const std::string &from,
               const config_t &config)
      : Plugin(id, config), from(from) {}
};

using branch_plugin_t = std::shared_ptr<BranchPlugin>;
using get_branch_plugin_t = branch_plugin_t (*)(const std::string &,
                                                const std::string &,
                                                const config_t &);
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_BRANCH_PLUGIN_H
