#ifndef AMANOGAWA_COLUMN_H
#define AMANOGAWA_COLUMN_H

#include "amanogawa/core/column_info.h"
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace amanogawa {
namespace core {
// TODO: use arrow
template <class T> struct Column {
  const ColumnInfo col_info;
  const std::vector<T> data;

  Column(const ColumnInfo &col_info, const std::vector<T> &data)
      : col_info(col_info), data(data) {}
};

struct IntColumn : Column<int> {
  IntColumn(const ColumnInfo &col_info, const std::vector<int> &data)
      : Column(col_info, data) {}
};

struct DoubleColumn : Column<double> {
  DoubleColumn(const ColumnInfo &col_info, const std::vector<double> &data)
      : Column(col_info, data) {}
};

struct StringColumn : Column<std::string> {
  StringColumn(const ColumnInfo &col_info, const std::vector<std::string> &data)
      : Column(col_info, data) {}
};
} // namespace core
} // namespace amanogawa

#endif // AMANOGAWA_COLUMN_H
