#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 //vista
#endif

#if defined(DEBUG) || defined(_DEBUG) //D3D Debug
	#ifndef D3D_DEBUG_INFO
	#define D3D_DEBUG_INFO
	#endif
#endif

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <d3dx10.h>
#include <dxerr.h>
#include <cassert>
#include <vector>

#if defined(DEBUG) | defined(_DEBUG) //D3D Debug tool
#ifndef HR
#define HR(x)                                              \
	{                                                      \
	HRESULT hr = (x);                                      \
	if(FAILED(hr))                                         \
		{                                                  \
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                  \
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#define ReleaseCOM(_x) { if (_x){_x->Release(); _x = 0;}} //Macro for releasing COM object

const D3DXCOLOR WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const D3DXCOLOR YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);
const D3DXCOLOR BEACH_SAND(1.0f, 0.96f, 0.62f, 1.0f);
const D3DXCOLOR LIGHT_YELLOW_GREEN(0.48f, 0.77f, 0.46f, 1.0f);
const D3DXCOLOR DARK_YELLOW_GREEN(0.1f, 0.48f, 0.19f, 1.0f);
const D3DXCOLOR DARKBROWN(0.45f, 0.39f, 0.34f, 1.0f);