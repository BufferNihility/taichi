#include "cuda_driver.h"

#include "taichi/system/dynamic_loader.h"
#include "taichi/backends/cuda/cuda_context.h"

TLANG_NAMESPACE_BEGIN

std::string get_cuda_error_message(uint32 err) {
  const char *err_name_ptr;
  const char *err_string_ptr;
  CUDADriver::get_instance_without_context().get_error_name(err, &err_name_ptr);
  CUDADriver::get_instance_without_context().get_error_string(err,
                                                              &err_string_ptr);
  return fmt::format("CUDA Error {}: {}", err_name_ptr, err_string_ptr);
}

std::unique_ptr<CUDADriver> CUDADriver::instance;

CUDADriver::CUDADriver() {
#if defined(TI_PLATFORM_LINUX)
  loader = std::make_unique<DynamicLoader>("libcuda.so");
#else
  loader = std::make_unique<DynamicLoader>("nvcuda.dll");
#endif

  loader->load_function("cuGetErrorName", get_error_name);
  loader->load_function("cuGetErrorString", get_error_string);

#define PER_CUDA_FUNCTION(name, symbol_name, ...) \
  name.set(loader->load_function(#symbol_name));  \
  name.set_names(#name, #symbol_name);
#include "taichi/backends/cuda/cuda_driver_functions.inc.h"
#undef PER_CUDA_FUNCTION

  TI_INFO("CUDA driver loaded");
}

// This is for initializing the CUDA context itself
CUDADriver &CUDADriver::get_instance_without_context() {
  if (!instance)
    instance = std::make_unique<CUDADriver>();
  return *instance;
}

CUDADriver &CUDADriver::get_instance() {
  CUDAContext::get_instance();
  return get_instance_without_context();
}

TLANG_NAMESPACE_END