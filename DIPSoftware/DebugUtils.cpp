#include <cstdarg>
#include <unordered_map>

#include "DebugUtils.h"
#include "Utils.h"

using std::string;
using std::unordered_map;

void __m_assert_check__(bool val, const char *expr, const char *file, const char *func, int line) {
	if (val) {
		return;
	}
	Utils::c_fprintf(COLOR_RED, stderr, "assertion \"%s\" failed, in %s, (%s:%d)\n", expr, func, file, line);
	abort();
}

void __print_debug__(const char *file, const char *func, int line, const char *format, ...) {
	static unordered_map<int, string> colorMap;
	static int color = 0;

	if (!colorMap[line].size()) {
		colorMap[line] = Utils::int2ANSIColor(color);
		color = (color + 1) % 5;
	}

	Utils::c_fprintf(colorMap[line].c_str(), stderr, "[%s:%d] ", func, line);

	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

void error_exit(const char *msg) {
	Utils::c_fprintf(COLOR_RED, stderr, "Error occured: %s\n", msg);
	abort();
}