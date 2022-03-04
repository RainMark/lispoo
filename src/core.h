#pragma once

#include <lispoo.h>

namespace lispoo {

#define BI_OP(fn, op)                                                 \
  inline std::shared_ptr<Expr> fn(const std::shared_ptr<Expr>& a,     \
                                  const std::shared_ptr<Expr>& b) {   \
    if (a->type() == Type::Float && b->type() == Type::Float) {       \
      return std::make_shared<Float>(get_value<Float>(a)              \
                                         op get_value<Float>(b));     \
    }                                                                 \
    if (a->type() == Type::Integer && b->type() == Type::Integer) {   \
      return std::make_shared<Integer>(get_value<Integer>(a)          \
                                           op get_value<Integer>(b)); \
    }                                                                 \
    if (a->type() == Type::Float) {                                   \
      return std::make_shared<Float>(get_value<Float>(a)              \
                                         op get_value<Integer>(b));   \
    }                                                                 \
    if (b->type() == Type::Float) {                                   \
      return std::make_shared<Float>(get_value<Integer>(a)            \
                                         op get_value<Float>(b));     \
    }                                                                 \
    oops(std::string(#op) +                                           \
         " failed, type: " + std::to_string((int)a->type()) + " " +   \
         std::to_string((int)b->type()));                             \
    return nil;                                                       \
  }

BI_OP(sum, +)
BI_OP(sub, -)
BI_OP(mul, *)
BI_OP(div, /)

BI_OP(eq, ==)
BI_OP(gt, >)
BI_OP(lt, <)
BI_OP(gte, >=)
BI_OP(lte, <=)

BI_OP(_and, &&)
BI_OP(_or, ||)

#undef BI_OP

inline std::shared_ptr<Expr> message(const std::shared_ptr<Expr>& expr) {
  auto type = expr->type();
  switch (type) {
    case Type::Null: {
      std::cout << "nil";
      break;
    }
    case Type::Integer: {
      std::cout << get_value<Integer>(expr);
      break;
    }
    case Type::Float: {
      std::cout << get_value<Float>(expr);
      break;
    }
    case Type::Symbol: {
      std::cout << get_value<Symbol>(expr);
      break;
    }
    case Type::Callable: {
      std::cout << "<fn>: " << expr.get();
      break;
    }
    case Type::List: {
      auto value = get_value<List>(expr);
      std::cout << "(";
      for (auto i = 0; i < value.size(); ++i) {
        if (i > 0) {
          std::cout << " ";
        }
        message(value[i]);
      }
      std::cout << ")";
    }
    default:
      break;
  }
  return nil;
}

inline void init() {
  // syntax
  putenv("quote", [](auto expr, auto env) {
    assert_len(expr, 2);
    auto value = get_value<List>(expr);
    return value[1];
  });
  putenv("define", [](auto expr, auto env) {
    assert_len(expr, 3);
    auto value = get_value<List>(expr);
    assert_type<Type::Symbol>(value[1]);
    auto symbol = get_value<Symbol>(value[1]);
    if (!is_nil(env->get(symbol))) {
      oops("symbol defined: " + symbol);
    }
    env->put(symbol, eval(value[2], env));
    return nil;
  });
  putenv("set!", [](auto expr, auto env) {
    assert_len(expr, 3);
    auto value = get_value<List>(expr);
    assert_type<Type::Symbol>(value[1]);
    auto symbol = get_value<Symbol>(value[1]);
    env->put(symbol, eval(value[2], env));
    return nil;
  });
  putenv("progn", [](auto expr, auto env) {
    auto val = nil;
    auto value = get_value<List>(expr);
    for (auto i = 1; i < value.size(); ++i) {
      val = eval(value[i], env);
    }
    return val;
  });
  putenv("if", [](auto expr, auto env) {
    // (if (cond) (then body) (else body))
    assert_len(expr, 4);
    auto value = get_value<List>(expr);
    if (is_true(eval(value[1], env))) {
      return eval(value[2], env);
    } else {
      return eval(value[3], env);
    }
  });
  putenv("while", [](auto expr, auto env) {
    // (while (cond) (loop body))
    assert_len(expr, 3);
    auto value = get_value<List>(expr);
    while (is_true(eval(value[1], env))) {
      eval(value[2], env);
    }
    return nil;
  });
  putenv("lambda", [](auto expr, auto env) {
    // (lambda (args) (body))
    assert_len(expr, 3);
    auto lambda = [expr](auto args, auto parent) {
      auto value = get_value<List>(expr);
      auto symbols = get_value<List>(value[1]);
      // arguments bind
      auto args_value = get_value<List>(args);
      if (symbols.size() != args_value.size() - 1) {
        oops("arguments error");
      }
      auto env = std::make_shared<Env>(parent);
      for (auto i = 0; i < symbols.size(); ++i) {
        auto symbol = get_value<Symbol>(symbols[i]);
        env->put(symbol, eval(args_value[i + 1], parent));
      }
      // eval body
      return eval(value[2], env);
    };
    return std::make_shared<Callable>(std::move(lambda));
  });

  // normal builtin function
#define PUT_BI_OP(fn, op)                                \
  putenv(#op, [](auto expr, auto env) {                  \
    assert_len(expr, 3);                                 \
    auto value = get_value<List>(expr);                  \
    return fn(eval(value[1], env), eval(value[2], env)); \
  })
  PUT_BI_OP(sum, +);
  PUT_BI_OP(sub, -);
  PUT_BI_OP(mul, *);
  PUT_BI_OP(div, /);

  PUT_BI_OP(eq, ==);
  PUT_BI_OP(gt, >);
  PUT_BI_OP(lt, <);
  PUT_BI_OP(gte, >=);
  PUT_BI_OP(lte, <=);

  PUT_BI_OP(_and, &&);
  PUT_BI_OP(_or, ||);

#undef PUT_BI_OP

  putenv("message", [](auto expr, auto env) {
    auto value = get_value<List>(expr);
    for (auto i = 1; i < value.size(); ++i) {
      message(eval(value[i], env));
      std::cout << std::endl;
    }
    return nil;
  });
}

}  // namespace lispoo
