#include "FirstPersonCamera.h"
#include "stdafx.h"
#include "FirstPersonCamera.h"
#include <iostream>

void FirstPersonCamera::UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	lookAt = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	lookAt = XMVector3Normalize(lookAt);
	pos += moveLeftRight * camRight;
	pos += moveBackForward * camForward;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	up = XMVector3TransformCoord(up, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	//pos += moveLeftRight * camRight;
	//pos += moveBackForward * camForward;
	pos += moveUp * camUp;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	moveUp = 0.0f;
	lookAt = pos + lookAt;

	cBufferCPU->viewMatrix = XMMatrixLookAtLH(pos, lookAt, up);
	cBufferCPU->projMatrix = projMatrix;
	XMStoreFloat4(&(cBufferCPU->eyePos), pos);

}