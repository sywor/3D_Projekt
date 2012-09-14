#pragma once

#include "D3DUtil.h"
#include <string>
#include <vector>

struct ParticleVertex
{
	D3DXVECTOR3 initPos;
	D3DXVECTOR3 initVel;
	D3DXVECTOR2 size;
	float age;
	UINT type;
};

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	// Time elapsed since the system was reset.
	float getAge()const;

	void setEyePos(const D3DXVECTOR3& _eyePos);
	void setEmitPos(const D3DXVECTOR3& _emitPos);
	void setEmitDir(const D3DXVECTOR3& _emitDir);

	void init(ID3D10Device* _device, ID3D10Effect* FX, ID3D10ShaderResourceView* _texArrayRV, UINT _maxParticles);

	void reset();
	void update(float _dt, float _gameTime);
	void draw();

private:
	void buildVB();

	UINT									maxParticles;
	bool									firstRun;

	float									gameTime;
	float									timeStep;
	float									age;

	D3DXVECTOR4								eyePos;
	D3DXVECTOR4								emitPos;
	D3DXVECTOR4								emitDir;

	ID3D10Device*							localDevice;
	ID3D10Buffer*							initVB;	
	ID3D10Buffer*							drawVB;
	ID3D10Buffer*							streamOutVB;

	ID3D10ShaderResourceView*				texArrayRV;
	ID3D10ShaderResourceView*				randomTexRV;

	ID3D10EffectTechnique*					streamOutTech;
	ID3D10EffectTechnique*					drawTech;

	ID3D10EffectMatrixVariable*				viewProjVar;
	ID3D10EffectScalarVariable*				gameTimeVar;
	ID3D10EffectScalarVariable*				timeStepVar;
	ID3D10EffectVectorVariable*				eyePosVar;
	ID3D10EffectVectorVariable*				emitPosVar;
	ID3D10EffectVectorVariable*				emitDirVar;

	ID3D10EffectShaderResourceVariable*		texArrayVar;
	ID3D10EffectShaderResourceVariable*		randomTexVar;
};