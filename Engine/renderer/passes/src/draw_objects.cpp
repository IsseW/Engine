#include<renderer/passes/draw_objects.h>

void draw_objects(Renderer& rend, const World& world, const AssetHandler& assets, FirstPass::Globals globals, bool pixel_shader, 
	const Vec<Id<Object>>& objects_to_draw, const Vec<Id<Reflective>>& reflectives_to_draw)
{
	rend.ctx.context->RSSetState(rend.first_pass.rs_default);

	rend.first_pass.globals.update(rend.ctx.context, &globals);

	const usize UNIFORM_COUNT = 3;
	ID3D11Buffer* uniforms[UNIFORM_COUNT] = { rend.first_pass.globals.buffer, rend.first_pass.object_renderer.object.buffer, rend.first_pass.object_renderer.material.buffer };
	rend.ctx.context->VSSetConstantBuffers(0, UNIFORM_COUNT, uniforms);
	rend.ctx.context->IASetInputLayout(rend.first_pass.object_renderer.layout);

	rend.ctx.context->HSSetConstantBuffers(0, 1, &rend.first_pass.object_renderer.detail_data.buffer);
	rend.ctx.context->DSSetConstantBuffers(0, 2, uniforms);
	
	if (pixel_shader) {
		rend.ctx.context->PSSetConstantBuffers(0, UNIFORM_COUNT, uniforms);
		rend.ctx.context->PSSetShader(rend.first_pass.object_renderer.ps, nullptr, 0);
		rend.ctx.context->PSSetSamplers(0, 1, &rend.first_pass.object_renderer.sampler_state);
	}

	auto bind_mat = [&](Option<AId<Material>> mat_handle) {
		auto bind_image = [&](u32 slot, Option<AId<Image>> image_handle) {
			const Image* image = assets.get_or_default(image_handle);
			image->binded.as_ptr().then_do([&](const Image::Binded* binded) {
				rend.ctx.context->PSSetShaderResources(slot, 1, &binded->srv);
			});
		};
		const Material* mat = assets.get_or_default(mat_handle);
		bind_image(0, mat->ambient.tex);
		bind_image(1, mat->diffuse.tex);
		bind_image(2, mat->specular.tex);
		bind_image(3, mat->normal.tex);

		MaterialData data = mat->get_data();
		rend.first_pass.object_renderer.material.update(rend.ctx.context, &data);
	};

	auto bind_mesh = [&](Option<AId<Mesh>> mesh_handle) {
		const Mesh* mesh = assets.get_or_default(mesh_handle);
		if (mesh->binded.is_none()) {
			return (const Mesh*)nullptr;
		}
		auto binded = mesh->binded.as_ptr().unwrap_unchecked();
		u32 stride = sizeof(Vertex);
		u32 offset = 0;
		rend.ctx.context->IASetVertexBuffers(0, 1, &binded->vertex_buffer, &stride, &offset);
		rend.ctx.context->IASetIndexBuffer(binded->index_buffer, DXGI_FORMAT_R16_UINT, 0);
		return mesh;
	};

	bool tesselation_on = true;
	auto set_tesselation = [&](bool on, Aabb<f32> bounds) {
		if (tesselation_on && !on) {
			rend.ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			rend.ctx.context->HSSetShader(nullptr, nullptr, 0);
			rend.ctx.context->DSSetShader(nullptr, nullptr, 0);
			rend.ctx.context->VSSetShader(rend.first_pass.object_renderer.vs, nullptr, 0);
			tesselation_on = false;
		}
		else if (!tesselation_on && on) {
			rend.ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			rend.ctx.context->HSSetShader(rend.first_pass.object_renderer.hs, nullptr, 0);
			rend.ctx.context->DSSetShader(rend.first_pass.object_renderer.ds, nullptr, 0);
			rend.ctx.context->VSSetShader(rend.first_pass.object_renderer.tess_vs, nullptr, 0);
			tesselation_on = true;
		}
		if (tesselation_on) {

			auto data = DetailData{ 8.0f / (1.0f + bounds.distance_to(world.camera.transform.translation)) };
			rend.first_pass.object_renderer.detail_data.update(rend.ctx.context, &data);
		}
	};
	set_tesselation(false, {});

	for (const auto& id : objects_to_draw) {
		const auto& obj = *world.objects.get(id).unwrap();
		set_tesselation(obj.tesselate, obj.bounds);
		ObjectData object_data = obj.transform.get_data();
		rend.first_pass.object_renderer.object.update(rend.ctx.context, &object_data);

		auto mesh = bind_mesh(obj.mesh);
		if (mesh) {
			for (const SubMesh& sub_mesh : mesh->submeshes) {
				if (pixel_shader) {
					bind_mat(sub_mesh.material);
				}
				rend.ctx.context->DrawIndexed(sub_mesh.end_index - sub_mesh.start_index, sub_mesh.start_index, 0);
			}
		}
	}

	if (pixel_shader) {
		rend.ctx.context->PSSetShader(rend.first_pass.object_renderer.refl_ps, nullptr, 0);
	}

	for (const auto& id : reflectives_to_draw) {
		const auto& reflective = *world.reflective.get(id).unwrap();
		set_tesselation(reflective.tesselate, reflective.bounds);

		ObjectData object_data = reflective.transform.get_data();
		rend.first_pass.object_renderer.object.update(rend.ctx.context, &object_data);

		auto mesh = bind_mesh(reflective.mesh);

		if (mesh) {
			if (pixel_shader) {
				rend.ctx.context->PSSetShaderResources(4, 1, &reflective.cube_texture.srv);
			}

			for (const SubMesh& sub_mesh : mesh->submeshes) {
				if (pixel_shader) {
					bind_mat(sub_mesh.material);
				}
				rend.ctx.context->DrawIndexed(sub_mesh.end_index - sub_mesh.start_index, sub_mesh.start_index, 0);
			}
		}
	}
	set_tesselation(false, {});

	if (pixel_shader) {
		rend.ctx.context->PSSetShader(rend.first_pass.object_renderer.ps, nullptr, 0);
	}

	rend.ctx.context->RSSetState(rend.first_pass.rs_cull_none);

	rend.ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	rend.ctx.context->VSSetShader(rend.first_pass.particle_renderer.vs, nullptr, 0);
	rend.ctx.context->IASetInputLayout(rend.first_pass.particle_renderer.il);
	

	rend.ctx.context->GSSetShader(rend.first_pass.particle_renderer.gs, nullptr, 0);
	uniforms[1] = rend.first_pass.particle_renderer.gs_globals.buffer;
	rend.ctx.context->GSSetConstantBuffers(0, 2, uniforms);

	world.particle_systems.values([&](const ParticleSystem& system) {
		auto globals = ParticleRenderer::Globals{
			system.start_size,
		};
		rend.first_pass.particle_renderer.gs_globals.update(rend.ctx.context, &globals);

		auto data = system.transform.get_data();
		rend.first_pass.object_renderer.object.update(rend.ctx.context, &data);

		if (pixel_shader) {
			bind_mat(system.material);
		}
		u32 stride = sizeof(Vertex);
		u32 offset = 0;

		rend.ctx.context->IASetVertexBuffers(0, 1, &system.particles.buffer, &stride, &offset);
		rend.ctx.context->Draw(system.num_particles(), 0);
	});
	// Unbind
	ID3D11ShaderResourceView* views[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	rend.ctx.context->PSSetShaderResources(0, 5, views);
	rend.ctx.context->VSSetShader(nullptr, nullptr, 0);
	rend.ctx.context->HSSetShader(nullptr, nullptr, 0);
	rend.ctx.context->DSSetShader(nullptr, nullptr, 0);
	rend.ctx.context->GSSetShader(nullptr, nullptr, 0);
	rend.ctx.context->PSSetShader(nullptr, nullptr, 0);
	ID3D11Buffer* null = nullptr;
	u32 stride = sizeof(Vertex);
	u32 offset = 0;
	rend.ctx.context->IASetVertexBuffers(0, 1, &null, &stride, &offset);
}