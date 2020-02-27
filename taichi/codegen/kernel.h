// Driver class for source2source kernel codegen. This file is OBSOLETE

#pragma once

#include "taichi/tlang_util.h"
#include "taichi/program.h"

TLANG_NAMESPACE_BEGIN

class KernelCodeGen {
 public:
  Program *prog;
  Kernel *kernel;
  KernelCodeGen(const std::string &kernel_name) {
  }

  virtual void lower() = 0;

  virtual void codegen() {
    TI_NOT_IMPLEMENTED
  }

  virtual FunctionType codegen_llvm() {
    TI_NOT_IMPLEMENTED;
    return nullptr;
  }

  virtual FunctionType compile(Program &prog, Kernel &kernel);
};

TLANG_NAMESPACE_END