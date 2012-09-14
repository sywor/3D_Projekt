#include "D3DUtil.h"
//
//typedef std::vector<D3DXVECTOR3> VertexList;
//typedef std::vector<DWORD> IndexList;

//void Subdevide(VertexList& _vertices, IndexList& _indices)
//{
//	VertexList vin = _vertices;
//	IndexList iin = _indices;
//
//	_vertices.resize(0);
//	_indices.resize(0);
//
//	UINT numTriangels = (UINT)iin.size() / 3;
//
//	for (UINT i = 0; i < numTriangels; i++)
//	{
//		D3DXVECTOR3 v0 = vin[ iin[i*3+0] ];
//		D3DXVECTOR3 v1 = vin[ iin[i*3+1] ];
//		D3DXVECTOR3 v2 = vin[ iin[i*3+2] ];
//
//		D3DXVECTOR3 m0 = 0.5f*(v0 + v1);
//		D3DXVECTOR3 m1 = 0.5f*(v1 + v2);
//		D3DXVECTOR3 m2 = 0.5f*(v0 + v2);
//
//		_vertices.push_back(v0); // 0
//		_vertices.push_back(v1); // 1
//		_vertices.push_back(v2); // 2
//		_vertices.push_back(m0); // 3
//		_vertices.push_back(m1); // 4
//		_vertices.push_back(m2); // 5
//
//		_indices.push_back(i*6+0);
//		_indices.push_back(i*6+3);
//		_indices.push_back(i*6+5);
//		
//		_indices.push_back(i*6+3);
//		_indices.push_back(i*6+4);
//		_indices.push_back(i*6+5);
//		
//		_indices.push_back(i*6+5);
//		_indices.push_back(i*6+4);
//		_indices.push_back(i*6+2);
//		
//		_indices.push_back(i*6+3);
//		_indices.push_back(i*6+1);
//		_indices.push_back(i*6+4);
//	}
//}

//void BuildGoeSphere(UINT _numSubDivisions, float _radius, std::vector<D3DXVECTOR3>& _vertices, std::vector<DWORD>& _indices)
//{
//	_numSubDivisions = Min(_numSubDivisions, UINT(5));
//
//	const float X = 0.525731f; 
//	const float Z = 0.850651f;
//
//	D3DXVECTOR3 pos[12] = 
//	{
//		D3DXVECTOR3(-X, 0.0f, Z),  D3DXVECTOR3(X, 0.0f, Z),  
//		D3DXVECTOR3(-X, 0.0f, -Z), D3DXVECTOR3(X, 0.0f, -Z),    
//		D3DXVECTOR3(0.0f, Z, X),   D3DXVECTOR3(0.0f, Z, -X), 
//		D3DXVECTOR3(0.0f, -Z, X),  D3DXVECTOR3(0.0f, -Z, -X),    
//		D3DXVECTOR3(Z, X, 0.0f),   D3DXVECTOR3(-Z, X, 0.0f), 
//		D3DXVECTOR3(Z, -X, 0.0f),  D3DXVECTOR3(-Z, -X, 0.0f)
//	};
//
//	DWORD k[60] = 
//	{
//		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,    
//		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,    
//		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0, 
//		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7 
//	};
//
//	_vertices.resize(12);
//	_indices.resize(60);
//
//	for (int i = 0; i < 12; i++)
//	{
//		_vertices[i] = pos[i];
//	}
//
//	for (int i = 0; i < 60; i++)
//	{
//		_indices[i] = k[i];
//	}
//
//	for (UINT i = 0; i < _numSubDivisions; i++)
//	{
//		Subdevide(_vertices, _indices);
//	}
//
//	for (UINT i = 0; i < _vertices.size(); i++)
//	{
//		D3DXVec3Normalize(&_vertices[i], &_vertices[i]);
//		_vertices[i] *= _radius;
//	}
//}