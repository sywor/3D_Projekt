#pragma once

#include "D3DUtil.h"

namespace fx
{
	extern ID3D10Effect* SkyFX;
	extern ID3D10Effect* TerrainFX;
	extern ID3D10Effect* MeshFX;
	extern ID3D10Effect* FireFX;

	void InitAll(ID3D10Device* device);
	void DestroyAll();
}