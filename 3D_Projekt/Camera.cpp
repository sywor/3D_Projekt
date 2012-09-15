#include "Camera.h"
#include "Terrain.h"


Camera& GetCamera()
{
	static Camera camera;
	return camera;
}

Camera::Camera()
{
	
	pos		= D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	right   = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	up      = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	look    = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	D3DXMatrixIdentity(&viewMatrix);
	D3DXMatrixIdentity(&projmatrix);

	recFrameCount = 0;
	RecordBool = false;
}

Camera::~Camera(){}

D3DXVECTOR3& Camera::Position()
{
	return pos;
}

D3DXVECTOR3& Camera::Look()
{
	return look;
}

D3DXMATRIX Camera::ViewMatrix()const
{
	return viewMatrix;
}

D3DXMATRIX Camera::ProjMatrix()const
{
	return projmatrix;
}

void Camera::SetLens(float _fovY, float _aspect, float _zn, float _zf)
{
	D3DXMatrixPerspectiveFovLH(&projmatrix, _fovY, _aspect, _zn, _zf);
}

void Camera::Strafe(float _d)
{
	pos += _d * right;
}

void Camera::Walk(float _d)
{
	pos += _d * look;
}

void Camera::Pitch(float _angle)
{
	D3DXMATRIX p;
	D3DXMatrixRotationAxis(&p, &right, _angle);

	D3DXVec3TransformNormal(&up, &up, &p);
	D3DXVec3TransformNormal(&look, &look, &p);
}

void Camera::RotateY(float _angle)
{
	D3DXMATRIX r;
	D3DXMatrixRotationY(&r, _angle);

	D3DXVec3TransformNormal(&right, &right, &r);
	D3DXVec3TransformNormal(&up, &up, &r);
	D3DXVec3TransformNormal(&look, &look, &r);
}

void Camera::RebuildView()
{
	D3DXVec3Normalize(&look, &look);
	D3DXVec3Cross(&up, &look, &right);

	D3DXVec3Normalize(&up, &up);
	D3DXVec3Cross(&right, &up, &look);

	D3DXVec3Normalize(&right, &right);

	float x = D3DXVec3Dot(&pos, &right);
	float y = D3DXVec3Dot(&pos, &up);
	float z = D3DXVec3Dot(&pos, &look);

	viewMatrix(0,0) = right.x;
	viewMatrix(1,0) = right.y;
	viewMatrix(2,0) = right.z;
	viewMatrix(3,0) = x;

	viewMatrix(0,1) = up.x;
	viewMatrix(1,1) = up.y;
	viewMatrix(2,1) = up.z;
	viewMatrix(3,1) = y;

	viewMatrix(0,2) = look.x;
	viewMatrix(1,2) = look.y;
	viewMatrix(2,2) = look.z;
	viewMatrix(3,2) = z;

	viewMatrix(0,3) = 0.0f;
	viewMatrix(1,3) = 0.0f;
	viewMatrix(2,3) = 0.0f;
	viewMatrix(3,3) = 1.0f;
}

void Camera::Record(float _dt)
{
	recFPS += _dt; 

	if (recFPS >= 0.01f)
	{
		camPath cp(pos, look);
		path.push_back(cp);
		recFPS = 0.0f;
		recFrameCount++;
		return;
	} 
}

void Camera::SaveRecording()
{
	file.open("path1.campath", std::ios::out | std::ios::binary);

	for (size_t i = 0; i < path.size(); i++)
	{
		file.write(reinterpret_cast<char*>(&path[i]), sizeof(path[0]));
	}

	file.close();
	recFrameCount = 0;
	return;
}