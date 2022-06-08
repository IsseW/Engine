#include<math/mat.h>
#include<renderer/renderer.h>

#include<Windows.h>


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
// TODO: Add a TEST define?
#ifdef _DEBUG
void run_tests();
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	Console __console{};

#ifdef _DEBUG
	run_tests();
#endif

	create_renderer(hInstance, 600, 600, nCmdShow);

	system("pause");
	return 0;
}