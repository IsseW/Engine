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

	auto test_texture = assets.load<Image>("resources/test_texture.png");

	auto cam = Camera::perspective(Transform::from_translation(Vec3<f32>(0.0, 0.0, -10.0)), 60.0f * F32::TO_RAD);
	auto dir_light = DirLight();
	World world(cam, dir_light);
	world.add(Object(Transform(), Rgb(0.5, 0.5, 0.0)).with_mesh(cube_mesh).with_image(test_texture));

	Window* window = create_window(hInstance, 1000, 1000, nCmdShow).unwrap();

	Renderer renderer = create_renderer(*window).unwrap();
	window->set_renderer(&renderer);

	auto cleanup = [&]() {
		clean_up_ui();
		assets.clean_up();
		renderer.clean_up();
		delete window;
	};

	setup_ui(window, renderer);
	MSG msg = { };
	auto last_frame = std::chrono::high_resolution_clock::now();
	while (true) {
		window->new_frame();
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			window->update(msg);

			switch (msg.message) {
			case WM_QUIT:
			case WM_DESTROY:
				cleanup();
				PostQuitMessage(0);
				return 0;
			default:
				break;
			}
		}
		auto now = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1, 1>>>(now - last_frame).count();
		last_frame = now;

		world.update(dt, *window);

		renderer.begin_draw(world, assets);

		renderer.draw_first_pass(*window, world, assets);

		update_ui(*window, world, assets);
		renderer.present();
	}
	return 0;
}