#include "ir.h"

TLANG_NAMESPACE_BEGIN

class IRPrinter : public IRVisitor {
 public:
  int current_indent;

  IRPrinter() {
    current_indent = -1;
  }

  template <typename... Args>
  void print(std::string f, Args &&... args) {
    print_raw(fmt::format(f, std::forward<Args>(args)...));
  }

  void print_raw(std::string f) {
    for (int i = 0; i < current_indent; i++)
      fmt::print("  ");
    std::cout << f;
    fmt::print("\n");
  }

  static void run(IRNode *node) {
    auto p = IRPrinter();
    fmt::print("==========\n");
    node->accept(&p);
    fmt::print("==========\n");
  }

  void visit(Block *stmt_list) {
    current_indent++;
    for (auto &stmt : stmt_list->statements) {
      stmt->accept(this);
    }
    current_indent--;
  }

  void visit(AssignStmt *assign) {
    print("{} = {}", assign->lhs->serialize(), assign->rhs->serialize());
  }

  void visit(AllocaStmt *alloca) {
    print("{}alloca {}", alloca->type_hint(), alloca->ident.name());
  }

  void visit(BinaryOpStmt *bin) {
    print("{}{} = {} {} {}", bin->type_hint(), bin->name(),
          binary_type_name(bin->op_type), bin->lhs->name(), bin->rhs->name());
  }

  void visit(IfStmt *if_stmt) {
    print("if {} {{", if_stmt->cond->name());
    if (if_stmt->true_statements)
      if_stmt->true_statements->accept(this);
    if (if_stmt->false_statements) {
      print("}} else {{");
      if_stmt->false_statements->accept(this);
    }
    print("}}");
  }

  void visit(FrontendIfStmt *if_stmt) {
    print("if {} {{", if_stmt->condition->serialize());
    if (if_stmt->true_statements)
      if_stmt->true_statements->accept(this);
    if (if_stmt->false_statements) {
      print("}} else {{");
      if_stmt->false_statements->accept(this);
    }
    print("}}");
  }

  void visit(FrontendPrintStmt *print_stmt) {
    print("print {}", print_stmt->expr.serialize());
  }

  void visit(PrintStmt *print_stmt) {
    print("{}print {}", print_stmt->type_hint(), print_stmt->stmt->name());
  }

  void visit(ConstStmt *const_stmt) {
    print("{}{} = const {}", const_stmt->type_hint(), const_stmt->name(),
          const_stmt->value.serialize());
  }

  void visit(WhileControlStmt *stmt) {
    print("while control {}, {}", stmt->mask.name(), stmt->cond->name());
  }

  void visit(WhileStmt *stmt) {
    print("while 1 {{");
    stmt->body->accept(this);
    print("}}");
  }

  void visit(FrontendWhileStmt *stmt) {
    print("while {} {{", stmt->cond->serialize());
    stmt->body->accept(this);
    print("}}");
  }

  void visit(FrontendForStmt *for_stmt) {
    print("for {} in range({}, {}) {{", for_stmt->loop_var_id.name(),
          for_stmt->begin->serialize(), for_stmt->end->serialize());
    for_stmt->body->accept(this);
    print("}}");
  }

  void visit(RangeForStmt *for_stmt) {
    print("for {} in range({}, {}, step {}) {{", for_stmt->loop_var.name(),
          for_stmt->begin->name(), for_stmt->end->name(), for_stmt->vectorize);
    for_stmt->body->accept(this);
    print("}}");
  }

  void visit(GlobalPtrStmt *stmt) {
    std::string snode_name;
    if (stmt->snode) {
      snode_name = stmt->snode->name;
    } else {
      snode_name = "unknown";
    }
    std::string s = fmt::format("{}{} = ptr {}[", stmt->type_hint(),
                                stmt->name(), snode_name);

    for (int i = 0; i < (int)stmt->indices.size(); i++) {
      s += fmt::format("{}", stmt->indices[i]->name());
      if (i + 1 < (int)stmt->indices.size()) {
        s += ", ";
      }
    }
    s += "]";
    print_raw(s);
  }

  void visit(LocalLoadStmt *stmt) {
    print("{}{} = load {}", stmt->type_hint(), stmt->name(),
          stmt->ident.name());
  }

  void visit(LocalStoreStmt *stmt) {
    print("[local store] {} = {}", stmt->ident.name(), stmt->stmt->name());
  }

  void visit(GlobalLoadStmt *stmt) {
    print("{}{} = load {}", stmt->type_hint(), stmt->raw_name(),
          stmt->ptr->name());
  }

  void visit(GlobalStoreStmt *stmt) {
    print("[global store] {} = {}", stmt->ptr->name(), stmt->data->name());
  }
};

namespace irpass {

void print(IRNode *root) {
  return IRPrinter::run(root);
}

}  // namespace irpass

TLANG_NAMESPACE_END
