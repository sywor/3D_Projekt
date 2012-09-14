#pragma once

#include "D3DUtil.h"
#include "GameTimer.h"
#include "Camera.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE _hInstance);
	virtual ~D3DApp();

	int run();

	virtual void initApp();
	virtual void onResize();	// reset projection matrix etc
	virtual void updateScene(float _dt);
	virtual void drawScene(); 
	virtual LRESULT msgProc(UINT _msg, WPARAM _wParam, LPARAM _lParam);

protected:

	void initMainWindow();
	void initDirect3D();

	HINSTANCE appHInst;
	HWND      MainHWnd;
	bool      appPaused;
	bool      appMinimized;
	bool      appMaximized;
	bool      appResizing;

	GameTimer timer;

	std::wstring frameStats;

	ID3D10Device*			device;
	IDXGISwapChain*			swapChain;
	ID3D10Texture2D*		depthStencilBuffer;
	ID3D10RenderTargetView* renderTargetView;
	ID3D10DepthStencilView* depthStencilView;
	ID3DX10Font*			font;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mainWndCaption;
	D3DXCOLOR clearColor;
	int clientWidth;
	int clientHeight;
};