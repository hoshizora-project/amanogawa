#ifndef AMANOGAWA_ARROW_HELPER_H
#define AMANOGAWA_ARROW_HELPER_H

#include <arrow/api.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace amanogawa {
namespace core {

struct Visitor {
  virtual ~Visitor() = default;

  virtual void visit(const arrow::Int32Array &array) = 0;
  virtual void visit(const arrow::StringArray &array) = 0;
};

struct Nibbler {
  explicit Nibbler(const std::shared_ptr<arrow::Schema> &schema) {
    const auto fields = schema->fields();
    for (const auto &field : fields) {
      if (field->type()->id() == arrow::Int32Type::type_id) {
      }
    }
  }
};

void nibble(const arrow::Table &table, Nibbler f) {}

template <class Func>
void each_value(const std::shared_ptr<arrow::Table> &table,
                const std::string &name, Func f) {
  const auto field_idx = table->schema()->GetFieldIndex(name);
  const auto col = table->column(field_idx);
  const auto chunks = col->data()->chunks();

  if (col->type()->id() == arrow::Int32Type::type_id) {
    for (const auto &chunk : chunks) {
      const auto array = std::static_pointer_cast<arrow::Int32Array>(chunk);
      for (int64_t i = 0, end = array->length(); i < end; ++i) {
        f(array->Value(i));
      }
    }
  } else if (col->type()->id() == arrow::DoubleType::type_id) {
    for (const auto &chunk : chunks) {
      const auto array = std::static_pointer_cast<arrow::DoubleArray>(chunk);
      for (int64_t i = 0, end = array->length(); i < end; ++i) {
        f(array->Value(i));
      }
    }
  } else if (col->type()->id() == arrow::StringType::type_id) {
    for (const auto &chunk : chunks) {
      const auto array = std::static_pointer_cast<arrow::StringArray>(chunk);
      for (int64_t i = 0, end = array->length(); i < end; ++i) {
        f(array->GetString(i));
      }
    }
  } else {
    //
  }
}

struct Lazy {
  template <class F, class... A> auto operator()(F &&f, A &&... a) {
    return [&](auto &&... b) {
      return std::forward<F>(f)(std::forward<A>(a)...,
                                std::forward<decltype(b)>(b)...);
    };
  }
};

template <class Func>
void each_row(const std::shared_ptr<arrow::Table> &table,
              const std::vector<std::string> &names, Func f) {
  auto _ = Lazy{};
  auto func = f;
  for (const auto &name : names) {
    each_value(table, name, [&func, &_, f](auto val) { func = _(f, val); });
    f();
  }

  /*
  for (size_t i = 0, num_cols = names.size(); i < num_cols; ++i) {
    const auto name = name.at(i);
    const auto field_idx = table->schema()->GetFieldIndex(name);
    const auto col = table->column(field_idx);
    const auto chunks = col->data()->chunks();

    if (col->type()->id() == arrow::Int32Type::type_id) {
      //flow.emplace_back([]() {
        for (const auto &chunk : chunks) {
          const auto array = std::static_pointer_cast<arrow::Int32Array>(chunk);
          for (int64_t j = 0, end = array->length(); j < end; ++j) {
            std::bind(f, array->Value(j));
            f(array->Value(j));
          }
        }
      //});
    } else if (col->type()->id() == arrow::DoubleType::type_id) {
      //flow.emplace_back([]() {
        for (const auto &chunk : chunks) {
          const auto array =
              std::static_pointer_cast<arrow::DoubleArray>(chunk);
          for (int64_t j = 0, end = array->length(); j < end; ++j) {
            f(array->Value(j));
          }
        }
      //});
    } else if (col->type()->id() == arrow::StringType::type_id) {
      //flow.emplace_back([]() {
        for (const auto &chunk : chunks) {
          const auto array =
              std::static_pointer_cast<arrow::StringArray>(chunk);
          for (int64_t j = 0, end = array->length(); j < end; ++j) {
            f(array->GetString(j));
          }
        }
      //});
    } else {
      throw std::runtime_error("Invalid type");
    }
  }*/
}
} // namespace core
} // namespace amanogawa

#endif // AMANOGAWA_ARROW_HELPER_H
