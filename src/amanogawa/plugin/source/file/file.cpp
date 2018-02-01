#include "amanogawa/core/dl.h"
#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <text/csv/istream.hpp>

namespace amanogawa {
namespace plugin {
namespace source {
namespace file {

struct SourceFilePlugin : SourcePlugin {
  std::string plugin_name() const override { return "file"; }

  const std::unique_ptr<core::DL>
      format_lib; // FIXME: Should be held at flow_graph
  const format_plugin_t format_plugin;

  explicit SourceFilePlugin(const std::string &id, const config_t &config)
      : SourcePlugin(id, config),
        format_lib(core::DL::open("format_" +
                                  *this->config->get_qualified_as<std::string>(
                                      string::keyword::format_type))),
        format_plugin(as_format(format_lib->sym<plugin::get_format_plugin_t>(
            string::func::get_plugin)(id, this->root_config))) {
    init_logger();
  }

  std::shared_ptr<arrow::Table> spring() const override {
    logger->info("spring");

    const auto file_name = *config->get_as<std::string>("path");
    return format_plugin->parse(file_name);
  }
}; // namespace file

extern "C" source_plugin_t
get_plugin(const std::string &id, const config_t &config) {
  return std::make_shared<SourceFilePlugin>(id, config);
}
} // namespace file
} // namespace source
} // namespace plugin
} // namespace amanogawa