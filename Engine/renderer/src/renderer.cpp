#include<renderer/renderer.h>
#include<renderer/window.h>

const float clear_color_with_alpha[4] = { 0,0,0,0 };

void clean_up_ctx(RendererCtx& ctx) {
	ctx.ds_view->Release();
	ctx.ds_texture->Release();
	ctx.rtv->Release();
	ctx.swap_chain->Release();
	ctx.context->Release();
	ctx.device->Release();
}

void clean_up_object_renderer(ObjectRenderer& object_renderer) {
	object_renderer.layout->Release();
	object_renderer.ps->Release();
	object_renderer.vs->Release();
}

void clean_up_first_pass(FirstPass& first_pass) {
	first_pass.globals.buffer->Release();
	clean_up_object_renderer(first_pass.object_renderer);
}

ObjectRenderer::Locals ObjectRenderer::Locals::from_object(const Object& obj) {
	return Locals{
		obj.transform.get_mat().transposed(),
		obj.color.with_w(1.0)
	};
}

void Renderer::clean_up() {
	clean_up_ctx(ctx);
	clean_up_first_pass(first_pass);
}

void Renderer::resize(Vec2<u16> size)  {
	if (size.x <= 0 || size.y <= 0) {
		return;
	}
	ctx.viewport.Width = (f32)size.x;
	ctx.viewport.Height = (f32)size.y;
	if (ctx.rtv) { ctx.rtv->Release();  }
	if (ctx.ds_texture) ctx.ds_texture->Release();
	if (ctx.ds_view)ctx.ds_view->Release();
	auto depth_stencil = create_depth_stencil(ctx.device, size.x, size.y).unwrap();
	ctx.ds_view = depth_stencil.ds_view;
	ctx.ds_texture = depth_stencil.ds_texture;
	ctx.swap_chain->ResizeBuffers(0, size.x, size.y, DXGI_FORMAT_UNKNOWN, 0);
	ctx.rtv = create_render_target_view(ctx.device, ctx.swap_chain).unwrap();
}

void Renderer::begin_draw(const World& world, AssetHandler& assets) {
	// Do some setup and bind required resource before we draw.
	world.objects.values([&](const Object* obj) {
		assets.get_or_default(obj->mesh)->bind(ctx.device);
		assets.get_or_default(obj->image)->bind(ctx.device);
	});
	

	ctx.context->OMSetRenderTargets(1, &ctx.rtv, ctx.ds_view);
	ctx.context->RSSetViewports(1, &ctx.viewport);
	ctx.context->ClearRenderTargetView(ctx.rtv, clear_color_with_alpha);
	ctx.context->ClearDepthStencilView(ctx.ds_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

Globals Globals::from_world(const World& world, f32 ratio) {
	return Globals{
		world.camera.get_view().transposed(),
		world.camera.get_proj(ratio).transposed(),
	};
}

void Renderer::draw_first_pass(const Window& window, const World& world, const AssetHandler& assets) {
	// First update the globals buffer
	auto globals = Globals::from_world(world, window.ratio());
	first_pass.globals.update(ctx.context, &globals);

	ctx.context->VSSetShader(first_pass.object_renderer.vs, nullptr, 0);
	ctx.context->PSSetShader(first_pass.object_renderer.ps, nullptr, 0);

	ID3D11Buffer* uniforms[2] = { first_pass.globals.buffer, first_pass.object_renderer.locals.buffer };
	ctx.context->VSSetConstantBuffers(0, 2, uniforms);
	ctx.context->PSSetConstantBuffers(0, 2, uniforms);
	ctx.context->IASetInputLayout(first_pass.object_renderer.layout);
	ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	world.objects.values([&](const Object* obj) {
		auto locals = ObjectRenderer::Locals::from_object(*obj);
		first_pass.object_renderer.locals.update(ctx.context, &locals);

		const Image* image = assets.get_or_default(obj->image);

		if (image->binded.is_none()) {
			return;
		}
		auto binded = image->binded.as_ptr().unwrap_unchecked();

		ctx.context->PSSetShaderResources(0, 1, &binded->rsv);
		ctx.context->PSSetSamplers(0, 1, &binded->sampler_state);

		const Mesh* mesh = assets.get_or_default(obj->mesh);
		for (const SubMesh& sub_mesh : mesh->submeshes) {
			if (sub_mesh.binded.is_none()) {
				continue;
			}

			auto binded = sub_mesh.binded.as_ptr().unwrap_unchecked();
			u32 stride = sizeof(Vertex);
			u32 offset = 0;
			ctx.context->IASetVertexBuffers(0, 1, &binded->vertex_buffer, &stride, &offset);
			ctx.context->IASetIndexBuffer(binded->index_buffer, DXGI_FORMAT_R32_UINT, 0);

			ctx.context->DrawIndexed(sub_mesh.indices.len(), 0, 0);
		}
	});
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}