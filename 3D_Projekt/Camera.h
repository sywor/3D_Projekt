#pragma once

#include "d3dUtil.h"
#include <fstream>

struct camPath
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 look;

	camPath(D3DXVECTOR3 _p, D3DXVECTOR3 _l)
	{
		pos = _p;
		look = _l;
	}
};

class Camera
{
public:
	Camera();
	~Camera();

	D3DXVECTOR3& Position();
	D3DXVECTOR3& Look();

	D3DXMATRIX ViewMatrix()const;
	D3DXMATRIX ProjMatrix()const;

	void SetLens(float _fovY, float _aspect, float _zn, float _zf);

	void Strafe(float _d);
	void Walk(float _d);

	void Pitch(float _angle);
	void RotateY(float _angle);

	void RebuildView();
	void CheckRec();

	bool Record, Recorded;

private:

	D3DXVECTOR3 pos;
	D3DXVECTOR3 right;
	D3DXVECTOR3 up;
	D3DXVECTOR3 look;

	D3DXMATRIX viewMatrix;
	D3DXMATRIX projmatrix;

	std::ofstream file;

	std::vector<camPath> path;
};

Camera& GetCamera();