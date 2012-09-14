#pragma once

#include "D3DUtil.h"

struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}

	D3DXVECTOR3 pos;
	D3DXVECTOR3 dir;
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR specular;
	D3DXVECTOR3 att;
	float spotPow;
};

struct Material
{
	Material()
	{
		ZeroMemory(this, sizeof(Material));
	}

	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR specular;
	float specPower;
};