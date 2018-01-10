#include "amanogawa/include/api.h"
#include "amanogawa/core/executor.h"

namespace amanogawa {
void main(int argc, char *argv[]) {
  const auto config_file = argc > 1 ? argv[1] : "../example/example.toml";
  const auto config = core::Config::load_from_file(config_file);
  core::execute(config);
}
} // namespace amanogawa

int main(int argc, char *argv[]) { amanogawa::main(argc, argv); }
