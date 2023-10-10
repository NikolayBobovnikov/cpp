#include <process_request_func.h>

ProcessRequestFunc::ProcessRequestFunc(FuncType func)
    : m_func(func)
{
  using Traits = FunctionTraits<FuncType>;
  static_assert(std::is_same_v<typename Traits::return_type, void>,
                "Return type must be void");
  static_assert(
      std::is_same_v<std::tuple_element_t<0, typename Traits::args_tuple>,
                     Request *>,
      "First argument must be Request*");
  static_assert(
      std::is_same_v<std::tuple_element_t<1, typename Traits::args_tuple>,
                     Stopper>,
      "Second argument must be Stopper");
}

// Remaining code is the same
void ProcessRequestFunc::operator()(Request *req,
                                    Stopper stopper) const noexcept
{
  m_func(req, std::move(stopper));
}