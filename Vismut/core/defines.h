#ifndef VISMUT_CORE_DEFINES_H
#define VISMUT_CORE_DEFINES_H

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define attribute_hot __attribute__((hot))
#define attribute_cold __attribute__((cold))
#define attribute_pure __attribute__((pure))
#define attribute_const __attribute__((const))
#define attribute_noinline __attribute__((noinline))
#define attribute_always_inline __attribute__((always_inline))
#define attribute_unused __attribute__((unused))
#define attribute_nonnull(...) __attribute__((nonnull(__VA_ARGS__)))
#define attribute_returns_nonnull __attribute__((returns_nonnull))
#define attribute_malloc __attribute__((malloc))
#define attribute_nodiscard __attribute__((warn_unused_result))
#define attribute_noreturn __attribute__((noreturn))
#define attribute_alloc_size(x) __attribute__((alloc_size(x)))
#define attribute_format_printf(fmt_idx, first_idx)                                                \
    __attribute__((format(printf, fmt_idx, first_idx)))
#define attribute_nullable __attribute__((_Nullable))
#define attribute_aligned(x) __attribute__((aligned(x)))
#define attribute_alloc_align(x) __attribute__((alloc_align(x)))

#define WRAPPER_START do {
#define WRAPPER_END                                                                                \
    }                                                                                              \
    while (0)

#define RISKY_FAILED(expression, err_var) unlikely((err_var = (expression)) != VISMUT_ERROR_OK)

#define SAFE_RISKY_EXPRESSION(expression, err_var)                                                 \
    if (RISKY_FAILED(expression, err_var))                                                         \
        return err_var;

#define ALIGN_UP(addr, size)                                                                       \
    ((__typeof__(addr))(((uintptr_t)(addr) + (uintptr_t)(size) - 1) & ~((uintptr_t)(size) - 1)))

#define ALIGN_DOWN(addr, size) ((addr) & ~((size) - 1))

#define COUNTOF(arr) (sizeof(arr) / sizeof(*arr))

#endif
