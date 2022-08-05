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

	auto cam = Camera::perspective(Transform::from_translation({ 0.0, 5.0, -10.0 }).looking_at({0.0}), 60.0f * F32::TO_RAD);
	World world(cam);
	
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

		renderer.draw(world, assets, dt);

		renderer.ctx.context->OMSetRenderTargets(1, &renderer.ctx.screen.rtv, nullptr);

		auto fps_ts = (f64)std::chrono::duration_cast<std::chrono::nanoseconds>((std::chrono::high_resolution_clock::now() - now)).count() / 1000000.0;
		processed_frames += 1;
		average_frametime = dt * 1000.0 + average_frametime * (0.80 / processed_frames);
		if (processed_frames >= 60) {
			fps = 1000.0 / average_frametime;
			processed_frames = 0;
		}

		update_ui(*window, renderer, world, assets, (f32)fps, (f32)average_frametime);

		renderer.present();
	}
	return 0;
}