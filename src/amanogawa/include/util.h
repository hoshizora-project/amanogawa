#ifndef AMANOGAWA_UTIL_H
#define AMANOGAWA_UTIL_H

#include <arrow/api.h>
#include <chrono>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef DEBUG_ON
//#include "pcm/cpucounters.h"
#endif

namespace amanogawa {
// logger
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
    {"int32", "int32"},    {"int", "int32"},     {"float64", "float64"},
    {"double", "float64"}, {"utf8", "utf8"},     {"string", "utf8"},
    {"date32", "date32"},  {"date64", "date64"}, {"date", "date64"}};

static std::unordered_map<std::string,
                          std::function<std::shared_ptr<arrow::DataType>(void)>>
    arrow_data_type_table = {{"int32", []() { return arrow::int32(); }},
                             {"float64", []() { return arrow::float64(); }},
                             {"utf8", []() { return arrow::utf8(); }},
                             {"date32", []() { return arrow::date32(); }},
                             {"date64", []() { return arrow::date64(); }}};

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
        {"utf8",
         [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::StringBuilder>(pool);
         }},
        {"date32",
         [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::Date32Builder>(pool);
         }},
        {"date64", [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::Date64Builder>(pool);
         }}};

auto get_arrow_builder(const std::string &type,
                       arrow::MemoryPool *pool = arrow::default_memory_pool()) {
  return arrow_builder_table.at(normalize_table.at(type))(pool);
}

// others
inline std::vector<std::string> split(const std::string &str, const char &sep) {
  std::vector<std::string> res;
  auto first = str.begin();
  while (first != str.end()) {
    auto last = first;
    while (last != str.end() && *last != sep) {
      ++last;
    }
    res.emplace_back(std::string(first, last));
    if (last != str.end()) {
      ++last;
    }
    first = last;
  }
  return res;
}

struct partial_score {
  const std::chrono::high_resolution_clock::time_point time;

#ifdef DEBUG_ON_ //
  const SystemCounterState counter;

  explicit partial_score(std::chrono::high_resolution_clock::time_point time,
                         SystemCounterState counter)
      : time(time), counter(counter) {}
#else
  explicit partial_score(std::chrono::high_resolution_clock::time_point time)
      : time(time) {}
#endif
};

std::unordered_map<std::string, std::unique_ptr<partial_score>> scores;

void point(const std::string &key) {
  get_logger("debug")->info(key);
  const auto time = std::chrono::high_resolution_clock::now();
#ifdef DEBUG_ON_ //
  const auto counter = getSystemCounterState();

  scores[key] = std::make_unique<partial_score>(time, counter);
#else
  scores[key] = std::make_unique<partial_score>(time);
#endif
}

void report(const std::string &start_key, const std::string &end_key) {
  const auto logger = get_logger("debug");
#ifdef DEBUG_ON_ //
  const auto start = scores[start_key]->counter;
  const auto end = scores[end_key]->counter;
  logger->info("\n"
               "[{} -> {}]\n"
               "ElapsedTime[sec]:\t{}\n"
               "Read[GB]:\t{}\n"
               "Write[GB]:\t{}\n"
               "L2CacheMisses:\t{}\n"
               "L3CacheMisses:\t{}\n"
               "L2CacheHitRatio:\t{}\n"
               "L3CacheHitRatio:\t{}\n"
               "InstructionsRetired:\t{}\n"
               "IPC:\t{}",
               start_key, end_key,
               std::chrono::duration_cast<std::chrono::milliseconds>(
                   scores[end_key]->time - scores[start_key]->time)
                       .count() /
                   1000.0,
               getBytesReadFromMC(start, end) / 1024.0 / 1024.0 / 1024.0,
               getBytesWrittenToMC(start, end) / 1024.0 / 1024.0 / 1024.0,
               getL2CacheMisses(start, end), getL3CacheMisses(start, end),
               getL2CacheHitRatio(start, end), getL3CacheHitRatio(start, end),
               getInstructionsRetired(start, end), getIPC(start, end));
#else
  logger->info("\n"
               "[{} -> {}]\n"
               "ElapsedTime[sec]:\t{}",
               start_key, end_key,
               std::chrono::duration_cast<std::chrono::milliseconds>(
                   scores[end_key]->time - scores[start_key]->time)
                       .count() /
                   1000.0);
#endif
}
} // namespace amanogawa

#endif // AMANOGAWA_UTIL_H
