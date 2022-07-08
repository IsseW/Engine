#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct ObjectRenderer {
	struct Locals {
		Mat4<f32> world_matrix;
		Vec4<f32> color;

		static Locals from_object(const Object& obj);
	};
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;
	ID3D11InputLayout* layout;

	Uniform<Locals> locals;

	static Result<ObjectRenderer, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};

struct GBuffer {
	RenderTexture albedo;
	RenderTexture normal;
	RenderTexture position;

	static Result<GBuffer, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();

	void clear(ID3D11DeviceContext* ctx);

	std::array<ID3D11RenderTargetView*, 3> targets() const;
};

struct FirstPass {
	struct Globals {
		static Globals from_world(const World& world, f32 ratio);

		Mat4<f32> view_matrix;
		Mat4<f32> proj_matrix;
	};
	ObjectRenderer object_renderer;
	Uniform<Globals> globals;

	DepthTexture depth;
	GBuffer gbuffer;

	void draw(const RendererCtx& ctx, const World& world, const AssetHandler& assets);

	static Result<FirstPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();
};