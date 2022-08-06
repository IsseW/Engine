#pragma once
#include"primitives.h"

__declspec(noreturn)
void __panic(const char* message, const char* file, usize line);

#define STRINGIFY_NX(str) #str
#define STRINGIFY(str) STRINGIFY_NX(str)

#define CONCAT_TO_STR(x,y) STRINGIFY(x ##y)

#define CUR_FILE_AND_LINE_STR CONCAT_TO_STR(__FILE__, __LINE__)

#define PANIC(message) __panic(message, __FILE__, __LINE__)

#define ASSERT(e) if (!(e)) { PANIC(STRINGIFY(Assertion failed: e)); }

#define ASSERT_EQ(a, b) if ((a) != (b)) { PANIC(STRINGIFY(Assertion failed: a != b)); }

#define TODO PANIC("Not implemented yet.")