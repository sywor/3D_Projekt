#pragma once 

class D3DProj : public D3DApp
{
public:

	D3DProj(HINSTANCE _hInst);
	~D3DProj();
	void initApp();
	void onResize();
	void updateScene(float _dt);
	void drawScene();

	LRESULT msgProc(UINT _msg, WPARAM _wParam, LPARAM _lParam);

private:

	POINT oldMousePos;

	ParticleSystem partSys;
	Terrain terr;
	Sky sky;
	Light parrallelLight;

	D3DXMATRIX landWorld;
	ID3D10ShaderResourceView* enviromentMapRV;
};