#include <string>
#include <vector>
#include <dlfcn.h>
#include <pybind11/pybind11.h>
#include <arrow/api.h>
#include <iostream>
//#include "nagare/core/plugin/extract.h"
//#include "nagare/core/plugin/transform.h"
//#include "nagare/core/plugin/load.h"

//int main() {
int hoge(const std::string &dll_name) {
  //void *extractor;
  //void *transformer;
  //void *loader;
  //std::vector<std::string> (*extract)(std::string);
  //std::vector<std::string> (*transform)(std::vector<std::string>);
  //void (*load)(std::vector<std::string>);
  std::shared_ptr<arrow::Array> array;
  {
    // 真偽値の配列データを確保する領域
    auto memory_pool = arrow::default_memory_pool();
    // 真偽値の配列を作成するビルダー
    arrow::BooleanBuilder builder(memory_pool);
    // 1つ目の要素はtrue
    builder.Append(true);
    // 2つ目の要素はfalse
    builder.Append(false);
    // 3つ目の要素はtrue
    builder.Append(true);
    // 3要素の配列を作成
    builder.Finish(&array);
  }

  // 内容を確認
  // 出力：
  // 0:true
  // 1:false
  // 2:true
  for (int64_t i = 0; i < array->length(); ++i) {
    auto boolean_array = static_cast<arrow::BooleanArray *>(array.get());
    std::cout << i << ":";
    // i番目の値を出力
    std::cout << (boolean_array->Value(i) ? "true" : "false");
    std::cout << std::endl;
  }

  printf("a\n");

  //auto mode = RTLD_LOCAL; // mac
  auto mode = RTLD_LAZY; // linux
  void *extractor = dlopen("libfrom_csv.so", mode);
  void *transformer = dlopen("libadd.so", mode);
  void *loader = dlopen("libto_plain.so", mode);

  printf("a\n");
  if(extractor == nullptr) {
    printf("%s\n", dlerror());
  }
  if(transformer == nullptr) {
    printf("%s\n", dlerror());
  }
  if(loader == nullptr) {
    printf("%s\n", dlerror());
  }

  auto extract = (std::vector<std::string> (*)(const std::string &)) dlsym(extractor, "extract");
  if(extract == nullptr) {
    printf("%s\n", dlerror());
  }
  printf("a\n");
  auto transform = (std::vector<std::string> (*)(std::vector<std::string>))dlsym(transformer, "transform");
  if(transform == nullptr) {
    printf("%s\n", dlerror());
  }
  printf("a\n");
  auto load = (void (*)(const std::string &, const std::vector<std::string> &))dlsym(loader, "load");
  if(load == nullptr) {
    printf("%s\n", dlerror());
  }
  printf("a\n");

  auto data = extract("raw.csv");
  printf("a\n");
  auto transformed = transform(data);
  printf("a\n");
  load("result", transformed);
  printf("a\n");

  dlclose(extractor);
  dlclose(transformer);
  dlclose(loader);

  return 0;
}

PYBIND11_MODULE(nagare, m) {
  m.doc() = "pybind11 exampleuuuuu";
  m.def("hoge", &hoge, "This is a hoge");
}
