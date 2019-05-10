#pragma once

#include <DirectXMath.h>
#include <Camera.h>

class FirstPersonCamera : public Camera
{

private:
	XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX camRotationMatrix;
	XMMATRIX groundWorld;
	float height = 1.0f;
	float camYaw = 0.0f;
	float camPitch = 0.0f;
public:
	void UpdateCamera();
	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float moveUp = 0.0f;
	void updateRotation(float yaw, float pitch) { camYaw += yaw; camPitch += pitch; }
	FirstPersonCamera(ID3D11Device* device) : Camera(device) {};
	FirstPersonCamera(ID3D11Device* device, DirectX::XMVECTOR init_pos, DirectX::XMVECTOR init_up, DirectX::XMVECTOR init_lookAt) :Camera(device, init_pos, init_up, init_lookAt) {};
};

#pragma once
