#include"option.h"
#include"result.h"
#include"tuple.h"
#include <Windows.h>

struct Console {

	Console() {
		AllocConsole();
		freopen_s(&console_err, "CONOUT$", "w", stderr);
		freopen_s(&console_out, "CONOUT$", "w", stdout);
		freopen_s(&console_in, "CONIN$", "r", stdin);
	}
	~Console() {
		if (console_err) {
			fclose(console_err);
		}
		if (console_out) {
			fclose(console_out);
		}
		if (console_in) {
			fclose(console_in);
		}
	}
private:
	FILE* console_err;
	FILE* console_out;
	FILE* console_in;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	Console __console{};

	Option<int> t{32};
	int i = t.unwrap();
	Option<int> a{};
	

	std::cerr << "wa\n";
	std::cout << i;
	int x;
	std::cin >> x;
	a.unwrap();

}