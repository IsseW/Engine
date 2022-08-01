#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct ShadowPass {
	struct Globals {
		static Globals from_light(const DirLight& light, const Camera& camera);
		static Globals from_light(const SpotLight& light, const Camera& camera);

		Mat4<f32> texture_matrix;
	};
	struct ObjectData {
		Mat4<f32> world_matrix;

		static ObjectData from_object(const Object& obj);
	};

	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;

	Uniform<Globals> globals;
	Uniform<ObjectData> object;

	DepthTextures directional_shadows;

	DepthTextures spot_shadows;

	static Result<ShadowPass, RenderCreateError> create(ID3D11Device* device, Vec2<u16> size);

	void resize(ID3D11Device* device, Vec2<u16> size);

	void draw(Renderer& rend, const World& world, const AssetHandler& assets);
	 
	void clean_up();
};
