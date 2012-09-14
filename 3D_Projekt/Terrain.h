#pragma once 

#include "D3DUtil.h"
#include <string>
#include <vector>

struct InitInfo
{
	std::string HeightmapFilename;
	std::wstring LayerMapFilename0;
	std::wstring LayerMapFilename1;
	std::wstring LayerMapFilename2;
	std::wstring LayerMapFilename3;
	std::wstring LayerMapFilename4;
	std::wstring BlendMapFilename;
	float HeightScale;
	float HeightOffset;
	UINT NumRows;
	UINT NumCols;
	float CellSpacing;
};

class Terrain
{
public:
	Terrain();
	~Terrain();
	
	float width()const;
	float depth()const;
	float getHeight(float _x, float _z)const;

	void init(ID3D10Device* _device, const InitInfo& _initInfo);

	void setDirectionToSun(const D3DXVECTOR3& _sunDir);

	void draw(const D3DXMATRIX& _worldMatrix);

private:
	void loadHeightmap();
	void smooth();
	bool inBounds(UINT i, UINT j);
	float average(UINT i, UINT j);
	void buildVB();
	void buildIB();

	InitInfo info;

	UINT numVertices;
	UINT numFaces;

	std::vector<float> heightmap;

	ID3D10Device* localDevice;
	ID3D10Buffer* vb;
	ID3D10Buffer* ib;
	ID3DX10Mesh*				terrainMesh;

	ID3D10ShaderResourceView* layer0;
	ID3D10ShaderResourceView* layer1;
	ID3D10ShaderResourceView* layer2;
	ID3D10ShaderResourceView* layer3;
	ID3D10ShaderResourceView* layer4;
	ID3D10ShaderResourceView* blendMap;

	ID3D10EffectTechnique* tech;
	ID3D10EffectMatrixVariable* worldViewProjectionVar;
	ID3D10EffectMatrixVariable* worldVar;
	ID3D10EffectVectorVariable* dirToSunVar;
	ID3D10EffectShaderResourceVariable* layer0Var;
	ID3D10EffectShaderResourceVariable* layer1Var;
	ID3D10EffectShaderResourceVariable* layer2Var;
	ID3D10EffectShaderResourceVariable* layer3Var;
	ID3D10EffectShaderResourceVariable* layer4Var;
	ID3D10EffectShaderResourceVariable* blendMapVar;
};