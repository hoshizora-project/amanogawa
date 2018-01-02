#include "amanogawa/core/executor.h"

#include "amanogawa/core/column.h"

namespace amanogawa {
void main(int argc, char *argv[]) {
  const auto config_file = argc > 1 ? argv[1] : "../example/example.toml";
  amanogawa::core::execute(config_file);

  auto int_col_info = core::ColumnInfo("id", typeid(int), 0);
  std::vector<int> data = {1, 2, 3, 4, 4, 2, 3, 4};
  auto int_col = core::IntColumn(int_col_info, data);
}
} // namespace amanogawa

int main(int argc, char *argv[]) { amanogawa::main(argc, argv); }