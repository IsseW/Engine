#include<math/mat.h>
#include<renderer/window.h>
#include<chrono>
#include<Windows.h>
#include<ui/ui.h>
#include<math/consts.h>
#include<renderer/transform.h>
#include<assets/shape.h>


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

	AssetHandler assets {};

	auto cube_mesh = assets.insert(unit_cube());
	assets.load<Mesh>("resources/u.wavefront");
	assets.load<Mesh>("resources/test.wavefront");

	auto cam = Camera::perspective(Transform::from_translation(Vec3<f32>(5.0, 0.0, 0.0)).looking_at(Vec3<f32>::unit_y()), 60.0f * F32::TO_RAD);
	auto dir_light = DirLight();
	World world(cam, dir_light);
	world.add(Object(Transform(), cube_mesh, Rgb(0.5, 0.5, 0.0)));
	world.add(Object(Transform(), cube_mesh, Rgb(0.5, 0.0, 0.5)));

	Window* window = create_window(hInstance, 1600, 1600, nCmdShow).unwrap();

	Renderer renderer = create_renderer(window).unwrap();
	window->set_renderer(&renderer);

	setup_ui(window, renderer);
	MSG msg = { };
	auto last_frame = std::chrono::high_resolution_clock::now();
	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT) {
		auto now = std::chrono::high_resolution_clock::now();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (!ui_handle_input(msg.hwnd, msg.message, msg.wParam, msg.lParam)) {
				switch (msg.message) {
				case WM_DESTROY:
					PostQuitMessage(0);
					return 0;
				default:
					break;
				}
			} 
		}
		float dt = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - last_frame).count();
		// world.update(dt);
		last_frame = now;

		renderer.begin_draw(world, assets);

		renderer.draw_first_pass(window, world, assets);

		update_ui(window, world, assets);
		renderer.present();
	}
	clean_up_ui();
	assets.clean_up();
	renderer.clean_up();
	delete window;

	return 0;
}