#pragma once

#include"passes/shadow_pass.h"
#include"passes/first_pass.h"
#include"passes/second_pass.h"

struct RendererCtx {
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swap_chain;
	D3D11_VIEWPORT viewport;

	RenderTarget screen;

	static Result<RendererCtx, RenderCreateError> create(const Window& window);
	void resize(Vec2<u16> size);
	void clean_up();

	Vec2<u16> size() const;
	f32 ratio() const;
};

struct Window;

struct Viewpoint {
	Vec<Id<Object>> objects_in_camera;

	Mat4<f32> view;
	Mat4<f32> proj;
	Vec3<f32> pos;
	ID3D11UnorderedAccessView* render_target;
	Option<Id<Reflective>> skip_reflective;
};

struct DrawingContext {
	Vec<Viewpoint> viewpoints;
	Aabb<f32> psr_bounds;

	static DrawingContext create(const Renderer& renderer, const World& world, const AssetHandler& assets);
};

struct Renderer {
	RendererCtx ctx;

	ShadowPass shadow_pass;
	FirstPass first_pass;
	SecondPass second_pass;

	void clean_up();
	void draw(const World& world, AssetHandler& assets, f32 delta);
	void resize(const Window& window, Vec2<u16>);
	void present();

	static Result<Renderer, RenderCreateError> create(const Window& window);
};

struct DepthStencilRes {
	ID3D11Texture2D* ds_texture;
	ID3D11DepthStencilView* ds_view;
};