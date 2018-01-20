#ifndef AMANOGAWA_UTIL_H
#define AMANOGAWA_UTIL_H

#include <arrow/api.h>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace amanogawa {
using logger_t = std::shared_ptr<spdlog::logger>;
// TODO: Thread-safe
static logger_t get_logger(const std::string &id) {
  const auto fqid = "amanogawa@" + id;
  auto logger = spdlog::get(fqid);
  return logger != nullptr ? logger : spdlog::stderr_color_mt(fqid);
}

static void drop_logger(const std::string &id) {
  spdlog::drop("amanogawa@" + id);
}

// arrow
static std::unordered_map<std::string, std::string> normalize_table = {
    {"int32", "int32"},    {"int", "int32"}, {"float64", "float64"},
    {"double", "float64"}, {"utf8", "utf8"}, {"string", "utf8"}};

static std::unordered_map<std::string,
                          std::function<std::shared_ptr<arrow::DataType>(void)>>
    arrow_data_type_table = {{"int32", []() { return arrow::int32(); }},
                             {"float64", []() { return arrow::float64(); }},
                             {"utf8", []() { return arrow::utf8(); }}};

auto get_arrow_data_type(const std::string &type) {
  return arrow_data_type_table.at(normalize_table.at(type))();
}

static std::unordered_map<
    std::string,
    std::function<std::shared_ptr<arrow::ArrayBuilder>(arrow::MemoryPool *)>>
    arrow_builder_table = {
        {"int32",
         [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::Int32Builder>(pool);
         }},
        {"float64",
         [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::DoubleBuilder>(pool);
         }},
        {"utf8", [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::StringBuilder>(pool);
         }}};

auto get_arrow_builder(const std::string &type,
                       arrow::MemoryPool *pool = arrow::default_memory_pool()) {
  return arrow_builder_table.at(normalize_table.at(type))(pool);
}
} // namespace amanogawa

#endif // AMANOGAWA_UTIL_H
