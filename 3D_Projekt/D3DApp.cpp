#include "D3DApp.h"

#include <sstream>

LRESULT CALLBACK
MainWndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	static D3DApp* app = 0;

	switch(_msg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* cs = (CREATESTRUCT*)_lParam;
			app = (D3DApp*)cs->lpCreateParams;
			return 0;
		}
	}

	if (app)
	{
		return app->msgProc(_msg, _wParam, _lParam);
	} 
	else
	{
		return DefWindowProc(_hwnd, _msg, _wParam, _lParam);
	}
}

D3DApp::D3DApp(HINSTANCE _hInstance)
{
	appHInst			= _hInstance;
	MainHWnd			= 0;
	appPaused			= false;
	appMinimized		= false;
	appMaximized		= false;
	appResizing			= false;

	frameStats			= L"";

	device				= 0;
	swapChain			= 0;
	depthStencilBuffer	= 0;
	renderTargetView	= 0;
	depthStencilView	= 0;
	font				= 0;

	mainWndCaption		= L"D3D10 Application";
	clearColor			= BLUE;
	clientWidth			= 1280;
	clientHeight		= 720;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(renderTargetView);
	ReleaseCOM(depthStencilView);
	ReleaseCOM(depthStencilBuffer);
	ReleaseCOM(swapChain);
	ReleaseCOM(device);
	ReleaseCOM(font);
}

int D3DApp::run()
{
	MSG msg = {0};
	timer.reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} 
		else
		{
			timer.tick();
			if (!appPaused)
			{
				updateScene(timer.getDeltaTime());
			} 
			else
			{
				Sleep(50);
			}

			drawScene();
		}
	}

	return (int)msg.wParam;
}

void D3DApp::initApp()
{
	initMainWindow();
	initDirect3D();

	D3DX10_FONT_DESC fontDesc;
	fontDesc.Height				= 30;
	fontDesc.Width				= 0;
	fontDesc.Weight				= 0;
	fontDesc.MipLevels			= 1;
	fontDesc.Italic				= false;
	fontDesc.CharSet			= DEFAULT_CHARSET;
	fontDesc.OutputPrecision	= OUT_DEFAULT_PRECIS;
	fontDesc.Quality			= DEFAULT_QUALITY;
	fontDesc.PitchAndFamily		= DEFAULT_PITCH | FF_DONTCARE;
	wcscpy_s(fontDesc.FaceName, L"Calibri");

	D3DX10CreateFontIndirect(device, &fontDesc, &font);
}

void D3DApp::onResize()
{
	ReleaseCOM(renderTargetView);
	ReleaseCOM(depthStencilBuffer);
	ReleaseCOM(depthStencilView);

	HR(swapChain->ResizeBuffers(1, clientWidth, clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D10Texture2D* backBuffer;
	HR(swapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(device->CreateRenderTargetView(backBuffer, 0, &renderTargetView));
	ReleaseCOM(backBuffer);

	D3D10_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width				= clientWidth;
	depthStencilDesc.Height				= clientHeight;
	depthStencilDesc.MipLevels			= 1;
	depthStencilDesc.ArraySize			= 1;
	depthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count	= 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage				= D3D10_USAGE_DEFAULT;
	depthStencilDesc.BindFlags			= D3D10_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags		= 0;
	depthStencilDesc.MiscFlags			= 0;

	HR(device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer));
	HR(device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView));

	device->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	D3D10_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width	= clientWidth;
	vp.Height	= clientHeight;
	vp.MinDepth	= 0.0f;
	vp.MaxDepth = 1.0f;

	device->RSSetViewports(1, &vp);
}

void D3DApp::updateScene(float _dt)
{
	static int frameCount = 0;
	static float base = 0.0f;

	frameCount++;

	if ((timer.getGameTime() - base) >= 1.0f)
	{
		float fps = (float)frameCount;
		float milliSecPerFrame = 1000.0f / fps;

		std::wostringstream out;
		out.precision(6);

		D3DXVECTOR3 pos = D3DXVECTOR3(GetCamera().Position().x, GetCamera().Position().y, GetCamera().Position().z);
		D3DXVECTOR3 look = GetCamera().Look();
		//D3DXMATRIX angle = GetCamera().getAngle();
		//D3DXMATRIX pitch = GetCamera().GetPitch();

		out << L"FPS: " << fps << L" Milliseconds per frame: " << milliSecPerFrame;
		out << L"\n" << L"Camera pos XYZ: " << pos.x << L" : " << pos.y << L" : " << pos.z;
		out << L"\n" << L"Camera look XYZ: " << look.x << " : "  << look.y << " : "  << look.z; 
		//out	<< L"\n" << angle(0,0) << L" : " << angle(0,1) << L" : " << angle(0,2) << L" : " << angle(0,3);
		//out << L"\n" << angle(1,0) << L" : " << angle(1,1) << L" : " << angle(1,2) << L" : " << angle(1,3);
		//out << L"\n" << angle(2,0) << L" : " << angle(2,1) << L" : " << angle(2,2) << L" : " << angle(2,3);
		//out << L"\n" << angle(3,0) << L" : " << angle(3,1) << L" : " << angle(3,2) << L" : " << angle(3,3);
		//out << L"\n" << L"Camera pitch:";
		//out << L"\n" << pitch(0,0) << L" : " << pitch(0,1) << L" : " << pitch(0,2) << L" : " << pitch(0,3);
		//out << L"\n" << pitch(1,0) << L" : " << pitch(1,1) << L" : " << pitch(1,2) << L" : " << pitch(1,3);
		//out << L"\n" << pitch(2,0) << L" : " << pitch(2,1) << L" : " << pitch(2,2) << L" : " << pitch(2,3);
		//out << L"\n" << pitch(3,0) << L" : " << pitch(3,1) << L" : " << pitch(3,2) << L" : " << pitch(3,3);
		frameStats = out.str();

		frameCount = 0;
		base += 1.0f;
	}
}

void D3DApp::drawScene()
{
	device->ClearRenderTargetView(renderTargetView, clearColor);
	device->ClearDepthStencilView(depthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);
}

LRESULT D3DApp::msgProc(UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_msg)
	{
	case WM_ACTIVATE: // minimize / maximize window
		if (LOWORD(_wParam) == WA_INACTIVE)
		{
			appPaused = true;
			timer.stop();
		} 
		else
		{
			appPaused = false;
			timer.start();
		}
		return 0;

	case WM_SIZE: // resize window
		clientWidth = LOWORD(_lParam);
		clientHeight = LOWORD(_lParam);

		if (device)
		{
			if (_wParam == SIZE_MINIMIZED)
			{
				appPaused = true;
				appMinimized = true;
				appMaximized = false;
			} 
			else if (_wParam == SIZE_MAXIMIZED)
			{
				appPaused = false;
				appMinimized = false;
				appMaximized = true;
				onResize();
			}

			else if (_wParam == SIZE_RESTORED)
			{
				if (appMinimized)
				{
					appPaused = false;
					appMinimized = false;
					onResize();
				}

				else if (appMaximized)
				{
					appPaused = false;
					appMaximized = false;
					onResize();
				}

				else
				{
					onResize();
				}
			}
		}

		return 0;

	case WM_ENTERSIZEMOVE:
		appPaused = true;
		appResizing = true;
		timer.stop();
		return 0;
		
	case WM_EXITSIZEMOVE:
		appPaused = false;
		appResizing = false;
		timer.start();
		onResize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)_lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)_lParam)->ptMinTrackSize.y = 200;
		return 0;
	}

	return DefWindowProc(MainHWnd, _msg, _wParam, _lParam);
}

void D3DApp::initMainWindow()
{
	WNDCLASS wc;
	wc.style			=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= MainWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= appHInst;
	wc.hIcon			= LoadIcon(0, IDI_APPLICATION);
	wc.hCursor			= LoadCursor(0, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= L"D3DX10_Project";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"Registering class failed", 0, 0);
		PostQuitMessage(0);
	}

	RECT R = {0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

	int actualWidth = R.right - R.left;
	int actualHeight = R.bottom - R.top;

	MainHWnd = CreateWindow(L"D3DX10_Project", mainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, actualWidth, actualHeight, 0, 0, appHInst, this);

	if (!MainHWnd)
	{
		MessageBox(0, L"Create window failed", 0, 0);
		PostQuitMessage(0);
	}

	ShowWindow(MainHWnd, SW_SHOW);
	UpdateWindow(MainHWnd);
}

void D3DApp::initDirect3D()
{
	DXGI_SWAP_CHAIN_DESC swapShainDesc;
	swapShainDesc.BufferDesc.Width						= clientWidth;
	swapShainDesc.BufferDesc.Height						= clientHeight;
	swapShainDesc.BufferDesc.RefreshRate.Numerator		= 60;
	swapShainDesc.BufferDesc.RefreshRate.Denominator	= 1;
	swapShainDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapShainDesc.BufferDesc.ScanlineOrdering			= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapShainDesc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;

	swapShainDesc.SampleDesc.Count						= 1;
	swapShainDesc.SampleDesc.Quality					= 0;

	swapShainDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapShainDesc.BufferCount							= 1;
	swapShainDesc.OutputWindow							= MainHWnd;
	swapShainDesc.Windowed								= true;
	swapShainDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;
	swapShainDesc.Flags									= 0;

	HR(D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, D3D10_CREATE_DEVICE_DEBUG, D3D10_SDK_VERSION, &swapShainDesc, &swapChain, &device));

	onResize();
}