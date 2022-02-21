#pragma once

#include <lispoo.h>

namespace lispoo {

inline std::shared_ptr<Expr> sum(const std::shared_ptr<List>& args) {
  assert_len(args, 2);
  auto a = args->value()[0], b = args->value()[1];
  if (a->type() == Type::Float
      && b->type() == Type::Float) {
    return std::make_shared<Float>(get_value<Float>(a) + get_value<Float>(b));
  }
  if (a->type() == Type::Integer
      && b->type() == Type::Integer) {
    return std::make_shared<Integer>(get_value<Integer>(a) + get_value<Integer>(b));
  }
  if (a->type() == Type::Float) {
    return std::make_shared<Float>(get_value<Float>(a) + get_value<Integer>(b));
  }
  if (b->type() == Type::Float) {
    return std::make_shared<Float>(get_value<Integer>(a) + get_value<Float>(b));
  }
  return nil;
}

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

} // namespace lispoo
