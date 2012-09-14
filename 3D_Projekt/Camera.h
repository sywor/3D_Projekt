#pragma once

#include "d3dUtil.h"

class Camera
{
public:
	Camera();
	~Camera();

	D3DXVECTOR3& Position();
	D3DXMATRIX& GetPitch();
	D3DXMATRIX& getAngle();

	D3DXMATRIX ViewMatrix()const;
	D3DXMATRIX ProjMatrix()const;

	void SetLens(float _fovY, float _aspect, float _zn, float _zf);

	void Strafe(float _d);
	void Walk(float _d);

	void Pitch(float _angle);
	void RotateY(float _angle);

	void RebuildView();

private:
	D3DXVECTOR3 pos;
	D3DXVECTOR3 right;
	D3DXVECTOR3 up;
	D3DXVECTOR3 look;

	D3DXMATRIX r, p;

	D3DXMATRIX viewMatrix;
	D3DXMATRIX projmatrix;
};

Camera& GetCamera();