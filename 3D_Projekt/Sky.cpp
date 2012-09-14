#include "Sky.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Effects.h"
#include "InputLayouts.h"

Sky::Sky()
{
	localDevice		= 0;
	vb				= 0;
	ib				= 0;
	cubeMap			= 0;
	numIndices		= 0;
}

Sky::~Sky()
{
	ReleaseCOM(vb);
	ReleaseCOM(ib);
}

void Sky::init(ID3D10Device* _device, ID3D10ShaderResourceView* _cubemap, float _radius)
{
	localDevice = _device;
	cubeMap = _cubemap;
	
	tech = fx::SkyFX->GetTechniqueByName("SkyTech");
	worldViewProjectionVar = fx::SkyFX->GetVariableByName("WVP")->AsMatrix();
	cubeMapVar = fx::SkyFX->GetVariableByName("cubeMap")->AsShaderResource();

	std::vector<D3DXVECTOR3> vertices;
	std::vector<DWORD> indices;

	BuildGoeSphere(2, _radius, vertices, indices);

	std::vector<SkyVertex> skyVert(vertices.size());

	for (UINT i = 0; i < vertices.size(); i++)
	{
		skyVert[i].pos = 0.5f * vertices[i];
	}

	D3D10_BUFFER_DESC vbDesc;
	vbDesc.Usage = D3D10_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(SkyVertex) * (UINT)skyVert.size();
	vbDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA vbData;
	vbData.pSysMem = &skyVert[0];

	HR(localDevice->CreateBuffer(&vbDesc, &vbData, &vb));

	numIndices = (UINT)indices.size();

	D3D10_BUFFER_DESC ibDesc;
	ibDesc.Usage = D3D10_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(DWORD) * numIndices;
	ibDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA ibData;
	ibData.pSysMem = &indices[0];

	HR(localDevice->CreateBuffer(&ibDesc, &ibData, &ib));
}

void Sky::BuildGoeSphere(UINT _numSubDivisions, float _radius, std::vector<D3DXVECTOR3>& _vertices, std::vector<DWORD>& _indices)
{
	_numSubDivisions = Min(_numSubDivisions, UINT(5));

	const float X = 0.525731f; 
	const float Z = 0.850651f;

	D3DXVECTOR3 pos[12] = 
	{
		D3DXVECTOR3(-X, 0.0f, Z),  D3DXVECTOR3(X, 0.0f, Z),  
		D3DXVECTOR3(-X, 0.0f, -Z), D3DXVECTOR3(X, 0.0f, -Z),    
		D3DXVECTOR3(0.0f, Z, X),   D3DXVECTOR3(0.0f, Z, -X), 
		D3DXVECTOR3(0.0f, -Z, X),  D3DXVECTOR3(0.0f, -Z, -X),    
		D3DXVECTOR3(Z, X, 0.0f),   D3DXVECTOR3(-Z, X, 0.0f), 
		D3DXVECTOR3(Z, -X, 0.0f),  D3DXVECTOR3(-Z, -X, 0.0f)
	};

	DWORD k[60] = 
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,    
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,    
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7 
	};

	_vertices.resize(12);
	_indices.resize(60);

	for (int i = 0; i < 12; i++)
	{
		_vertices[i] = pos[i];
	}

	for (int i = 0; i < 60; i++)
	{
		_indices[i] = k[i];
	}

	for (UINT i = 0; i < _numSubDivisions; i++)
	{
		Subdevide(_vertices, _indices);
	}

	for (UINT i = 0; i < _vertices.size(); i++)
	{
		D3DXVec3Normalize(&_vertices[i], &_vertices[i]);
		_vertices[i] *= _radius;
	}
}

void Sky::Subdevide(VertexList& _vertices, IndexList& _indices)
{
	VertexList vin = _vertices;
	IndexList iin = _indices;

	_vertices.resize(0);
	_indices.resize(0);

	UINT numTriangels = (UINT)iin.size() / 3;

	for (UINT i = 0; i < numTriangels; i++)
	{
		D3DXVECTOR3 v0 = vin[ iin[i*3+0] ];
		D3DXVECTOR3 v1 = vin[ iin[i*3+1] ];
		D3DXVECTOR3 v2 = vin[ iin[i*3+2] ];

		D3DXVECTOR3 m0 = 0.5f*(v0 + v1);
		D3DXVECTOR3 m1 = 0.5f*(v1 + v2);
		D3DXVECTOR3 m2 = 0.5f*(v0 + v2);

		_vertices.push_back(v0); // 0
		_vertices.push_back(v1); // 1
		_vertices.push_back(v2); // 2
		_vertices.push_back(m0); // 3
		_vertices.push_back(m1); // 4
		_vertices.push_back(m2); // 5

		_indices.push_back(i*6+0);
		_indices.push_back(i*6+3);
		_indices.push_back(i*6+5);

		_indices.push_back(i*6+3);
		_indices.push_back(i*6+4);
		_indices.push_back(i*6+5);

		_indices.push_back(i*6+5);
		_indices.push_back(i*6+4);
		_indices.push_back(i*6+2);

		_indices.push_back(i*6+3);
		_indices.push_back(i*6+1);
		_indices.push_back(i*6+4);
	}
}

void Sky::draw()
{
	D3DXVECTOR3 eyePos = GetCamera().Position();

	D3DXMATRIX W;
	D3DXMatrixTranslation(&W, eyePos.x, eyePos.y, eyePos.z);

	D3DXMATRIX V = GetCamera().ViewMatrix();
	D3DXMATRIX P = GetCamera().ProjMatrix();

	D3DXMATRIX WVP = W * V * P;

	HR(worldViewProjectionVar->SetMatrix((float*)WVP));
	HR(cubeMapVar->SetResource(cubeMap));

	UINT strid = sizeof(SkyVertex);
	UINT offset = 0;

	localDevice->IASetVertexBuffers(0, 1, &vb, &strid, &offset);
	localDevice->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	localDevice->IASetInputLayout(InputLayout::Pos);
	localDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D10_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		ID3D10EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0);
		localDevice->DrawIndexed(numIndices, 0, 0);
	}
}