#pragma once

#include "D3DUtil.h"
#include <vector>
#include <string>

struct TextureObject 
{
	std::wstring name;
	ID3D10ShaderResourceView* textureRV;

	TextureObject(std::wstring _name, ID3D10ShaderResourceView* _textureRV)
	{
		name = _name;
		textureRV = _textureRV;
	}
};


class TextureManager
{
public:

	friend TextureManager& GetTextureManager();
	typedef std::vector<std::wstring> StringVector;

	void init(ID3D10Device* _device);

	ID3D10ShaderResourceView* getRandomTex();
	ID3D10ShaderResourceView* createTex(std::wstring _filename);
	ID3D10ShaderResourceView* createTexArray(std::wstring _arrayName, const std::vector<std::wstring>& _filenames);
	ID3D10ShaderResourceView* createCubeTex(std::wstring _filename);


private:
	TextureManager();
	~TextureManager();

	void buildRandomTex();

	ID3D10Device* localDevice;
	StringVector textureNames;
	std::vector<TextureObject> textureRVs;
	ID3D10ShaderResourceView* randomTexRV;
};

D3DX10INLINE float RandomFloat()
{
	return (float)(rand()) / (float)RAND_MAX; 
}

D3DX10INLINE float RandomFloat(float _a, float _b)
{
	return _a + RandomFloat() * (_b - _a);
}

TextureManager& GetTextureManager();