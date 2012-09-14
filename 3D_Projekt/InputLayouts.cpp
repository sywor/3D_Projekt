#include "InputLayouts.h"
#include "Effects.h"

ID3D10InputLayout* InputLayout::Pos				= 0;
ID3D10InputLayout* InputLayout::PosNormalTex	= 0;
ID3D10InputLayout* InputLayout::Particle		= 0;

void InputLayout::InitAll(ID3D10Device* _device)
{

	D3D10_PASS_DESC passDescSky;
	fx::SkyFX->GetTechniqueByName("SkyTech")->GetPassByIndex(0)->GetDesc(&passDescSky);
	HR(_device->CreateInputLayout(posVertexDesc, 1, passDescSky.pIAInputSignature, passDescSky.IAInputSignatureSize, &Pos));

	D3D10_PASS_DESC passDescTerrain;
	fx::TerrainFX->GetTechniqueByName("TerrainTech")->GetPassByIndex(0)->GetDesc(&passDescTerrain);
	HR(_device->CreateInputLayout(posNormalTexVertexDesc, 3, passDescTerrain.pIAInputSignature, passDescTerrain.IAInputSignatureSize, &PosNormalTex));

	D3D10_PASS_DESC passDescParticels;
	fx::FireFX->GetTechniqueByName("StreamOutTech")->GetPassByIndex(0)->GetDesc(&passDescParticels);
	HR(_device->CreateInputLayout(particleDesc, 5, passDescParticels.pIAInputSignature, passDescParticels.IAInputSignatureSize, &Particle));
}

void InputLayout::DestroyAll()
{
	ReleaseCOM(Pos);
	ReleaseCOM(PosNormalTex);
	ReleaseCOM(Particle);
}