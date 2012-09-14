#include "Terrain.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Effects.h"
#include "InputLayouts.h"
#include "FreeImage.h"
#include <fstream>
#include <sstream>

namespace
{
	struct TerrainVertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texCoord;
	};
}

Terrain::Terrain()
{
	localDevice		= 0;
	vb				= 0;
	ib				= 0;
	layer0			= 0;
	layer1			= 0;
	layer3			= 0;
	layer4			= 0;
	blendMap		= 0;
}

Terrain::~Terrain()
{
	ReleaseCOM(vb);
	ReleaseCOM(ib);
}

float Terrain::width()const
{
	return (info.NumCols - 1) * info.CellSpacing;
}

float Terrain::depth()const
{
	return (info.NumRows - 1) * info.CellSpacing;
}

float Terrain::getHeight(float _x, float _z)const
{
	float c = (_x + 0.5f * width()) / info.CellSpacing;
	float d = (_z - 0.5f * depth()) / info.CellSpacing;

	int row = (int)floorf(d);
	int col	= (int)floorf(c);

	float A = heightmap[row * info.NumCols + col];
	float B = heightmap[row * info.NumCols + col + 1];
	float C = heightmap[(row + 1) * info.NumCols + col];
	float D = heightmap[(row + 1) * info.NumCols + col + 1];

	float s = c - (float)col;
	float t = d - (float)row;

	if (s + t <= 1.0f) //if upper triangle
	{
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	}
	else //lower triangle
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s) * uy + (1.0f - t) * vy;
	}
}

void Terrain::init(ID3D10Device* _device, const InitInfo& _initInfo)
{
	localDevice = _device;

	tech					= fx::TerrainFX->GetTechniqueByName("TerrainTech");
	worldViewProjectionVar	= fx::TerrainFX->GetVariableByName("WVP")->AsMatrix();
	worldVar				= fx::TerrainFX->GetVariableByName("world")->AsMatrix();
	dirToSunVar				= fx::TerrainFX->GetVariableByName("dirToSun")->AsVector();
	layer0Var				= fx::TerrainFX->GetVariableByName("layer0")->AsShaderResource();
	layer1Var				= fx::TerrainFX->GetVariableByName("layer1")->AsShaderResource();
	layer2Var				= fx::TerrainFX->GetVariableByName("layer2")->AsShaderResource();
	layer3Var				= fx::TerrainFX->GetVariableByName("layer3")->AsShaderResource();
	layer4Var				= fx::TerrainFX->GetVariableByName("layer4")->AsShaderResource();
	blendMapVar				= fx::TerrainFX->GetVariableByName("blendMap")->AsShaderResource();

	info = _initInfo;
	numVertices = info.NumRows * info.NumCols;
	numFaces = (info.NumRows - 1) * (info.NumCols - 1) * 2;

	loadHeightmap();
	smooth();
	buildVB();
	buildIB();

	layer0 = GetTextureManager().createTex(info.LayerMapFilename0);
	layer1 = GetTextureManager().createTex(info.LayerMapFilename1);
	layer2 = GetTextureManager().createTex(info.LayerMapFilename2);
	layer3 = GetTextureManager().createTex(info.LayerMapFilename3);
	layer4 = GetTextureManager().createTex(info.LayerMapFilename4);
	blendMap = GetTextureManager().createTex(info.BlendMapFilename);
}

void Terrain::setDirectionToSun(const D3DXVECTOR3& _sunDir)
{
	D3DXVECTOR4 tmp(_sunDir.x, _sunDir.y, _sunDir.z, 0.0f);
	dirToSunVar->SetFloatVector((float*)tmp);
}

void Terrain::draw(const D3DXMATRIX& _worldMatrix)
{
	localDevice->IASetInputLayout(InputLayout::PosNormalTex);

	UINT strid	= sizeof(TerrainVertex);
	UINT offset = 0;
	localDevice->IASetVertexBuffers(0, 1, &vb, &strid, &offset);
	localDevice->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	D3DXMATRIX view = GetCamera().ViewMatrix();
	D3DXMATRIX proj = GetCamera().ProjMatrix();

	D3DXMATRIX WVP = _worldMatrix * view * proj;

	worldViewProjectionVar->SetMatrix((float*)&WVP);
	worldVar->SetMatrix((float*)&_worldMatrix);

	layer0Var->SetResource(layer0);
	layer1Var->SetResource(layer1);
	layer2Var->SetResource(layer2);
	layer3Var->SetResource(layer3);
	layer4Var->SetResource(layer4);
	blendMapVar->SetResource(blendMap);

	D3D10_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		ID3D10EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0);

		localDevice->DrawIndexed(numFaces * 3, 0, 0);
	}
}

void Terrain::loadHeightmap()
{
	std::vector<unsigned char> in(info.NumRows * info.NumCols);

	std::ifstream inFile;
	inFile.open(info.HeightmapFilename.c_str(), std::ios_base::binary);

	if (inFile)
	{
		inFile.read((char*)&in[0], (std::streamsize)in.size());
		inFile.close();
	}

	heightmap.resize(info.NumRows * info.NumCols, 0);

	for (UINT i = 0; i < info.NumRows * info.NumCols; i++)
	{
		heightmap[i] = (float)in[i] * info.HeightScale + info.HeightOffset;
	}
}

//void Terrain::loadHeightmap()
//{
//	const char* str = info.HeightmapFilename.c_str();
//	FIBITMAP *heightMapImg = FreeImage_Load(FIF_RAW, str, RAW_DEFAULT);
//
//	int hmHeight = 0;
//	int hmWidth = 0;
//	float* heightData = 0;
//
//	if ( heightMapImg )
//	{
//		//Hämtar höjd och bred
//		hmHeight = FreeImage_GetHeight(heightMapImg);
//		hmWidth = FreeImage_GetWidth(heightMapImg);
//
//		heightData = new float[hmHeight * hmWidth];
//
//		RGBQUAD color;	
//		for (int y=0; y < hmHeight; y++)
//		{
//			for(int x=0; x < hmWidth; x++)
//			{
//				FreeImage_GetPixelColor(heightMapImg,x,y,&color);
//
//				float divisor = 255 / 6;
//				heightData[y*hmWidth + x] = color.rgbRed / divisor - 3;
//			}
//		}
//
//		FreeImage_Unload(heightMapImg);
//	}
//
//	UINT totalNum = hmHeight * hmWidth;
//	heightmap.resize(totalNum, 0);
//
//	for (UINT i = 0; i < totalNum; i++)
//	{
//		heightmap[i] = (float)heightData[i] * info.HeightScale + info.HeightOffset;
//	}
//}

	//size_t numVertices = hmHeight * hmWidth;
	//std::vector<TerrainVertex> meshVertices(numVertices);
	//float tC[2] = {0,1};
	//float tCStepSize[2] = { 1.0f/hmWidth, 1.0f/hmHeight};

	//float z = -32;
	//float x = -32;
	//int cnt = 0;
	//int wCnt = 0;
	//int hCnt = 0;	

	////Skapar vertiser
	//while (hCnt < hmHeight)
	//{
	//	while (wCnt < hmWidth)
	//	{
	//		meshVertices[cnt].texCoord = D3DXVECTOR2( tC[0], tC[1] );			
	//		meshVertices[cnt++].pos = D3DXVECTOR3( x, heightData[cnt], z );
	//		tC[0] += tCStepSize[0];
	//		wCnt++;
	//		x += 64.0f/(hmWidth-1);
	//	}

	//	x = -32;
	//	z += 64.0f/(hmWidth-1);
	//	tC[0] = 0;
	//	tC[1] -= tCStepSize[1];
	//	wCnt = 0;
	//	hCnt++;
	//}

	////Beräkna normaler
	//for ( size_t i = 0; i < numVertices - hmWidth; i++ )
	//{
	//	D3DXVECTOR3 N, v1, v2;	
	//	v1 = meshVertices[i+hmWidth].pos - meshVertices[i].pos;
	//	v2 = meshVertices[i+1].pos - meshVertices[i].pos;

	//	D3DXVec3Cross( &N, &v1, &v2);

	//	meshVertices[i].normal += N;
	//	meshVertices[i+1].normal += N;
	//	meshVertices[i+hmWidth].normal += N;

	//	v1 = meshVertices[i+hmWidth].pos - meshVertices[i+1].pos;
	//	v2 = meshVertices[i+hmWidth+1].pos - meshVertices[i+1].pos;

	//	D3DXVec3Cross( &N, &v1, &v2);

	//	meshVertices[i+1].normal += N;
	//	meshVertices[i+hmWidth].normal += N;
	//	meshVertices[i+hmWidth+1].normal += N;

	//	if ( i % hmWidth == hmWidth - 2 ) i++;
	//}

	//for ( size_t i = 0; i < numVertices - hmWidth; i++ )
	//{
	//	D3DXVec3Normalize(&meshVertices[i].normal,&meshVertices[i].normal); 
	//}

	////beräknar indexes
	//UINT numIndices = (hmHeight-1) * (hmWidth-1) * 6;
	//std::vector<UINT> meshIndices(numIndices);
	//cnt = 0;

	//for ( size_t i = 0; i < numVertices - hmWidth; i++ )
	//{
	//	meshIndices[cnt++] = i;
	//	meshIndices[cnt++] = i + hmWidth;
	//	meshIndices[cnt++] = i + 1;
	//	meshIndices[cnt++] = i + 1;
	//	meshIndices[cnt++] = i + hmWidth;
	//	meshIndices[cnt++] = i + hmWidth + 1;

	//	if ( i % hmWidth == hmWidth - 2 ) i++;
	//}

	//int numElements = sizeof(InputLayout::posNormalTexVertexDesc)/sizeof(InputLayout::posNormalTexVertexDesc[0]);

	////Skapar mesh
	//D3DX10CreateMesh(localDevice, InputLayout::posNormalTexVertexDesc, numElements, "POSITION", numVertices, numIndices/3, D3DX10_MESH_32_BIT, &terrainMesh);


	//Skickar in data i meshen och skickar meshen till grafikkortet

	//D3D10_BUFFER_DESC vertexBufferDesc;
	//vertexBufferDesc.Usage			= D3D10_USAGE_IMMUTABLE;
	//vertexBufferDesc.ByteWidth		= sizeof(TerrainVertex) * numVertices;
	//vertexBufferDesc.BindFlags		= D3D10_BIND_VERTEX_BUFFER;
	//vertexBufferDesc.CPUAccessFlags = 0;
	//vertexBufferDesc.MiscFlags		= 0;

	//D3D10_SUBRESOURCE_DATA initData;
	//initData.pSysMem = &meshVertices[0];
	//HR(localDevice->CreateBuffer(&vertexBufferDesc, &initData, &vb));

	//D3D10_BUFFER_DESC indexBufferDesc;
	//indexBufferDesc.Usage			= D3D10_USAGE_IMMUTABLE;
	//indexBufferDesc.ByteWidth		= sizeof(DWORD) * numFaces * 3;
	//indexBufferDesc.BindFlags		= D3D10_BIND_INDEX_BUFFER;
	//indexBufferDesc.CPUAccessFlags	= 0;
	//indexBufferDesc.MiscFlags		= 0;

	//initData.pSysMem = &meshIndices[0];
	//HR(localDevice->CreateBuffer(&indexBufferDesc, &initData, &ib));

	//terrainMesh->SetVertexData(0, meshVertices.data());
	//terrainMesh->SetIndexData(meshIndices.data(), numIndices);
	//terrainMesh->CommitToDevice();

/*	delete meshVertices;
	delete meshIndices;*/
//}

void Terrain::smooth()
{
	std::vector<float> dest(heightmap.size());

	for (UINT i = 0; i < info.NumRows; i++)
	{
		for (UINT j = 0; j < info.NumCols; j++)
		{
			dest[i * info.NumCols + j] = average(i, j);
		}
	}

	heightmap = dest;
}

float Terrain::average(UINT i, UINT j)
{
	float avg = 0.0f;
	float num = 0.0f;

	for (UINT m = i - 1; m <= i + 1; m++)
	{
		for (UINT n = j - 1; n <= j + 1; n++)
		{
			if (inBounds(m, n))
			{
				avg += heightmap[m * info.NumCols + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

bool Terrain::inBounds(UINT i, UINT j)
{
	if (i >= 0 && i < info.NumRows && j >= 0 && j < info.NumCols)
	{
		return true;
	} 
	else
	{
		return false;
	}
}

void Terrain::buildVB()
{
	std::vector<TerrainVertex> vertices(numVertices);

	float halfWidth = (info.NumCols - 1) * info.CellSpacing * 0.5f;
	float halfDepth = (info.NumRows - 1) * info.CellSpacing * 0.5f;

	float du = 1.0f / (info.NumCols - 1);
	float dv = 1.0f / (info.NumRows - 1);

	for (UINT i = 0; i < info.NumRows; i++)
	{
		float z = halfDepth - i * info.CellSpacing;

		for (UINT j = 0; j < info.NumCols; j++)
		{
			float x = -halfWidth + j * info.CellSpacing;
			float y = heightmap[i * info.NumCols + j];

			vertices[i * info.NumCols + j].pos = D3DXVECTOR3(x, y, z);
			vertices[i * info.NumCols + j].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			vertices[i * info.NumCols + j].texCoord.x = j * du;
			vertices[i * info.NumCols + j].texCoord.y = i * dv;
		}
	}


	float invTwoDX = 1.0f / (2.0f * info.CellSpacing);

	for (UINT i = 2; i < info.NumRows - 1; i++)
	{
		for (UINT j = 2; j < info.NumCols - 1; j++)
		{
			float A = heightmap[(i - 1) * info.NumCols + j];
			float B = heightmap[(i + 1) * info.NumCols + j];
			float C = heightmap[i * info.NumCols + j - 1];
			float D = heightmap[i * info.NumCols + j + 1];

			D3DXVECTOR3 tanZ(0.0f, (A - B) * invTwoDX, 1.0f);
			D3DXVECTOR3 tanX(1.0f, (D - C) * invTwoDX, 0.0f);
			D3DXVECTOR3 N;

			D3DXVec3Cross(&N, &tanZ, &tanX);
			D3DXVec3Normalize(&N,&N);

			vertices[i * info.NumCols + j].normal = N;
		}
	}

	D3D10_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage			= D3D10_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth		= sizeof(TerrainVertex) * numVertices;
	vertexBufferDesc.BindFlags		= D3D10_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags		= 0;
	
	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = &vertices[0];
	HR(localDevice->CreateBuffer(&vertexBufferDesc, &initData, &vb));
}

void Terrain::buildIB()
{
	std::vector<DWORD> indices(numFaces * 3);

	int k = 0;

	for (UINT i = 0; i < info.NumRows - 1; i++)
	{
		for (UINT j = 0; j < info.NumCols - 1; j++)
		{
			indices[k]		= i * info.NumCols + j;
			indices[k + 1]	= i * info.NumCols + j + 1;
			indices[k + 2]	= (i + 1) * info.NumCols + j;
			indices[k + 3]	= (i + 1) * info.NumCols + j;
			indices[k + 4]	= i * info.NumCols + j + 1;
			indices[k + 5]	= (i + 1) * info.NumCols + j + 1;

			k += 6;
		}
	}

	D3D10_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage			= D3D10_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth		= sizeof(DWORD) * numFaces * 3;
	indexBufferDesc.BindFlags		= D3D10_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags	= 0;
	indexBufferDesc.MiscFlags		= 0;

	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = &indices[0];
	HR(localDevice->CreateBuffer(&indexBufferDesc, &initData, &ib));
}