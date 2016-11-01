#include "expr_executor.h"

#include <string>
#include <boost/variant.hpp>

namespace setti {
namespace internal {

std::unique_ptr<Object> ExpressionExecutor::Exec(AstNode* node) {
  switch (node->type()) {
    case AstNode::NodeType::kLiteral: {
      return ExecLiteral(node);
    } break;
  }
}

std::unique_ptr<Object> ExpressionExecutor::ExecLiteral(AstNode* node) {
  Literal* literal = static_cast<Literal*>(node);
  switch (literal->literal_type()) {
    case Literal::Type::kInteger: {
      std::unique_ptr<Object> obj(
          new IntObject(boost::get<int>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kBool: {
      std::unique_ptr<Object> obj(
          new BoolObject(boost::get<bool>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kReal: {
      std::unique_ptr<Object> obj(
          new RealObject(boost::get<float>(literal->value())));
      return obj;
    } break;

    case Literal::Type::kString: {
    std::string str = boost::get<std::string>(literal->value());
      std::unique_ptr<Object> obj(
          new StringObject(std::move(str)));
      return obj;
    } break;
  }
}


}
}
