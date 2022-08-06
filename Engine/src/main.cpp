// Detects memory leaks.
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include<math/mat.h>
#include<renderer/window.h>
#include<chrono>
#include<Windows.h>
#include<ui/ui.h>
#include<math/consts.h>
#include<renderer/transform.h>
#include<assets/shape.h>
#include<functional>

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
	// Enables memory leak collection on exit.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	run_tests();
#endif


	Window* window = create_window(hInstance, 1000, 1000, nCmdShow).unwrap();
	auto maybe_renderer = Renderer::create(*window);

	if (maybe_renderer.is_err()) {
		delete window;
		std::cout << "Err: " << maybe_renderer.unwrap_err() << std::endl;
		PANIC("Error");
	}
	Renderer renderer = maybe_renderer.unwrap();
	window->set_renderer(&renderer);

	AssetHandler assets {};
	assets.load<Mesh>(std::filesystem::path { "resources/u.wavefront" });
	assets.load<Mesh>(std::filesystem::path{ "resources/test.wavefront" });
	auto sphere = assets.load<Mesh>(std::filesystem::path{ "resources/sphere.wavefront" });
	auto croc = assets.load<Mesh>(std::filesystem::path{ "resources/croc.wavefront" });

	auto cam = Camera::perspective(Transform::from_translation({ 0.0, 2.0, -5}).looking_at({0.0}), 60.0f * F32::TO_RAD);
	World world(cam);

	auto cube_mesh = assets.insert<Mesh>(unit_cube());

	std::function<void(usize, Vec3<f32>, f32)> func = [&](usize depth, Vec3<f32> origin, f32 len) {
		Vec3<f32> origins[8];
		f32 new_len = len / 2;
		usize index = 0;
		for (usize x = 0; x <= 1; ++x) {
			for (usize y = 0; y <= 1; ++y) {
				for (usize z = 0; z <= 1; ++z) {
					Vec3<f32> offset = Vec3<f32>{
						(f32)x * 2 - 1,
						(f32)y * 2 - 1,
						(f32)z * 2 - 1,
					};
					origins[index++] = origin + offset * new_len;
				}
			}
		}
		if (depth != 0) {
			for (usize oct = 0; oct < 8; ++oct) {
				auto oct_origin = origins[oct];
				func(depth - 1, oct_origin, new_len);
			}
		}
		else {
			for (usize oct = 0; oct < 8; ++oct) {
				auto oct_origin = origins[oct];
				world.add(Object(Transform::from_translation(oct_origin).with_scale(new_len)).with_mesh(cube_mesh));
			}
		}
	};
	//func(0, {0,0,0}, 1);

	for (usize y = 0; y < 1; ++y) {
		for (usize z = 0; z <= 6; ++z) {
			for (usize x = 0; x <= 12; ++x) {
				world.add(Object(Transform::from_translation({ (f32)x * 2.5f - 15.0f, (f32)y * 1.0f, (f32)z * 5.0f - 15.0f })).with_mesh(croc));
			}
		}
	}

	world.add(Object(Transform::from_translation({ 0.0f, -10.0f, 0.0 }).with_scale(4.0)).with_mesh(sphere));

	world.add(Light::directional(Transform::from_translation({ -2.0f, 5.0f, -2.0f }).looking_at({ 0.0f }), {0.5f}));
	world.add(Light::spot(Transform::from_translation({ 0.0f, 5.0f, 0.0f }).looking_at({ 0.0f }), { 1.0 }, F32::PI / 16.0f));

	world.add(ParticleSystem::create(renderer.ctx.device, Transform::from_translation({ 15.0f, 4.0f, 0.0f }), 1000));

	world.add(Reflective(renderer.ctx.device, Transform::from_translation({ 2.0f, 4.0f, 2.0f })).with_mesh(sphere));
	
	auto cleanup = [&]() {
		clean_up_ui();
		assets.clean_up();
		world.clean_up();
		renderer.clean_up();
		delete window;
	};

	setup_ui(window, renderer);
	MSG msg = { };
	auto last_frame = std::chrono::high_resolution_clock::now();
	u64 processed_frames = 0;
	f64 fps = 0.0;
	f64 average_frametime = 0.0;
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
		f32 dt = std::chrono::duration_cast<std::chrono::duration<f32, std::ratio<1, 1>>>(now - last_frame).count();
		last_frame = now;

		world.update(dt, *window, assets);

		renderer.draw(world, assets, window->input(), dt);

		renderer.ctx.context->OMSetRenderTargets(1, &renderer.ctx.screen.rtv, nullptr);

		processed_frames += 1;
		average_frametime = dt * 1000.0 + average_frametime * (0.80 / processed_frames);
		fps = 1000.0 / average_frametime;

		update_ui(*window, renderer, world, assets, (f32)fps, (f32)average_frametime);

		renderer.present();
	}
	return 0;
}