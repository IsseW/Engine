#pragma once
#include<renderer/util.h>
#include<renderer/world.h>

struct Line {
	Line(Vec3<f32> p0, Vec3<f32> p1) : p0{ p0.with_w(0) }, p1{p1.with_w(0)} {}

	Vec4<f32> p0, p1;
};

struct DebugPass {
	ID3D11VertexShader* vs;
	ID3D11InputLayout* il;
	ID3D11PixelShader* ps;
	ID3D11Buffer* vtx_buf;

	void draw(Renderer& rend, const World& world, Vec<Line> lines);

	static Result<DebugPass, RenderCreateError> create(ID3D11Device* device);

	void clean_up();
};