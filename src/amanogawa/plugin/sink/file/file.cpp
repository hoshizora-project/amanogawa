#include "amanogawa/core/dl.h"
#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <text/csv/ostream.hpp>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SinkFilePlugin : SinkPlugin {
  std::string plugin_name() const override { return "file"; }

  std::shared_ptr<core::DL> format_lib;
  format_plugin_t format_plugin;

  explicit SinkFilePlugin(const std::string &id, const std::string &from,
                          const config_t &config)
      : SinkPlugin(id, from, config),
        format_lib(core::DL::open("format_" +
                                  *this->config->get_qualified_as<std::string>(
                                      string::keyword::format_type))),
        format_plugin(as_format(format_lib->sym<plugin::get_format_plugin_t>(
            string::func::get_plugin)(id, this->root_config))) {
    init_logger();
  }

  // TODO: Report exit status
  void *drain(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("drain");

    const auto file_name = *config->get_as<std::string>("path");
    return format_plugin->format(file_name, table);
  }
};

extern "C" sink_plugin_t
get_plugin(const std::string &id, const std::string &from,
           const config_t &config) {
  return std::make_shared<SinkFilePlugin>(id, from, config);
}
} // namespace file
} // namespace sink
} // namespace plugin
} // namespace amanogawa