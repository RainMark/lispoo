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

  lispoo::init();
  unsigned long cursor = 0;
  std::vector<std::string> tokens;
  lispoo::tokenize(ss.str(), tokens);
  auto expr = lispoo::parse(tokens, cursor);
  lispoo::eval(expr, lispoo::global);
  return 0;
}
