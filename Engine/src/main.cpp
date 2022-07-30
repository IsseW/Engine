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
	assets.load<Mesh>("resources/sphere.wavefront");

	auto test_texture = assets.load<Image>("resources/test_texture.png");

	auto cam = Camera::perspective(Transform::from_translation(Vec3<f32>(0.0, 0.0, -10.0)), 60.0f * F32::TO_RAD);
	World world(cam);
	world.add(Object(Transform(), Vec3<f32>(0.5f, 0.5f, 0.0f)).with_mesh(cube_mesh).with_image(test_texture));
	world.add(DirLight(Transform::from_translation(Vec3<f32>(-2.0f, 5.0f, -2.0f)).looking_at(Vec3<f32>::zero()), Light { Vec3<f32>::one(), 1.0 }));

	Window* window = create_window(hInstance, 1000, 1000, nCmdShow).unwrap();

	auto maybe_renderer = Renderer::create(*window);

	if (maybe_renderer.is_err()) {
		std::cout << "Err: " << maybe_renderer.unwrap_err() << std::endl;
	}

	Renderer renderer = maybe_renderer.unwrap();
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

		renderer.draw(world, assets);

		renderer.ctx.context->OMSetRenderTargets(1, &renderer.ctx.screen.rtv, nullptr);

		update_ui(*window, renderer, world, assets);
		renderer.present();
	}
	return 0;
}