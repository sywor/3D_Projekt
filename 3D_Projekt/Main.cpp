
#include "D3DApp.h"

#include "Effects.h"
#include "InputLayouts.h"
#include "TextureManager.h"
#include "Sky.h"
#include "Terrain.h"
#include "ParticleSystem.h"
#include "Light.h"
#include "Main.h"
#include <vector>

using namespace std;

int WINAPI WinMain( __in HINSTANCE _hInstance, __in_opt HINSTANCE _hPrevInstance, __in LPSTR _lpCmdLine, __in int _nShowCmd )
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	D3DProj application(_hInstance);
	application.initApp();
	return application.run();	
}

D3DProj::D3DProj(HINSTANCE _hInst) : D3DApp(_hInst)
{
	enviromentMapRV = 0;
	D3DXMatrixIdentity(&Map);
}

D3DProj::~D3DProj()
{
	if (device)
	{
		device->ClearState();
	}

	fx::DestroyAll();
	InputLayout::DestroyAll();
}

void D3DProj::initApp()
{
	D3DApp::initApp();

	fx::InitAll(device);
	InputLayout::InitAll(device);
	GetTextureManager().init(device);

	clearColor = WHITE;
	GetCamera().Position() = D3DXVECTOR3(0.0f, -10.0f, 0.0f);

	enviromentMapRV = GetTextureManager().createCubeTex(L"cubeMap.dds");
	sky.init(device, enviromentMapRV, 5000.0f);

	InitInfo tii;

	tii.HeightmapFilename = "coastMountain513.raw";
	tii.LayerMapFilename0 = L"grass.dds";
	tii.LayerMapFilename1 = L"lightdirt.dds";	
	tii.LayerMapFilename2 = L"darkdirt.dds";	
	tii.LayerMapFilename3 = L"stone.dds";		
	tii.LayerMapFilename4 = L"snow.dds";		
	tii.BlendMapFilename  = L"blend2.dds";
	tii.HeightScale  = 0.25f;
	tii.HeightOffset = -20.0f;
	tii.NumRows      = 513;
	tii.NumCols      = 513;
	tii.CellSpacing  = 1.0f;

	terrain.init(device, tii);

	vector<wstring> flares;
	flares.push_back(L"flare0.dds");
	ID3D10ShaderResourceView* texArr = GetTextureManager().createTexArray(L"flares", flares);

	partSys.init(device, fx::FireFX, texArr, 500);

	parrallelLight.dir      = D3DXVECTOR3(0.707f, -0.707f, 0.0f);
	parrallelLight.ambient  = D3DXCOLOR(0.3f, 0.3f, 0.3f, 1.0f);
	parrallelLight.diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	parrallelLight.specular = D3DXCOLOR(0.5f, 0.4843f, 0.3f, 1.0f);

	terrain.setDirectionToSun(-parrallelLight.dir);
}

void D3DProj::onResize()
{
	D3DApp::onResize();

	float aspect = (float)clientWidth / clientHeight;
	GetCamera().SetLens(0.25f * (float)D3DX_PI, aspect, 0.5f, 1000.0f);
}

void D3DProj::updateScene(float _dt)
{
	D3DApp::updateScene(_dt);
	partSys.update(_dt, timer.getGameTime());
	GetCamera().Record(_dt);
	GetCamera().PlayRecording(_dt);
	GetCamera().RebuildView();

	if (!GetCamera().PlayBool)
	{	
		if (GetAsyncKeyState('A') & 0x8000)
		{
			GetCamera().Strafe(20.0f * _dt);
		}

		if (GetAsyncKeyState('D') & 0x8000)
		{
			GetCamera().Strafe(-20.0f * _dt);
		}

		if (GetAsyncKeyState('W') & 0x8000)
		{
			GetCamera().Walk(-20.0f * _dt);
		}

		if (GetAsyncKeyState('S') & 0x8000)
		{
			GetCamera().Walk(20.f * _dt);
		}
	}
}

void D3DProj::executeKeys(WPARAM _wparam)
{
	if (_wparam == 82) //R
	{
		if (GetCamera().RecordBool)
		{
			GetCamera().RecordBool = false;
		} 
		else
		{
			GetCamera().RecordBool = true;
		}
	} 
	else if (_wparam == 79) //O
	{
		GetCamera().SaveRecording();
	}
	else if (_wparam == 80) //P
	{
		if (GetCamera().PlayBool)
		{
			GetCamera().PlayBool = false;
		} 
		else
		{
			GetCamera().PlayBool = true;
		}
	}
	else if (_wparam == 76) //L
	{
		GetCamera().LoadRecording();
	}
}

void D3DProj::drawScene()
{
	D3DApp::drawScene();

	device->OMSetDepthStencilState(0, 0);

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	device->OMSetBlendState(0, blendFactor, 0xffffffff);
	device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	terrain.draw(Map);
	sky.draw();

	partSys.setEyePos(GetCamera().Position());
	partSys.draw();

	device->OMSetBlendState(0, blendFactor, 0xffffffff);

	RECT R = {5, 5, 0, 0};
	device->RSSetState(0);
	font->DrawText(0, frameStats.c_str(), -1, &R, DT_NOCLIP, WHITE);
	swapChain->Present(0, 0);
}

LRESULT D3DProj::msgProc(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	POINT mousePos;
	int dx = 0;
	int dy = 0;

	switch(_msg)
	{
	case WM_LBUTTONDOWN:
		if (_wParam & MK_LBUTTON)
		{
			SetCapture(MainHWnd);
			oldMousePos.x = LOWORD(_lParam);
			oldMousePos.y = LOWORD(_lParam);
		}
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		break;

	case WM_MOUSEMOVE:
		if (_wParam & MK_LBUTTON && !GetCamera().PlayBool)
		{
			mousePos.x = (int)LOWORD(_lParam);
			mousePos.y = (int)HIWORD(_lParam);

			dx = mousePos.x - oldMousePos.x;
			dy = mousePos.y - oldMousePos.y;

			GetCamera().Pitch(dy * 0.01f);
			GetCamera().RotateY(dx * 0.01f);

			oldMousePos = mousePos;
		}
		break;

	case WM_KEYDOWN:
		{
			executeKeys(_wParam);
		}
	}

	return D3DApp::msgProc(_msg, _wParam, _lParam);
}