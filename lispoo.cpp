#include <lispoo.h>
#include <core.h>
#include <sstream>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    lispoo::oops("lispoo ./xxx.lisp");
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

  lispoo::register_symbol("+", [](auto args) { return lispoo::sum(args); });
  lispoo::register_symbol("message", [](auto args) { return lispoo::message(args); });
  lispoo::eval(expr, lispoo::global);
  return 0;
}
