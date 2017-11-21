#include <string>
#include <vector>
#include <dlfcn.h>
#include <pybind11/pybind11.h>
//#include "nagare/core/plugin/extract.h"
//#include "nagare/core/plugin/transform.h"
//#include "nagare/core/plugin/load.h"

//int main() {
int hoge() {
  //void *extractor;
  //void *transformer;
  //void *loader;
  //std::vector<std::string> (*extract)(std::string);
  //std::vector<std::string> (*transform)(std::vector<std::string>);
  //void (*load)(std::vector<std::string>);

  printf("a\n");

  void *extractor = dlopen("libfrom_csv.dylib", RTLD_LOCAL);
  void *transformer = dlopen("libadd.dylib", RTLD_LOCAL);
  void *loader = dlopen("libto_plain.dylib", RTLD_LOCAL);

  printf("a\n");
  if(extractor == nullptr) {
    printf("null\n");
  }
  if(transformer == nullptr) {
    printf("null\n");
  }
  if(loader == nullptr) {
    printf("null\n");
  }

  auto extract = (std::vector<std::string> (*)(const std::string &)) dlsym(extractor, "extract");
  if(extract == nullptr) {
    printf("null0%s\n", dlerror());
  }
  printf("a\n");
  auto transform = (std::vector<std::string> (*)(std::vector<std::string>))dlsym(transformer, "transform");
  if(transform == nullptr) {
    printf("null1%s\n", dlerror());
  }
  printf("a\n");
  auto load = (void (*)(const std::string &, const std::vector<std::string> &))dlsym(loader, "load");
  if(load == nullptr) {
    printf("null2\n");
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
