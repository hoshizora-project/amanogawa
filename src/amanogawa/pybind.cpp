#include "amanogawa/core/executor.h"
#include "amanogawa/include/api.h"
#include <pybind11/pybind11.h>

namespace amanogawa {
PYBIND11_MODULE(amanogawa, m) {
  m.doc() = "amanogawa";
  pybind11::class_<Config> config(m, "Config");
  config.def("load_from_file", &Config::load_from_file,
             "load config from file");
  m.def("execute", &core::execute, "execute",
        pybind11::return_value_policy::reference);
}
} // namespace amanogawa