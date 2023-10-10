#ifndef FUNCTION_TRAITS_H
#define FUNCTION_TRAITS_H

// Corrected FunctionTraits definition, include the case for noexcept functions
template <typename T>
struct FunctionTraits;

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...) noexcept> {
  // NOLINTNEXTLINE (readability-identifier-naming)
  using return_type = ReturnType;
  // NOLINTNEXTLINE (readability-identifier-naming)
  using args_tuple = std::tuple<Args...>;
};

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...)> {
  // NOLINTNEXTLINE (readability-identifier-naming)
  using return_type = ReturnType;
  // NOLINTNEXTLINE (readability-identifier-naming)
  using args_tuple = std::tuple<Args...>;
};

#endif /* FUNCTION_TRAITS_H */
