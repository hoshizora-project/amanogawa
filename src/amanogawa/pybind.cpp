#include "amanogawa/include/api.h"
#include "amanogawa/core/executor.h"
#include <pybind11/pybind11.h>

namespace amanogawa {
PYBIND11_MODULE(amanogawa, m) {
m.doc() = "amanogawa";
pybind11::class_<core::Config> config(m, "Config");
config.def("load_from_file", &core::Config::load_from_file,
"load config from file");
m.def("execute", &core::execute, "execute");
}
} // namespace amanogawa