#include "Effects.h"

ID3D10Effect* fx::SkyFX		= 0;
ID3D10Effect* fx::TerrainFX = 0;
ID3D10Effect* fx::FireFX	= 0;

ID3D10Effect* CreateFX(ID3D10Device* _device, std::wstring _fileName)
{
	ID3D10Blob* compelationErrors = 0;
	HRESULT hr = 0;
	ID3D10Effect* fx = 0;

	hr = D3DX10CreateEffectFromFile(_fileName.c_str(), 0, 0, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, _device, 0, 0, &fx, &compelationErrors, 0);

	if (FAILED(hr) && compelationErrors)
	{
		if (compelationErrors)
		{
			MessageBoxA(0, (char*)compelationErrors->GetBufferPointer(), 0, 0);
			ReleaseCOM(compelationErrors);
		}

		DXTrace(__FILE__, (DWORD)__LINE__, hr, _fileName.c_str(), true);
	}

	return fx;
}

void fx::InitAll(ID3D10Device* device)
{
	SkyFX = CreateFX(device, L"sky.fx");
	TerrainFX = CreateFX(device, L"terrain.fx");
	FireFX = CreateFX(device, L"fire.fx");
}

void fx::DestroyAll()
{
	ReleaseCOM(SkyFX);
	ReleaseCOM(TerrainFX);
	ReleaseCOM(FireFX);
}