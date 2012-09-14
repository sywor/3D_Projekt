#pragma once

#include "D3DUtil.h"

typedef std::vector<D3DXVECTOR3> VertexList;
typedef std::vector<DWORD> IndexList;

struct SkyVertex 
{
	D3DXVECTOR3 pos;
};

class Sky
{
public:
	Sky();
	~Sky();
	void init(ID3D10Device* _device, ID3D10ShaderResourceView* _cubemap, float _radius);
	void draw();

private:

	ID3D10Device* localDevice;
	ID3D10Buffer* vb;
	ID3D10Buffer* ib;

	ID3D10ShaderResourceView* cubeMap;

	UINT numIndices;

	ID3D10EffectTechnique* tech;
	ID3D10EffectMatrixVariable* worldViewProjectionVar;
	ID3D10EffectShaderResourceVariable* cubeMapVar;

	void BuildGoeSphere(UINT _numSubDivisions, float _radius, std::vector<D3DXVECTOR3>& _vertices, std::vector<DWORD>& _indices);
	void Subdevide(VertexList& _vertices, IndexList& _indices);
};

D3DX10INLINE UINT Min(const UINT& _a, const UINT& _b)
{
	if (_a < _b)
	{
		return _a;
	} 
	else
	{
		return _b;
	}
}