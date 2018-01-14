#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <arrow/python/api.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace numpy {
struct SinkNumpyPlugin : SinkPlugin {
  std::string plugin_name() const override { return "numpy"; }
  const logger_t logger = get_logger(SinkPlugin::plugin_full_name());
  const Config::config_map plugin_config;

  explicit SinkNumpyPlugin(const Config &config)
      : SinkPlugin(config),
        plugin_config(sink_config->get_table(plugin_name())) {}

  void *drain(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("drain");

    std::vector<int64_t> indices;
    const auto cols = plugin_config->get_array_of<std::string>("columns");
    for (const auto &col : *cols) {
      indices.emplace_back(table->schema()->GetFieldIndex(col));
    }

    auto x = table->column(0);
    auto y = x->data();
    auto z = y->chunk(0);
    auto u = z->data();
    auto w = u->buffers;
    auto h = w[0];

    arrow::Tensor tensor(arrow::float64(), h, {1, table->num_rows()});

    Py_Initialize();

    auto res = new PyObject();
    arrow::py::TensorToNdarray(tensor, res, &res);
    return res;
  }
};

__attribute__((visibility("default"))) extern "C" get_sink_plugin_return_t
get_sink_plugin(const Config &config) {
  return std::make_unique<SinkNumpyPlugin>(config);
}
} // namespace numpy
} // namespace sink
} // namespace plugin
} // namespace amanogawa