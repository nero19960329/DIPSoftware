#pragma once

void __m_assert_check__(bool val, const char *expr, const char *file, const char *func, int line);
void error_exit(const char *msg);

#ifdef _DEBUG

#define m_assert(expr) __m_assert_check__(expr, # expr, __FILE__, __FUNCTION__, __LINE__)
#define print_debug(fmt, ...) __print_debug__(__FILE__, __FUNCTION__, __LINE__, fmt, ## __VA_ARGS__)

void __print_debug__(const char *file, const char *func, int line, const char *format, ...);

#else

#define m_assert(expr)
#define print_debug(fmt, ...)

#endif