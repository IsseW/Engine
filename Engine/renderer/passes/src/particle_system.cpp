#include<renderer/passes/particle_system.h>
#include<renderer/renderer.h>

void ParticleRenderer::run(Renderer& rend, const World& world, f32 delta_time) {
	rend.ctx.context->CSSetShader(cs, nullptr, 0);
	rend.ctx.context->CSSetConstantBuffers(0, 1, &system_data.buffer);
	world.particle_systems.values([&](const ParticleSystem& system) {

		if (system.paused) {
			return;
		}

		auto data = system.get_data(world.time, delta_time);
		system_data.update(rend.ctx.context, &data);
		ID3D11UnorderedAccessView* uavs[2] = {
			system.particles.uav,
			system.particle_data.uav,
		};
		rend.ctx.context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);
		rend.ctx.context->Dispatch(system.num_particles(), 1, 1);
	});
	// Unbind
	ID3D11UnorderedAccessView* uavs[2] = {nullptr, nullptr};
	rend.ctx.context->CSSetUnorderedAccessViews(0, 2, uavs, nullptr);
}

Result<ParticleRenderer, RenderCreateError> ParticleRenderer::create(ID3D11Device* device)
{
	VSIL vsil;
	TRY(vsil, load_vertex(device, "particle_vertex.cso", { { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } }));

	ID3D11ComputeShader* cs;
	TRY(cs, load_compute(device, "particle_compute.cso"));

	ID3D11GeometryShader* gs;
	TRY(gs, load_geometry(device, "particle_geometry.cso"));

	Uniform<ParticleSystemData> system_data;
	TRY(system_data, Uniform<ParticleSystemData>::create(device));

	Uniform<Globals> gs_globals;
	TRY(gs_globals, Uniform<Globals>::create(device));

	Index indices[] = {
		0,  1,  2,
		1,  2,  3,
	};
	D3D11_BUFFER_DESC buffer_desc;
	buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	buffer_desc.ByteWidth = sizeof(Index) * 6;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = indices;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	ID3D11Buffer* quad_index_buffer;
	if (FAILED(device->CreateBuffer(&buffer_desc, &data, &quad_index_buffer))) {
		return FailedBufferCreation;
	}

	return ok<ParticleRenderer, RenderCreateError>(ParticleRenderer{
		vsil.vs,
		vsil.il,
		cs,
		gs,
		system_data,
		gs_globals,
		quad_index_buffer,
	});
}

void ParticleRenderer::clean_up() {
	vs->Release();
	il->Release();

	cs->Release();
	gs->Release();

	system_data.clean_up();
	gs_globals.clean_up();

	quad_index_buffer->Release();
}
