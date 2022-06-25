#include<renderer/renderer.h>
#include<renderer/rtv.h>
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

Option<ObjectRenderer::Locals> ObjectRenderer::Locals::from_object(const Object& obj) {
	return obj.transform.get_mat().try_invert().map<Locals>([&](Mat4<f32> mat) { 
		return Locals {
			mat,
			obj.color.with_w(1.0)
		};
	});
}

void Renderer::clean_up() {
	clean_up_ctx(ctx);
	clean_up_first_pass(first_pass);
}

void Renderer::resize(u32 width, u32 height)  {
	if (ctx.rtv) { ctx.rtv->Release();  }
	ctx.swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	ctx.viewport.Width = (f32)width;
	ctx.viewport.Height = (f32)height;
	ctx.rtv = create_render_target_view(ctx.device, ctx.swap_chain).unwrap();
}

void Renderer::begin_draw(const World& world, AssetHandler& assets) {
	assets.default_asset<Image>()->bind(ctx.device);
	assets.default_asset<Mesh>()->bind(ctx.device);
	// Do some setup and bind required resource before we draw.
	world.objects.values([&](const Object* obj) {
		assets.get(obj->mesh).unwrap()->bind(ctx.device);
		auto img = obj->image;
		img.map<char>([&](Id<Image> img) { assets.get(img).unwrap()->bind(ctx.device); return 0; });
	});
	

	ctx.context->OMSetRenderTargets(1, &ctx.rtv, ctx.ds_view);
	ctx.context->RSSetViewports(1, &ctx.viewport);
	ctx.context->ClearRenderTargetView(ctx.rtv, clear_color_with_alpha);
}

Globals Globals::from_world(const World& world, u32 width, u32 height) {
	return Globals{
		world.camera.get_proj(width, height),
		world.camera.get_view()
	};
}

void Renderer::draw_first_pass(const Window* window, const World& world, const AssetHandler& assets) {
	// First update the globals buffer
	auto globals = Globals::from_world(world, window->width(), window->height());
	first_pass.globals.update(ctx.context, &globals);

	ctx.context->VSSetShader(first_pass.object_renderer.vs, nullptr, 0);
	ctx.context->PSSetShader(first_pass.object_renderer.ps, nullptr, 0);

	ID3D11Buffer* uniforms[2] = { first_pass.globals.buffer, first_pass.object_renderer.locals.buffer };
	ctx.context->VSSetConstantBuffers(0, 2, uniforms);
	ctx.context->PSSetConstantBuffers(0, 2, uniforms);
	world.objects.values([&](const Object* obj) {
		auto maybe_locals = ObjectRenderer::Locals::from_object(*obj);
		if (maybe_locals.is_none()) {
			return;
		}
		auto locals = maybe_locals.unwrap_unchecked();
		first_pass.object_renderer.locals.update(ctx.context, &locals);

		Option<Id<Image>> maybe_image = obj->image;
		const Image* image = maybe_image
			.map<Option<const Image*>>([&](Id<Image> image) { return assets.get(image); })
			.flatten<const Image*>()
			.unwrap_or_else([&]() { return assets.default_asset<Image>(); });

		if (image->binded.is_none()) {
			return;
		}
		auto binded = image->binded.as_ptr().unwrap_unchecked();

		// ctx.context->PSSetShaderResources(0, 1, &binded->rsv);
		// ctx.context->PSSetSamplers(0, 1, &binded->sampler_state);

		const Mesh* mesh = assets.get(obj->mesh).unwrap();
		for (const SubMesh& sub_mesh : mesh->submeshes) {
			if (sub_mesh.binded.is_none()) {
				continue;
			}
			
			auto binded = sub_mesh.binded.as_ptr().unwrap_unchecked();
			u32 stride = sizeof(Vertex);
			u32 offset = 0;
			ctx.context->IASetVertexBuffers(0, 1, &binded->vertex_buffer, &stride, &offset);
			ctx.context->IASetIndexBuffer(binded->index_buffer, DXGI_FORMAT_R16_UINT, 0);
			ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			ctx.context->Draw(sub_mesh.vertices.len(), 0);
		}
	});
}

void Renderer::present() {
	ctx.swap_chain->Present(1, 0);
}