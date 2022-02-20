#pragma once

#include <lispoo.h>

namespace lispoo {

inline std::shared_ptr<Expr> sum(const std::shared_ptr<List>& args) {
  len_eq(args, 2);
  auto a = args->value()[0], b = args->value()[1];
  if (a->type() == Type::Float
      && b->type() == Type::Float) {
    return std::make_shared<Float>(std::static_pointer_cast<Float>(a)->value() +
                                   std::static_pointer_cast<Float>(b)->value());
  }
  if (a->type() == Type::Integer
      && b->type() == Type::Integer) {
    return std::make_shared<Integer>(std::static_pointer_cast<Integer>(a)->value() +
                                     std::static_pointer_cast<Integer>(b)->value());
  }
  if (a->type() == Type::Float) {
    return std::make_shared<Float>(std::static_pointer_cast<Float>(a)->value() +
                                   std::static_pointer_cast<Integer>(b)->value());
  }
  if (b->type() == Type::Float) {
    return std::make_shared<Float>(std::static_pointer_cast<Integer>(a)->value() +
                                   std::static_pointer_cast<Float>(b)->value());
  }
}

inline std::shared_ptr<Expr> message(const std::shared_ptr<Expr>& expr) {
  if (!expr) {
    return null_expr;
  }
  auto type = expr->type();
  switch (type) {
    case Type::Null: {
      std::cout << "nil";
      break;
    }
    case Type::Integer: {
      std::cout << std::static_pointer_cast<Integer>(expr)->value();
      break;
    }
    case Type::Float: {
      std::cout << std::static_pointer_cast<Float>(expr)->value();
      break;
    }
    case Type::Symbol: {
      std::cout << std::static_pointer_cast<Symbol>(expr)->value();
      break;
    }
    case Type::Callable: {
      std::cout << "<function>: " << expr.get();
      break;
    }
    case Type::List: {
      auto& value = std::static_pointer_cast<List>(expr)->value();
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
  return null_expr;
}

} // namespace lispoo
