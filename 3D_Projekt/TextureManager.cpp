#include "TextureManager.h"
#include <fstream>

using namespace std;

TextureManager& GetTextureManager()
{
	static TextureManager tm;

	return tm;
}

TextureManager::TextureManager()
{
	localDevice = 0;
	randomTexRV = 0;
}

TextureManager::~TextureManager()
{
	for (UINT i = 0; i < textureRVs.size(); i++)
	{
		ReleaseCOM(textureRVs[i].textureRV);
	}
}

void TextureManager::init(ID3D10Device* _device)
{
	localDevice = _device;
	buildRandomTex();
}

ID3D10ShaderResourceView* TextureManager::getRandomTex()
{
	return randomTexRV;
}

ID3D10ShaderResourceView* TextureManager::createTex(std::wstring _filename)
{
	for (UINT i = 0; i < textureRVs.size(); i++)
	{
		if (textureRVs[i].name == _filename)
		{
			return textureRVs[i].textureRV;
		}
	}

	ID3D10ShaderResourceView* rv = 0;
	HR(D3DX10CreateShaderResourceViewFromFile(localDevice, _filename.c_str(), 0, 0, &rv, 0));

	textureRVs.push_back(TextureObject(_filename, rv));

	return rv;
}

ID3D10ShaderResourceView* TextureManager::createTexArray(std::wstring _arrayName, const std::vector<std::wstring>& _filenames)
{
	for (UINT i = 0; i < textureRVs.size(); i++)
	{
		if (textureRVs[i].name == _arrayName)
		{
			return textureRVs[i].textureRV;
		}
	}

	UINT arrSize = (UINT) _filenames.size();
	vector<ID3D10Texture2D*> sourceTexture(arrSize, 0);

	for (UINT i = 0; i < arrSize; i++)
	{
		D3DX10_IMAGE_LOAD_INFO imgLoadInfo;
		imgLoadInfo.Width			= D3DX10_FROM_FILE;
		imgLoadInfo.Height			= D3DX10_FROM_FILE;
		imgLoadInfo.Depth			= D3DX10_FROM_FILE;
		imgLoadInfo.FirstMipLevel	= 0;
		imgLoadInfo.MipLevels		= D3DX10_FROM_FILE;
		imgLoadInfo.Usage			= D3D10_USAGE_STAGING;
		imgLoadInfo.BindFlags		= 0;
		imgLoadInfo.CpuAccessFlags	= D3D10_CPU_ACCESS_WRITE | D3D10_CPU_ACCESS_READ;
		imgLoadInfo.MiscFlags		= 0;
		imgLoadInfo.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
		imgLoadInfo.Filter			= D3DX10_FILTER_NONE;
		imgLoadInfo.pSrcInfo		= 0;

		HR(D3DX10CreateTextureFromFile(localDevice, _filenames[i].c_str(), &imgLoadInfo, 0, (ID3D10Resource**)&sourceTexture[i], 0));
	}

	D3D10_TEXTURE2D_DESC tex2dDesc;
	sourceTexture[0]->GetDesc(&tex2dDesc);

	D3D10_TEXTURE2D_DESC tex2dArrDesc;
	tex2dArrDesc.Width				= tex2dDesc.Width;
	tex2dArrDesc.Height				= tex2dDesc.Height;
	tex2dArrDesc.MipLevels			= tex2dDesc.MipLevels;
	tex2dArrDesc.ArraySize			= arrSize;
	tex2dArrDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	tex2dArrDesc.SampleDesc.Count	= 1;
	tex2dArrDesc.SampleDesc.Quality	= 0;
	tex2dArrDesc.Usage				= D3D10_USAGE_DEFAULT;
	tex2dArrDesc.BindFlags			= D3D10_BIND_SHADER_RESOURCE;
	tex2dArrDesc.CPUAccessFlags		= 0;
	tex2dArrDesc.MiscFlags			= 0;

	ID3D10Texture2D* texArr = 0;
	HR(localDevice->CreateTexture2D(&tex2dArrDesc, 0, &texArr));

	for (UINT i = 0; i < arrSize; i++)
	{
		for (UINT j = 0; j < tex2dDesc.MipLevels; j++)
		{
			D3D10_MAPPED_TEXTURE2D mappedTex2D;
			sourceTexture[i]->Map(j, D3D10_MAP_READ, 0, &mappedTex2D);

			localDevice->UpdateSubresource(texArr, D3D10CalcSubresource(j, i, tex2dDesc.MipLevels), 0, mappedTex2D.pData, mappedTex2D.RowPitch, 0);

			sourceTexture[i]->Unmap(j);
		}
	}

	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format							= tex2dArrDesc.Format;
	viewDesc.ViewDimension					= D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip	= 0;
	viewDesc.Texture2DArray.MipLevels		= tex2dArrDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice	= 0;
	viewDesc.Texture2DArray.ArraySize		= arrSize;

	ID3D10ShaderResourceView* texArrRV = 0;

	HR(localDevice->CreateShaderResourceView(texArr, &viewDesc, &texArrRV));

	ReleaseCOM(texArr);

	for (UINT i = 0; i < arrSize; i++)
	{
		ReleaseCOM(sourceTexture[i]);
	}

	textureRVs.push_back(TextureObject(_arrayName, texArrRV));

	return texArrRV;
}

ID3D10ShaderResourceView* TextureManager::createCubeTex(std::wstring _filename)
{
	for (UINT i = 0; i < textureRVs.size(); i++)
	{
		if (textureRVs[i].name == _filename)
		{
			return textureRVs[i].textureRV;
		}
	}

	D3DX10_IMAGE_LOAD_INFO imgLoadInfo;
	imgLoadInfo.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;

	ID3D10Texture2D* tex2d = 0;

	HR(D3DX10CreateTextureFromFile(localDevice, _filename.c_str(), &imgLoadInfo, 0, (ID3D10Resource**)&tex2d, 0));

	D3D10_TEXTURE2D_DESC tex2dDesc;
	tex2d->GetDesc(&tex2dDesc);

	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format							= tex2dDesc.Format;
	viewDesc.ViewDimension					= D3D_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MipLevels			= tex2dDesc.MipLevels;
	viewDesc.TextureCube.MostDetailedMip	= 0;

	ID3D10ShaderResourceView* rv = 0;
	HR(localDevice->CreateShaderResourceView(tex2d, &viewDesc, &rv));

	ReleaseCOM(tex2d);

	textureRVs.push_back(TextureObject(_filename, rv));
	return rv;
}

void TextureManager::buildRandomTex()
{
	D3DXVECTOR4 randVal[1024];

	for (int i = 0; i < 1024; i++)
	{
		randVal[i].x = RandomFloat(-1.0f, 1.0f);
		randVal[i].y = RandomFloat(-1.0f, 1.0f);
		randVal[i].z = RandomFloat(-1.0f, 1.0f);
		randVal[i].w = RandomFloat(-1.0f, 1.0f);
	}

	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem					= randVal;
	initData.SysMemPitch				= 1024 * sizeof(D3DXVECTOR4);
	initData.SysMemSlicePitch			= 1024 * sizeof(D3DXVECTOR4);

	D3D10_TEXTURE1D_DESC texDesc;
	texDesc.Width						= 1024;
	texDesc.MipLevels					= 1;
	texDesc.Format						= DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage						= D3D10_USAGE_IMMUTABLE;
	texDesc.BindFlags					= D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags				= 0;
	texDesc.MiscFlags					= 0;
	texDesc.ArraySize					= 1;

	ID3D10Texture1D* randTex = 0;

	HR(localDevice->CreateTexture1D(&texDesc, &initData, &randTex));

	D3D10_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format						= texDesc.Format;
	viewDesc.ViewDimension				= D3D10_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels		= texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip	= 0;

	HR(localDevice->CreateShaderResourceView(randTex, &viewDesc, &randomTexRV));

	ReleaseCOM(randTex);
}