#pragma once
#include<renderer/util.h>
#include<renderer/world.h>
#include"particle_system.h"

struct ObjectRenderer {
	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;

	ID3D11HullShader* hs;
	ID3D11DomainShader* ds;

	ID3D11InputLayout* layout;

	Uniform<ObjectData> object;
	Uniform<MaterialData> material;

	static Result<ObjectRenderer, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};

struct GBuffer {
	RenderTexture ambient;
	RenderTexture diffuse;
	RenderTexture specular;
	RenderTexture normal;
	RenderTexture position;

	static Result<GBuffer, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();

	void clear(ID3D11DeviceContext* ctx);

	std::array<ID3D11RenderTargetView*, 5> targets() const;
};

struct FirstPass {
	struct Globals {
		static Globals from_world(const World& world, f32 ratio);
		static Globals from_light(const Light& light, const Camera& camera);

		Mat4<f32> view_matrix;
		Mat4<f32> proj_matrix;
		Vec3<f32> cam_wpos;

		f32 dummy;
	};
	ObjectRenderer object_renderer;
	ParticleRenderer particle_renderer;
	Uniform<Globals> globals;

	DepthTexture depth;
	GBuffer gbuffer;

	ID3D11RasterizerState* rs_default;
	ID3D11RasterizerState* rs_wireframe;
	ID3D11RasterizerState* rs_cull_none;

	void draw(Renderer& rend, const World& world, const AssetHandler& assets, const Viewpoint& viewpoint);

	static Result<FirstPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void clean_up();
};