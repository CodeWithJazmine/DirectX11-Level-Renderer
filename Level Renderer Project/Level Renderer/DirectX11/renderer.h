//#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#include "../Assets/FSLogo.h"





// Creation, Rendering & Cleanup
class RenderManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;

	

public:
	RenderManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
	}

private:



public:
	void Render()
	{
		
	}

	/*void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}*/


public:
	~RenderManager()
	{
		// ComPtr will auto release so nothing to do here yet 
	}
};
