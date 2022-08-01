#include<renderer/util.h>


void DepthTexture::clean_up() {
	if (dsv) dsv->Release();
	if (texture) texture->Release();
	if (srv) srv->Release();
}

void DepthTexture::clear(ID3D11DeviceContext* ctx) {
	ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DepthTextures::clear(ID3D11DeviceContext* ctx) {
	for (ID3D11DepthStencilView* dsv : *this) {
		ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

ID3D11DepthStencilView** DepthTextures::begin() {
	return dsvs;
}

ID3D11DepthStencilView** DepthTextures::end() {
	return dsvs + size.z;
}

ID3D11DepthStencilView** DepthTextures::begin() const {
	return dsvs;
}
ID3D11DepthStencilView** DepthTextures::end() const {
	return dsvs + size.z;
}

void DepthTexture::resize(ID3D11Device* device, Vec2<u16> size) {
	clean_up();
	*this = create(device, size).unwrap();
}

Result<DepthTexture, RenderCreateError> DepthTexture::create(ID3D11Device* device, Vec2<u16> size) {
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = size.x;
	textureDesc.Height = size.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* texture;
	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &texture))) {
		return FailedTextureCreation;
	}
	ID3D11DepthStencilView* view;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Flags = 0;
	dsv_desc.Texture2D = { 0 };

	if (FAILED(device->CreateDepthStencilView(texture, &dsv_desc, &view))) {
		texture->Release();
		return FailedDSVCreation;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
	resource_desc.Texture2D = { 0, 1 };
	resource_desc.Format = DXGI_FORMAT_R32_FLOAT;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ID3D11ShaderResourceView* srv;
	if (FAILED(device->CreateShaderResourceView(texture, &resource_desc, &srv))) {
		texture->Release();
		view->Release();
		return FailedSRVCreation;
	}
	
	return ok<DepthTexture, RenderCreateError>(DepthTexture{
			view,
			texture,
			srv,
		});
}

void DepthTextures::clean_up() {
	for (usize i = 0; i < size.z; ++i) {
		dsvs[i]->Release();
		srvs[i]->Release();
	}
	delete[] dsvs;
	delete[] srvs;
	if (texture) texture->Release();
	if (srv) srv->Release();
}

void DepthTextures::resize(ID3D11Device* device, Vec3<u16> size) {
	clean_up();
	*this = create(device, size).unwrap();
}

Result<DepthTextures, RenderCreateError> DepthTextures::create(ID3D11Device* device, Vec3<u16> size) {
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = size.x;
	textureDesc.Height = size.y;
	textureDesc.ArraySize = size.z;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* texture;
	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &texture))) {
		return FailedTextureCreation;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsv_desc.Flags = 0;
	dsv_desc.Texture2DArray.MipSlice = 0;
	dsv_desc.Texture2DArray.ArraySize = 1;

	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	resource_desc.Format = DXGI_FORMAT_R32_FLOAT;
	resource_desc.Texture2DArray.ArraySize = 1;
	resource_desc.Texture2DArray.MipLevels = 1;
	resource_desc.Texture2DArray.MostDetailedMip = 0;

	ID3D11DepthStencilView** dsvs = new ID3D11DepthStencilView * [size.z];
	ID3D11ShaderResourceView** srvs = new ID3D11ShaderResourceView * [size.z];


	for (usize i = 0; i < size.z; ++i) {
		dsv_desc.Texture2DArray.FirstArraySlice = i;

		if (FAILED(device->CreateDepthStencilView(texture, &dsv_desc, dsvs + i))) {
			texture->Release();
			for (usize j = 0; j < i; ++j) {
				dsvs[j]->Release();
				srvs[j]->Release();
			}
			delete[] dsvs;
			delete[] srvs;
			return FailedDSVCreation;
		}

		resource_desc.Texture2DArray.FirstArraySlice = i;
		if (FAILED(device->CreateShaderResourceView(texture, &resource_desc, srvs + i))) {
			texture->Release();
			for (usize j = 0; j < i; ++j) {
				dsvs[j]->Release();
				srvs[j]->Release();
			}
			dsvs[i]->Release();
			delete[] dsvs;
			delete[] srvs;
			return FailedSRVCreation;
		}
	}

	resource_desc.Texture2DArray.FirstArraySlice = 0;
	resource_desc.Texture2DArray.ArraySize = size.z;
	ID3D11ShaderResourceView* srv;
	if (FAILED(device->CreateShaderResourceView(texture, &resource_desc, &srv))) {
		texture->Release();
		for (usize i = 0; i < size.z; ++i) {
			dsvs[i]->Release();
			srvs[i]->Release();
		}
		delete[] dsvs;
		delete[] srvs;
		return FailedSRVCreation;
	}

	return ok<DepthTextures, RenderCreateError>(DepthTextures{
			dsvs,
			srvs,
			texture,
			srv,
			size,
		});
}


void RenderTexture::clean_up() {
	if (rtv) rtv->Release();
	if (texture) texture->Release();
	if (srv) srv->Release();
	if (uav) uav->Release();
}

void RenderTexture::resize(ID3D11Device* device, Vec2<u16> size) {
	clean_up();
	*this = create(device, size, format).unwrap();
}

Result<RenderTexture, RenderCreateError> RenderTexture::create(ID3D11Device* device, Vec2<u16> size, DXGI_FORMAT format) {
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = size.x;
	textureDesc.Height = size.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* texture;
	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &texture))) {
		return FailedTextureCreation;
	}
	ID3D11RenderTargetView* rtv;

	if (FAILED(device->CreateRenderTargetView(texture, 0, &rtv))) {
		texture->Release();
		return FailedRTVCreation;
	}


	ID3D11UnorderedAccessView* uav;

	if (FAILED(device->CreateUnorderedAccessView(texture, 0, &uav))) {
		texture->Release();
		rtv->Release();
		return FailedUAVCreation;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc;
	resource_desc.Texture2D = { 0, 1 };
	resource_desc.Format = textureDesc.Format;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ID3D11ShaderResourceView* srv;
	if (FAILED(device->CreateShaderResourceView(texture, &resource_desc, &srv))) {
		texture->Release();
		rtv->Release();
		uav->Release();
		return FailedSRVCreation;
	}

	return ok<RenderTexture, RenderCreateError>(RenderTexture{
			rtv,
			texture,
			srv,
			uav,
			format,
		});
}

void RenderTarget::clean_up() {
	if (uav) uav->Release();
	if (texture) texture->Release();
	if (rtv) rtv->Release();
}

void RenderTarget::resize(ID3D11Device* device, IDXGISwapChain* swap_chain) {
	clean_up();
	*this = create(device, swap_chain).unwrap();
}

Result<RenderTarget, RenderCreateError> RenderTarget::create(ID3D11Device* device, IDXGISwapChain* swap_chain) {
	// get the address of the back buffer
	ID3D11Texture2D* back_buffer = nullptr;
	if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)))) {
		return FailedBackBuffer;
	}
	ID3D11UnorderedAccessView* uav = nullptr;
	ID3D11RenderTargetView* rtv;
	if (FAILED(device->CreateRenderTargetView(back_buffer, NULL, &rtv))) {
		std::cout << "FailedRTVCreation" << std::endl;
		return FailedRTVCreation;
	}

	if (FAILED(device->CreateUnorderedAccessView(back_buffer, NULL, &uav))) {
		std::cout << "screen FailedUAVCreation" << std::endl;
		return FailedUAVCreation;
	}

	if (uav == nullptr) {
		std::cout << "bruh" << std::endl;
	}

	return ok<RenderTarget, RenderCreateError>(RenderTarget{
			back_buffer,
			uav,
			rtv,
		});
}
