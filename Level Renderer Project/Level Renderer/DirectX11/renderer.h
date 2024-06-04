//#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
//#include "../Assets/FSLogo.h"

// Creation, Rendering & Cleanup
class RenderManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;

	Level_Objects level;

public:
	RenderManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;

		GW::SYSTEM::GLog output;
		output.Create("render log.txt");
		output.EnableConsoleLogging(true);
		bool status = level.LoadLevel("../GameLevel.txt", "../Models", output.Relinquish());

		level.UploadLevelToGPU(_win, _d3d);
	}

	void Render()
	{
		level.RenderLevel(win, d3d);
	}

private:
	void CleanUp()
	{
		level.UnloadLevel();
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
