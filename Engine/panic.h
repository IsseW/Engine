#pragma once
#include<cstdlib>
#include<iostream>

__declspec(noreturn)
void __panic(const char* message, const char* file, uint32_t line) {
	std::cerr << message << "\nFile: " << file << "\nLine: " << line << std::endl; 
	system("pause");
	exit(-1);
}

#define PANIC(message) __panic(message, __FILE__, __LINE__)