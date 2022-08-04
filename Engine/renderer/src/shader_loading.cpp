#include<renderer/util.h>
#include<fstream>

Option<std::string> load_file_text(const char* file) {
	std::string data;
	std::ifstream reader;
	reader.open(file, std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return none<std::string>();
	}

	reader.seekg(0, std::ios::end);
	data.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	data.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());
	reader.close();

	return some(data);
}

Result<ID3D11PixelShader*, RenderCreateError> load_pixel(ID3D11Device* device, const char* file) {
	auto maybe_data = load_file_text(file);
	if (maybe_data.is_none()) {
		return MissingShaderFile;
	}
	auto data = maybe_data.unwrap_unchecked();
	ID3D11PixelShader* shader;
	if (FAILED(device->CreatePixelShader(data.c_str(), data.length(), nullptr, &shader)))
	{
		return FailedShaderCreation;
	}
	return ok<ID3D11PixelShader*, RenderCreateError>(shader);
}

Result<VSIL, RenderCreateError> load_vertex(ID3D11Device* device, const char* file, const std::vector<D3D11_INPUT_ELEMENT_DESC>& input) {
	auto maybe_data = load_file_text(file);
	if (maybe_data.is_none()) {
		return MissingShaderFile;
	}
	auto data = maybe_data.unwrap_unchecked();
	ID3D11VertexShader* shader;
	auto hr = device->CreateVertexShader(data.c_str(), data.length(), nullptr, &shader);
	if (FAILED(hr)) {
		return FailedShaderCreation;
	}
	ID3D11InputLayout* input_layout;
	if (FAILED(device->CreateInputLayout(input.data(), input.size(), data.c_str(), data.length(), &input_layout))) {
		return FailedLayoutCreation;
	}

	return ok<VSIL, RenderCreateError>(VSIL{
			shader,
			input_layout,
		});
}

Result<ID3D11ComputeShader*, RenderCreateError> load_compute(ID3D11Device* device, const char* file) {
	auto maybe_data = load_file_text(file);
	if (maybe_data.is_none()) {
		return MissingShaderFile;
	}

	std::string data = maybe_data.unwrap_unchecked();

	ID3D11ComputeShader* shader;
	if (FAILED(device->CreateComputeShader(data.c_str(), data.length(), nullptr, &shader))) {
		return FailedShaderCreation;
	}
	return ok<ID3D11ComputeShader*, RenderCreateError>(shader);
}

Result<ID3D11GeometryShader*, RenderCreateError> load_geometry(ID3D11Device* device, const char* file) {
	auto maybe_data = load_file_text(file);
	if (maybe_data.is_none()) {
		return MissingShaderFile;
	}

	std::string data = maybe_data.unwrap_unchecked();
	ID3D11GeometryShader* shader;

	if (FAILED(device->CreateGeometryShader(data.c_str(), data.length(), nullptr, &shader))) {
		return FailedShaderCreation;
	}
	return ok<ID3D11GeometryShader*, RenderCreateError>(shader);
}

Result<ID3D11HullShader*, RenderCreateError> load_hull(ID3D11Device* device, const char* file) {
	auto maybe_data = load_file_text(file);
	if (maybe_data.is_none()) {
		return MissingShaderFile;
	}

	std::string data = maybe_data.unwrap_unchecked();
	ID3D11HullShader* shader;

	if (FAILED(device->CreateHullShader(data.c_str(), data.length(), nullptr, &shader))) {
		return FailedShaderCreation;
	}
	return ok<ID3D11HullShader*, RenderCreateError>(shader);
}

Result<ID3D11DomainShader*, RenderCreateError> load_domain(ID3D11Device* device, const char* file) {
	auto maybe_data = load_file_text(file);
	if (maybe_data.is_none()) {
		return MissingShaderFile;
	}

	std::string data = maybe_data.unwrap_unchecked();
	ID3D11DomainShader* shader;

	if (FAILED(device->CreateDomainShader(data.c_str(), data.length(), nullptr, &shader))) {
		return FailedShaderCreation;
	}
	return ok<ID3D11DomainShader*, RenderCreateError>(shader);
}