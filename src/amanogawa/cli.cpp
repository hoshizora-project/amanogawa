#include "amanogawa/core/executor.h"
#include "amanogawa/include/api.h"

namespace amanogawa {
void main(int argc, char *argv[]) {
  const auto config_file = argc > 1 ? argv[1] : "../example/example.toml";
  const auto config = Config::from_file(config_file);
  core::execute(config);
}
} // namespace amanogawa

int main(int argc, char *argv[]) { amanogawa::main(argc, argv); }
