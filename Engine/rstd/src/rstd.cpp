#include<rstd/panic.h>
#include<cstdlib>
#include<iostream>

void __panic(const char* message, const char* file, usize line) {
	std::cerr << message << "\nFile: " << file << "\nLine: " << line << std::endl;
	system("pause");
	exit(-1);
}