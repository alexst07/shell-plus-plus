#include "cmd-entry.h"
#include "objects/object-factory.h"
#include "interpreter/scope-executor.h"
#include "utils/scope-exit.h"

namespace seti {
namespace internal {

void CmdDeclEntry::Exec(Executor* parent, std::vector<std::string>&& args) {
  // it is the table function
  SymbolTablePtr table =
      SymbolTable::Create(SymbolTable::TableType::FUNC_TABLE);

  // main symbol of function
  symbol_table_.Push(table, false);

  BlockExecutor executor(parent, symbol_table_, true);

  // scope exit case an excpetion thrown
  auto cleanup = MakeScopeExit([&]() {
    executor.ExecuteDeferStack();
    symbol_table_.Pop();
  });
  IgnoreUnused(cleanup);

  std::vector<ObjectPtr> vec_args;

  for (auto& arg: args) {
    ObjectFactory obj_factory(symbol_table_);
    ObjectPtr str_obj(obj_factory.NewString(arg));
    vec_args.push_back(str_obj);
  }

  ObjectFactory obj_factory(symbol_table_);
  ObjectPtr array_obj(obj_factory.NewArray(std::move(vec_args)));

  // arguments as passed to command as an array called args
  symbol_table_.SetEntry("args", array_obj);

  executor.Exec(start_node_.get());
}

}
}
