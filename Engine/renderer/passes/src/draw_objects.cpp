#include<renderer/passes/draw_objects.h>

void draw_objects(Renderer& rend, const World& world, const AssetHandler& assets, FirstPass::Globals globals, bool pixel_shader) {
	rend.ctx.context->RSSetState(rend.first_pass.rs_default);
	rend.ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	rend.first_pass.globals.update(rend.ctx.context, &globals);

	const usize UNIFORM_COUNT = 3;
	ID3D11Buffer* uniforms[UNIFORM_COUNT] = { rend.first_pass.globals.buffer, rend.first_pass.object_renderer.object.buffer, rend.first_pass.object_renderer.material.buffer };
	rend.ctx.context->VSSetConstantBuffers(0, UNIFORM_COUNT, uniforms);
	rend.ctx.context->IASetInputLayout(rend.first_pass.object_renderer.layout);


	rend.ctx.context->VSSetShader(rend.first_pass.object_renderer.vs, nullptr, 0);
	
	if (pixel_shader) {
		rend.ctx.context->PSSetConstantBuffers(0, UNIFORM_COUNT, uniforms);
		rend.ctx.context->PSSetShader(rend.first_pass.object_renderer.ps, nullptr, 0);
	}
	else {
		rend.ctx.context->PSSetShader(nullptr, nullptr, 0);
	}

	auto bind_mat = [&](Option<AId<Material>> mat_handle) {
		const Material* mat = assets.get_or_default(mat_handle);
		auto bind_image = [&](u32 slot, Option<AId<Image>> image_handle) {
			const Image* image = assets.get_or_default(image_handle);
			image->binded.as_ptr().then_do([&](const Image::Binded* binded) {
				rend.ctx.context->PSSetShaderResources(slot, 1, &binded->srv);
				rend.ctx.context->PSSetSamplers(slot, 1, &binded->sampler_state);
				});
		};
		bind_image(0, mat->ambient.tex);
		bind_image(1, mat->diffuse.tex);
		bind_image(2, mat->specular.tex);
		bind_image(3, mat->shinyness.tex);

		MaterialData data = mat->get_data();
		rend.first_pass.object_renderer.material.update(rend.ctx.context, &data);
	};

	world.objects.values([&](const Object& obj) {
		ObjectData object_data = obj.get_data();
		rend.first_pass.object_renderer.object.update(rend.ctx.context, &object_data);

		const Mesh* mesh = assets.get_or_default(obj.mesh);
		if (mesh->binded.is_none()) {
			std::cerr << "Mesh needed for drawing isn't bound." << std::endl;
			return;
		}
		auto binded = mesh->binded.as_ptr().unwrap_unchecked();
		u32 stride = sizeof(Vertex);
		u32 offset = 0;
		rend.ctx.context->IASetVertexBuffers(0, 1, &binded->vertex_buffer, &stride, &offset);
		rend.ctx.context->IASetIndexBuffer(binded->index_buffer, DXGI_FORMAT_R16_UINT, 0);
		for (const SubMesh& sub_mesh : mesh->submeshes) {
			if (pixel_shader) {
				bind_mat(sub_mesh.material);
			}
			rend.ctx.context->DrawIndexed(sub_mesh.end_index - sub_mesh.start_index, sub_mesh.start_index, 0);
		}
	});

	rend.ctx.context->RSSetState(rend.first_pass.rs_cull_none);

	rend.ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	rend.ctx.context->VSSetShader(rend.first_pass.particle_renderer.vs, nullptr, 0);
	rend.ctx.context->IASetInputLayout(rend.first_pass.particle_renderer.il);
	rend.ctx.context->IASetIndexBuffer(rend.first_pass.particle_renderer.quad_index_buffer, DXGI_FORMAT_R16_UINT, 0);
	

	rend.ctx.context->GSSetShader(rend.first_pass.particle_renderer.gs, nullptr, 0);
	uniforms[1] = rend.first_pass.particle_renderer.gs_globals.buffer;
	rend.ctx.context->GSSetConstantBuffers(0, 2, uniforms);

	world.particle_systems.values([&](const ParticleSystem& system) {
		auto globals = ParticleRenderer::Globals{
			system.start_size,
		};
		rend.first_pass.particle_renderer.gs_globals.update(rend.ctx.context, &globals);

		auto data = system.get_object_data();
		rend.first_pass.object_renderer.object.update(rend.ctx.context, &data);

		if (pixel_shader) {
			bind_mat(system.material);
		}
		u32 stride = sizeof(Vertex);
		u32 offset = 0;

		rend.ctx.context->IASetVertexBuffers(0, 1, &system.particles.buffer, &stride, &offset);
		rend.ctx.context->Draw(system.num_particles(), 0);
		// rend.ctx.context->Draw(system.num_particles(), 0);
	});
	// Unbind
	rend.ctx.context->GSSetShader(nullptr, nullptr, 0);
	ID3D11Buffer* null = nullptr;
	u32 stride = sizeof(Vertex);
	u32 offset = 0;
	rend.ctx.context->IASetVertexBuffers(0, 1, &null, &stride, &offset);
}