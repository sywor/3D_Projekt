#include "ParticleSystem.h"
#include "TextureManager.h"
#include "InputLayouts.h"
#include "Effects.h"
#include "Camera.h"

ParticleSystem::ParticleSystem()
{
	localDevice		= 0;
	initVB			= 0;
	drawVB			= 0;
	streamOutVB		= 0;
	texArrayRV		= 0;
	randomTexRV		= 0;
	firstRun		= true;
	gameTime		= 0.0f;
	timeStep		= 0.0f;
	age				= 0.0f;

	eyePos			= D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	emitPos			= D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	emitDir			= D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.0f);
}

ParticleSystem::~ParticleSystem()
{
	ReleaseCOM(initVB);
	ReleaseCOM(drawVB);
	ReleaseCOM(streamOutVB);
}

void ParticleSystem::setEyePos(const D3DXVECTOR3& _eyePos)
{
	eyePos = D3DXVECTOR4(_eyePos.x, _eyePos.y, _eyePos.z, 1.0f);
}

void ParticleSystem::setEmitPos(const D3DXVECTOR3& _emitPos)
{
	emitPos = D3DXVECTOR4(_emitPos.x, _emitPos.y, _emitPos.z, 1.0f);
}

void ParticleSystem::setEmitDir(const D3DXVECTOR3& _emitDir)
{
	emitDir = D3DXVECTOR4(_emitDir.x, _emitDir.y, _emitDir.z, 0.0f);
}

void ParticleSystem::init(ID3D10Device* _device, ID3D10Effect* FX, ID3D10ShaderResourceView* _texArrayRV, UINT _maxParticles)
{
	localDevice		= _device;
	maxParticles	= _maxParticles;
	texArrayRV		= _texArrayRV;
	randomTexRV		= GetTextureManager().getRandomTex();

	streamOutTech	= FX->GetTechniqueByName("StreamOutTech");	
	drawTech		= FX->GetTechniqueByName("DrawTech");	
	viewProjVar		= FX->GetVariableByName("ViewProj")->AsMatrix();
	gameTimeVar		= FX->GetVariableByName("GameTime")->AsScalar();
	timeStepVar		= FX->GetVariableByName("TimeStep")->AsScalar();
	eyePosVar		= FX->GetVariableByName("EyePos")->AsVector();
	emitPosVar		= FX->GetVariableByName("EmitPos")->AsVector();
	emitDirVar		= FX->GetVariableByName("EmitDir")->AsVector();
	texArrayVar		= FX->GetVariableByName("TexArray")->AsShaderResource();
	randomTexVar	= FX->GetVariableByName("RandomTex")->AsShaderResource();

	buildVB();
}

void ParticleSystem::reset()
{
	firstRun	= true;
	age			= 0.0f;
}

void ParticleSystem::update(float _dt, float _gameTime)
{
	gameTime	= _gameTime;
	timeStep	= _dt;
	age			+= _dt;
}

void ParticleSystem::draw()
{
	D3DXMATRIX V = GetCamera().ViewMatrix();
	D3DXMATRIX P = GetCamera().ProjMatrix();

	viewProjVar->SetMatrix((float*)&(V * P));
	gameTimeVar->SetFloat(gameTime);
	timeStepVar->SetFloat(timeStep);
	eyePosVar->SetFloatVector((float*)&eyePos);
	emitPosVar->SetFloatVector((float*)&emitPos);
	emitDirVar->SetFloatVector((float*)&emitDir);
	texArrayVar->SetResource(texArrayRV);
	randomTexVar->SetResource(randomTexRV);

	localDevice->IASetInputLayout(InputLayout::Particle);
	localDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;

	if (firstRun)
	{
		localDevice->IASetVertexBuffers(0, 1, &initVB, &stride, &offset);
	} 
	else
	{
		localDevice->IASetVertexBuffers(0, 1, &drawVB, &stride, &offset);
	}

	localDevice->SOSetTargets(1, &streamOutVB, &offset);

	D3D10_TECHNIQUE_DESC techDesc;
	streamOutTech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		streamOutTech->GetPassByIndex(i)->Apply(0);

		if (firstRun)
		{
			localDevice->Draw(1, 0);
			firstRun = false;
		} 
		else
		{
			localDevice->DrawAuto();
		}
	}

	ID3D10Buffer* bufferArr[1] = {0};
	localDevice->SOSetTargets(1, bufferArr, &offset);

	std::swap(drawVB, streamOutVB);

	localDevice->IASetVertexBuffers(0, 1, &drawVB, &stride, &offset);
	drawTech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; i++)
	{
		drawTech->GetPassByIndex(i)->Apply(0);
		localDevice->DrawAuto();
	}
}

void ParticleSystem::buildVB()
{
	D3D10_BUFFER_DESC buffDesc;
	buffDesc.Usage			= D3D10_USAGE_DEFAULT;
	buffDesc.ByteWidth		= sizeof(ParticleVertex);
	buffDesc.BindFlags		= D3D10_BIND_VERTEX_BUFFER;
	buffDesc.CPUAccessFlags = 0;
	buffDesc.MiscFlags		= 0;

	ParticleVertex p;
	ZeroMemory(&p, sizeof(ParticleVertex));
	p.age = 0.0f;
	p.type = 0;

	D3D10_SUBRESOURCE_DATA initData;
	initData.pSysMem = &p;

	HR(localDevice->CreateBuffer(&buffDesc, &initData, &initVB));

	buffDesc.ByteWidth = sizeof(ParticleVertex) * maxParticles;
	buffDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER | D3D10_BIND_STREAM_OUTPUT;

	HR(localDevice->CreateBuffer(&buffDesc, 0, &drawVB));
	HR(localDevice->CreateBuffer(&buffDesc, 0, &streamOutVB));
}