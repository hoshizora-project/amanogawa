#include "amanogawa/core/confing.h"
#include "amanogawa/core/executor.h"
#ifdef PYTHON
#include <pybind11/pybind11.h>
#endif

namespace amanogawa {
void main(int argc, char *argv[]) {
  const auto config_file = argc > 1 ? argv[1] : "../example/example.toml";
  const auto config = core::Config::load_from_file(config_file);
  core::execute(config);
}
} // namespace amanogawa

int main(int argc, char *argv[]) { amanogawa::main(argc, argv); }

namespace amanogawa {
#ifdef PYTHON
PYBIND11_MODULE(amanogawa, m) {
  m.doc() = "amanogawa";
  pybind11::class_<core::Config> config(m, "Config");
  config.def("load_from_file", &core::Config::load_from_file,
             "load config from file");
  m.def("execute", &core::execute, "execute");
}
#endif
} // namespace amanogawa