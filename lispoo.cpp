#include <lispoo.h>
#include <core.h>
#include <sstream>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    lispoo::oops("error: no input files");
  }
  std::ifstream ifs(argv[1]);
  std::stringstream ss;
  if (!ifs.is_open()) {
    lispoo::oops("can't open: " + std::string(argv[1]));
  }
  ss << ifs.rdbuf();
  std::vector<std::string> tokens;
  lispoo::tokenize(ss.str(), tokens);
  unsigned long cursor = 0;
  auto expr = lispoo::parse(tokens, cursor);

  lispoo::putenv("+", [](auto args) {
    return lispoo::sum(args);
  });
  lispoo::putenv("car", [](auto args) {
    return args->value()[0];
  });
  lispoo::putenv("cdr", [](auto args) {
    auto value = args->value();
    auto cdr = std::make_shared<lispoo::List>();
    cdr->value().assign(value.begin() + 1, value.end());
    return cdr;
  });
  lispoo::putenv("message", [](auto args) {
    auto value = lispoo::get_value<lispoo::List>(args);
    for (auto& v : value) {
      lispoo::message(v);
      std::cout << std::endl;
    }
    return lispoo::nil;
  });
  lispoo::eval(expr, lispoo::global);
  return 0;
}
