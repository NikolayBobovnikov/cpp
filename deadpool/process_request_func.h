#ifndef PROCESS_REQUEST_FUNC_H
#define PROCESS_REQUEST_FUNC_H

#include <external_api.h>
#include <function_traits.hpp>

// Correct the static_asserts to properly refer to the FunctionTraits
class ProcessRequestFunc
{
 public:
  using FuncType = decltype(&ProcessRequest);

  ProcessRequestFunc(FuncType func);

  // Remaining code is the same
  void operator()(Request *req, Stopper stopper) const noexcept;

 private:
  FuncType m_func;
};

#endif /* PROCESS_REQUEST_FUNC_H */
