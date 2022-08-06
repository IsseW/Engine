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

void setup_scene(World& world, Renderer& renderer, AssetHandler& assets) {
	world.add(Light::directional(Transform::from_translation({ -2.0f, 5.0f, -2.0f }).looking_at({ 0.0f }), { 0.5f }));
	auto mat_group_handle = assets.load<MaterialGroup>(fs::path { "resources\\icons.mtl" });
	auto mat_group = assets.get(mat_group_handle).unwrap();

	auto particle_system = 
		ParticleSystem::create(renderer.ctx.device, Transform::from_translation({ -15.0f, 30.0f, -15.0f }), 10000);
	particle_system.material = mat_group->get("Veloren");
	
	particle_system.min_spawn_point = { -10.0f, -0.5f, -10.0f };
	particle_system.max_spawn_point = { 10.0f, 0.5f, 10.0f };

	particle_system.start_dir = { 0.0f, -0.1f, 0.0f };

	particle_system.start_angle_random = 0.2f;

	particle_system.vel_magnitude_min = 0.0f;
	particle_system.vel_magnitude_max = 0.1f;

	particle_system.acceleration = { 0.0f, -9.8f, 0.0f };
	particle_system.min_life_time = 2.0f;
	particle_system.max_life_time = 4.0f;
	particle_system.start_size = 0.2f;

	world.add(std::move(particle_system));
	
	auto look_dir = Vec3<f32>{ 1.0f, 2.0f, 2.0f }.normalized();
	auto forward_dir = look_dir.cross(Vec3<f32>::unit_z()).normalized();
	auto up_dir = look_dir.cross(forward_dir);

	auto rot = Quat<f32>::looking_dir(look_dir, forward_dir, up_dir);
	rot = Quat<f32>::from_euler(Vec3<f32>{0.0f, 90.0f, 80.0f} * F32::TO_RAD);

	world.add(Object(Transform::from_translation({ 20.0f, 1.0f, 5.0f }).with_rotation(rot))
			.with_mesh(assets.load<Mesh>(fs::path{ "resources/quad_rust.wavefront" })));

	world.add(Object(Transform::from_translation({ 20.0f, 1.0f, 7.0f }).with_rotation(rot))
		.with_mesh(assets.load<Mesh>(fs::path{ "resources/quad_bevy.wavefront" })));

	auto quad = assets.load<Mesh>(fs::path{ "resources/quad_veloren.wavefront" });

	world.add(Object(Transform::from_translation({ 20.0f, 1.0f, 9.0f }).with_rotation(rot))
		.with_mesh(quad));

	auto croc_mesh = assets.load<Mesh>(std::filesystem::path{ "resources/croc.wavefront" });

	Vec3<f32> center { 15.0f, 0.0f, -15.0f };
	f32 radius = 8.0f;
	Vec3<f32> colors[6] = {
		{0.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 1.0f},
		{1.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
	};
	for (u32 z = 0; z < 6; ++z) {
		Vec3<f32> p = center + Vec3<f32> {0.0f, 0.0f, -(f32)z * 10.0f};
		for (f32 a = 0.0f; a <= 360.0f; a += 15.0f) {
			auto rot = Quat<f32>::from_euler(Vec3<f32>{ 0.0f, 0.0f, a } *F32::TO_RAD);
			auto dir = rot* Vec3<f32>::unit_x();
			world.add(Object(Transform::from_translation(p + dir * radius)).with_mesh(croc_mesh));
		}
		f32 offset = radius / 2.0f;

		world.add(Light::spot(
			Transform::from_translation(p + Vec3<f32>{(f32)(z % 2)* offset - (f32)(z % 2 + 1) * offset, radius * 2.0f, 0.0})
			.looking_at(p), colors[z], 13.0f * F32::TO_RAD));
	}
	auto sphere = assets.load<Mesh>(std::filesystem::path{ "resources/sphere.wavefront" });
	world.add(Reflective(renderer.ctx.device, 
		Transform::from_translation(center + Vec3<f32>{0.0f, 0.0f, -25.0f}).with_scale(4.0f)).with_mesh(sphere));

	world.add(Object(Transform::from_translation(center + Vec3<f32>{0.0f, 0.0f, -5.0f}).with_scale(0.5f)).with_mesh(sphere));
	world.add(Object(Transform::from_translation(center + Vec3<f32>{0.0f, 0.0f, -15.0f}).with_scale(0.5f)).with_mesh(sphere));
	world.add(Object(Transform::from_translation(center + Vec3<f32>{0.0f, 0.0f, -30.0f}).with_scale(0.5f)).with_mesh(sphere));

	auto cube = assets.load<Mesh>(std::filesystem::path{ "resources/cube.wavefront" });

	world.add(Reflective(renderer.ctx.device, Transform::from_translation(center)).with_mesh(cube));
	

	world.add(Object(
		Transform::from_translation(Vec3<f32>{0.0f, -10.0f, 0.0f})
			.with_scale(100.0)).with_mesh(quad));

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	Console __console{};

#ifdef _DEBUG
	// Enables memory leak collection on exit.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	run_tests();
#endif


	Window* window = create_window(hInstance, { 1000, 1000 }, nCmdShow).unwrap();
	auto maybe_renderer = Renderer::create(*window);

	if (maybe_renderer.is_err()) {
		delete window;
		std::cout << "Err: " << maybe_renderer.unwrap_err() << std::endl;
		PANIC("Error");
	}
	Renderer renderer = maybe_renderer.unwrap();
	window->set_renderer(&renderer);

	AssetHandler assets {}; 

	auto cam = Camera::perspective(Transform::from_translation({ 0.0, 2.0, -5}).looking_at({0.0}), 60.0f * F32::TO_RAD);
	World world(cam);
	
	setup_scene(world, renderer, assets);

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