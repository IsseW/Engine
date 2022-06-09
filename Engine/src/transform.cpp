#include "Transform.h"

DirectX::XMFLOAT4X4 Transform::get_matrix() {
	DirectX::XMFLOAT3 rotation_origin_vector = {
		0.0f,
		0.0f,
		0.0f,
	};
	auto rotation_origin = DirectX::XMLoadFloat3(&rotation_origin_vector);

	auto scaling = DirectX::XMLoadFloat3(&scale);

	auto rotation = DirectX::XMLoadFloat4(&this->rotation);
	auto translation = DirectX::XMLoadFloat3(&this->translation);
	auto matrix = DirectX::XMMatrixAffineTransformation(scaling, rotation_origin, rotation, translation);

	DirectX::XMFLOAT4X4 result;
	DirectX::XMStoreFloat4x4(&result, matrix);
	return result;
}