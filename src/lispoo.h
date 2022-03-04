#pragma once

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace lispoo {

inline void oops(const std::string& err) {
  std::cerr << err << std::endl;
  std::exit(EXIT_FAILURE);
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
class Atom : public Expr {
 public:
  explicit Atom(const T& value) : value_(value) {}
  Type type() override { return Type::Atom; }
  const T& value() const { return value_; }

 private:
  T value_;
};

class Symbol : public Atom<std::string> {
 public:
  explicit Symbol(const std::string& value) : Atom(value) {}
  Type type() override { return Type::Symbol; }
};

class Integer : public Atom<long> {
 public:
  explicit Integer(const long& value) : Atom(value) {}
  Type type() override { return Type::Integer; }
};

class Float : public Atom<double> {
 public:
  explicit Float(const double& value) : Atom(value) {}
  Type type() override { return Type::Float; }
};

class Null : public Expr {
 public:
  Type type() override { return Type::Null; }
};
static const std::shared_ptr<Expr> nil = std::make_shared<Null>();

class List : public Expr {
 public:
  Type type() override { return Type::List; }
  const std::vector<std::shared_ptr<Expr>>& value() const { return value_; }
  void append(const std::shared_ptr<Expr>& expr) { value_.emplace_back(expr); }

 private:
  std::vector<std::shared_ptr<Expr>> value_;
};

class Env;
class Callable : public Expr {
 public:
  using Fn = std::function<std::shared_ptr<Expr>(
      const std::shared_ptr<Expr>&, const std::shared_ptr<Env>& env)>;

  explicit Callable(Fn&& lambda) : lambda_(lambda) {}
  Type type() override { return Type::Callable; }
  const Fn& value() const { return lambda_; }

 private:
  Fn lambda_;
};

// environment

class Env {
 public:
  Env(const std::shared_ptr<Env>& env) : env_(env) {}

  std::shared_ptr<Expr> get(const std::string& symbol) const {
    auto it = expr_map_.find(symbol);
    if (it != expr_map_.end()) {
      return it->second;
    }
    if (env_) {
      return env_->get(symbol);
    }
    return nil;
  }
  void put(const std::string& symbol, const std::shared_ptr<Expr>& expr) {
    expr_map_[symbol] = expr;
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<Expr>> expr_map_;
  std::shared_ptr<Env> env_;
};
static std::shared_ptr<Env> global =
    std::make_shared<Env>(std::shared_ptr<Env>());
inline void putenv(const std::string& symbol, Callable::Fn&& lambda) {
  global->put(symbol,
              std::make_shared<Callable>(std::forward<Callable::Fn>(lambda)));
}

// type utils

inline bool is_par(char ch) { return ch == '(' || ch == ')'; }
template <typename T>
inline decltype(auto) get_value(const std::shared_ptr<Expr>& expr) {
  return std::static_pointer_cast<T>(expr)->value();
}
template <Type t>
inline bool is_type(const std::shared_ptr<Expr>& expr) {
  return expr && expr->type() == t;
}
inline bool is_nil(const std::shared_ptr<Expr>& expr) {
  return is_type<Type::Null>(expr);
}
inline bool is_number(const std::shared_ptr<Expr>& expr) {
  return is_type<Type::Integer>(expr) || is_type<Type::Float>(expr);
}
inline bool is_symbol(const std::shared_ptr<Expr>& expr) {
  return is_type<Type::Symbol>(expr);
}
inline bool is_true(const std::shared_ptr<Expr>& expr) {
  if (!is_number(expr)) {
    oops("is_true() failed, not number type: " +
         std::to_string((int)expr->type()));
  }
  if (is_type<Type::Integer>(expr)) {
    return get_value<Integer>(expr);
  }
  return get_value<Float>(expr);
}

// asserts

template <Type t>
inline void assert_type(const std::shared_ptr<Expr>& expr) {
  if (!is_type<t>(expr)) {
    oops("syntax error");
  }
}
inline void assert_len(const std::shared_ptr<Expr>& expr, long expect) {
  if (!is_type<Type::List>(expr)) {
    oops("assert_len() failed, not List type");
  }
  auto value = get_value<List>(expr);
  if (value.size() != expect) {
    oops("assert_len() failed, expect: " + std::to_string(expect));
  }
}

// parse & evaluate

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
    for (; !(std::isspace(str[i]) || is_par(str[i])); ++i)
      ;
    tokens.emplace_back(str.c_str() + s, i - s);
    i--;
  }
}

inline std::shared_ptr<Expr> parse_atom(const std::vector<std::string>& tokens,
                                        long& cursor) {
  auto token = tokens[cursor];
  auto type = Type::Symbol;
  if (std::isdigit(token[0]) || (token[0] == '-' && token.size() > 1)) {
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
    default:
      break;
  }
  return nil;
}

inline std::shared_ptr<Expr> parse(const std::vector<std::string>& tokens,
                                   long& cursor) {
  if (cursor >= tokens.size()) {
    oops("parse error");
  }
  if (tokens[cursor] == "(") {
    auto list = std::make_shared<List>();
    while (tokens[++cursor] != ")") {
      list->append(parse(tokens, cursor));
    }
    return list;
  }
  return parse_atom(tokens, cursor);
}

inline std::shared_ptr<Expr> eval(const std::shared_ptr<Expr>& expr,
                                  const std::shared_ptr<Env>& env) {
  if (!expr) {
    oops("syntax error");
  }
  if (is_number(expr)) {
    return expr;
  }
  if (is_symbol(expr)) {
    return env->get(get_value<Symbol>(expr));
  }
  assert_type<Type::List>(expr);
  auto value = get_value<List>(expr);
  assert_type<Type::Symbol>(value[0]);
  auto name = get_value<Symbol>(value[0]);
  // function/lambda call
  // (symbol arg1 arg2 arg3 ... )
  auto callable = env->get(name);
  if (!callable) {
    oops("unknow symbol: " + name);
  }
  if (!is_type<Type::Callable>(callable)) {
    oops("can't call symbol: " + name);
  }
  return get_value<Callable>(callable)(expr, env);
}

}  // namespace lispoo
