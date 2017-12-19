#ifndef AMANOGAWA_COLUMN_INFO_H
#define AMANOGAWA_COLUMN_INFO_H

#include "amanogawa/core/row.h"
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace amanogawa {
namespace core {

struct ColumnInfo {
  std::string name;
  std::type_index type;
  size_t index;

  ColumnInfo(const std::string &name, const std::type_index &type,
             const size_t &index)
      : name(name), type(type), index(index) {}
};

using ColumnsInfo = std::vector<ColumnInfo>;

std::unordered_map<std::string, std::type_index> type_map = {
    {"int", typeid(int)},
    {"double", typeid(double)},
    {"string", typeid(std::string)}};

template <class Func>
void visit(Row row, ColumnsInfo cols_info, size_t idx, Func f) {
  const auto type = cols_info.at(idx).type;
  if (type == typeid(int)) {
    f(linb::any_cast<int>(row.at(idx)));
  } else if (type == typeid(double)) {
    f(linb::any_cast<double>(row.at(idx)));
  } else if (type == typeid(std::string)) {
    f(linb::any_cast<std::string>(row.at(idx)));
  } else {
  }
}

} // namespace core
} // namespace amanogawa

#endif