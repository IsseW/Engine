#pragma once
#include<renderer/renderer.h>

template<typename F>
void draw_objects(const RendererCtx& ctx, const World& world, const AssetHandler& assets, F bind_object) {
	ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	world.objects.values([&](const Object& obj) {
		if (bind_object(obj)) {
			return;
		}

		const Mesh* mesh = assets.get_or_default(obj.mesh);
		for (const SubMesh& sub_mesh : mesh->submeshes) {
			if (sub_mesh.binded.is_none()) {
				continue;
			}

			auto binded = sub_mesh.binded.as_ptr().unwrap_unchecked();
			u32 stride = sizeof(Vertex);
			u32 offset = 0;
			ctx.context->IASetVertexBuffers(0, 1, &binded->vertex_buffer, &stride, &offset);
			ctx.context->IASetIndexBuffer(binded->index_buffer, DXGI_FORMAT_R16_UINT, 0);

			ctx.context->DrawIndexed(sub_mesh.indices.len(), 0, 0);
		}
		});
}