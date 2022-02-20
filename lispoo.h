#pragma once

#include <iostream>
#include <exception>

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace lispoo {

inline void oops(const std::string& err) {
  std::cerr << err << std::endl;
  exit(-1);
}

enum class Type {
  Null = 0,
  Float,
  Integer,
  Symbol,
  Atom,

  List,
  Callable,
};

class Expr {
public:
  virtual Type type() = 0;
};

template <typename T>
class Atom: public Expr {
public:
  explicit Atom(const T& value) : value_(value) {}
  Type type() override { return Type::Atom; }
  const T& value() const { return value_; }

private:
  T value_;
};

class Symbol: public Atom<std::string> {
public:
  explicit Symbol(const std::string& value) : Atom(value) {}
  Type type() override { return Type::Symbol; }
};

class Integer: public Atom<long> {
public:
  explicit Integer(const long& value) : Atom(value) {}
  Type type() override { return Type::Integer; }
};

class Float: public Atom<double> {
public:
  explicit Float(const double& value) : Atom(value) {}
  Type type() override { return Type::Float; }
};

class Null: public Expr {
public:
  Type type() override { return Type::Null; }
};
static const std::shared_ptr<Null> null_expr = std::make_shared<Null>();

class List: public Expr {
public:
  Type type() override { return Type::List; }
  const std::vector<std::shared_ptr<Expr>>& value() const { return value_; }
  void append(const std::shared_ptr<Expr>& expr) { value_.emplace_back(expr); }

private:
  std::vector<std::shared_ptr<Expr>> value_;
};

class Callable: public Expr {
public:
  using Fn = std::function<std::shared_ptr<Expr>(const std::shared_ptr<List>&)>;

  explicit Callable(Fn&& lambda) : lambda_(lambda) {}
  Type type() override { return Type::Callable; }
  const Fn& value() const {
    return lambda_;
  }

private:
  Fn lambda_;
};

class Env {
public:
  Env(const std::shared_ptr<Env>& env) : env_(env) {}

  std::shared_ptr<Expr> get(const std::shared_ptr<Symbol>& symbol) const {
    // std::cout << "get: " << symbol->value() << std::endl;
    auto it = expr_map_.find(symbol->value());
    if (it != expr_map_.end()) {
      return it->second;
    }
    if (env_) {
      return env_->get(symbol);
    }
    return null_expr;
  }
  void put(const std::shared_ptr<Symbol>& symbol, const std::shared_ptr<Expr>& expr) {
    // std::cout << "put: " << symbol->value() << std::endl;
    expr_map_[symbol->value()] = expr;
  }

private:
  std::unordered_map<std::string, std::shared_ptr<Expr>> expr_map_;
  std::shared_ptr<Env> env_;
};

static std::shared_ptr<Env> global = std::make_shared<Env>(std::shared_ptr<Env>());

inline void register_symbol(const std::string& symbol, Callable::Fn&& lambda) {
  global->put(std::make_shared<Symbol>(symbol), std::make_shared<Callable>(std::forward<Callable::Fn>(lambda)));
}

inline bool is_par(char ch) {
  return ch == '(' || ch == ')';
}
inline bool is_number(const Type& type) {
  return type == Type::Integer || type == Type::Float;
}
inline bool is_atom(const Type& type) {
  return type < Type::Atom;
}
inline bool is_true(const std::shared_ptr<Expr>& expr) {
  if (!is_number(expr->type())) {
    return false;
  }
  if (expr->type() == Type::Integer) {
    return std::static_pointer_cast<Integer>(expr)->value();
  }
  return std::static_pointer_cast<Float>(expr)->value() != 0.0;
}
inline void len_eq(const std::shared_ptr<Expr>& expr, unsigned long expect) {
  if (expr->type() != Type::List) {
    oops("len_eq() can't check non List type");
  }
  auto list = std::static_pointer_cast<List>(expr);
  if (list->value().size() != expect) {
    auto symbol = std::static_pointer_cast<Symbol>(list->value()[0]);
    oops("symbol: " + symbol->value() + " length not eq: " + std::to_string(expect));
  }
}

inline void tokenize(const std::string& str, std::vector<std::string>& tokens) {
  for (auto i = 0; i < str.size(); ++i) {
    if (std::isspace(str[i])) {
      continue;
    }
    if (is_par(str[i])) {
      tokens.emplace_back(str.c_str() + i, 1);
      continue;
    }
    auto s = i;
    for (; !(std::isspace(str[i]) || is_par(str[i])); ++i);
    tokens.emplace_back(str.c_str() + s, i - s);
    i--;
  }
}

inline std::shared_ptr<Expr> parse_atom(const std::vector<std::string>& tokens, unsigned long& cursor) {
  auto token = tokens[cursor];
  auto type = Type::Symbol;
  if (std::isdigit(token[0]) || token[0] == '-') {
    type = Type::Integer;
    for (auto i = 1; i < token.size(); ++i) {
      if (token[i] == '.') {
        if (type == Type::Float) {
          oops("parse failed, token: " + token);
        }
        type = Type::Float;
        continue;
      }
      if (!std::isdigit(token[i])) {
        oops("parse failed, token: " + token);
      }
    }
  }
  switch (type) {
  case Type::Symbol:
    return std::make_shared<Symbol>(token);
  case Type::Integer:
    return std::make_shared<Integer>(std::stol(token));
  case Type::Float:
    return std::make_shared<Float>(std::stod(token));
  }
  return std::shared_ptr<Expr>();
}

inline std::shared_ptr<Expr> parse(const std::vector<std::string>& tokens, unsigned long& cursor) {
  if (cursor >= tokens.size()) {
    oops("parse error");
  }
  // std::cout << "parse: " << tokens[cursor] << std::endl;
  if (tokens[cursor] == "(") {
    auto list = std::make_shared<List>();
    while (tokens[++cursor] != ")") {
      list->append(parse(tokens, cursor));
    }
    return list;
  }
  return parse_atom(tokens, cursor);
}

inline std::shared_ptr<Expr> eval(const std::shared_ptr<Expr>& expr, const std::shared_ptr<Env>& env) {
  if (!expr) {
    oops("syntax error");
    return null_expr;
  }
  auto type = expr->type();
  if (is_number(type)) {
    return expr;
  }
  if (type == Type::Symbol) {
    return env->get(std::static_pointer_cast<Symbol>(expr));
  }
  if (type != Type::List) {
    oops("syntax error");
  }
  auto& value = std::static_pointer_cast<List>(expr)->value();
  std::shared_ptr<Symbol> _ = std::static_pointer_cast<Symbol>(value[0]);
  auto& name = _->value();
  if (name == "quote") {
    len_eq(expr, 2);
    return value[1];
  }
  if (name == "def") {
    len_eq(expr, 3);
    auto symbol = std::static_pointer_cast<Symbol>(value[1]);
    if (env->get(symbol)->type() != Type::Null) {
      oops("symbol defined: " + symbol->value());
    }
    env->put(symbol, eval(value[2], env));
    return null_expr;
  }
  if (name == "set!") {
    len_eq(expr, 3);
    auto symbol = std::static_pointer_cast<Symbol>(value[1]);
    env->put(symbol, eval(value[2], env));
    return null_expr;
  }
  if (name == "prog") {
    for (auto i = 1; i < value.size(); ++i) {
      eval(value[i], env);
    }
    return null_expr;
  }
  if (name == "if") {
    // (if (cond) (then body) (else body))
    len_eq(expr, 4);
    if (is_true(eval(value[1], env))) {
      return eval(value[2], env);
    } else {
      return eval(value[3], env);
    }
  }
  if (name == "while") {
    // (while (cond) (loop body))
    len_eq(expr, 3);
    while (is_true(eval(value[1], env))) {
      eval(value[2], env);
    }
    return null_expr;
  }
  if (name == "lambda") {
    // (lambda (args) (body))
    len_eq(expr, 3);
    auto lambda = [expr, parent = env](const std::shared_ptr<List>& args) {
      auto& value = std::static_pointer_cast<List>(expr)->value();
      auto symbols = std::static_pointer_cast<List>(value[1]);
      // arguments bind
      len_eq(symbols, args->value().size());
      auto env = std::make_shared<Env>(parent);
      for (auto i = 0; i < symbols->value().size(); ++i) {
        auto symbol = std::static_pointer_cast<Symbol>(symbols->value()[i]);
        env->put(symbol, args->value()[i]);
      }
      // eval body
      return eval(value[2], env);
    };
    return std::make_shared<Callable>(std::move(lambda));
  }
  // function/lambda call
  // (symbol arg1 arg2 arg3 ... )
  auto callable = env->get(_);
  if (!callable) {
    oops("unknow symbol: " + name);
  }
  if (callable->type() != Type::Callable) {
    oops("can't call symbol: " + name);
  }
  auto args = std::make_shared<List>();
  for (auto i = 1; i < value.size(); ++i) {
    args->append(eval(value[i], env));
  }
  return std::static_pointer_cast<Callable>(callable)->value()(args);
}

} // namespace lispoo
