#include "amanogawa/core/executor.h"

int main(int argc, char *argv[]) {
  const auto config_file = argc > 1 ? argv[1] : "../example/example.toml";
  amanogawa::core::execute(config_file);
}