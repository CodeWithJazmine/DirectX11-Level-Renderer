//#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#include "../Assets/FSLogo.h"
#include "load_object_oriented.h"
//#pragma comment(lib, "d3dcompiler.lib") 



// Creation, Rendering & Cleanup
class RendererManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;

public:
	RendererManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		
		GW::SYSTEM::GLog log;
		log.Create("../Level Renderer/LevelLoaderLog.txt");
		log.EnableConsoleLogging(true);

		log.Log("Start Program.");

		Level_Objects objectOrientedLoader;
		objectOrientedLoader.LoadLevel("../Level Renderer/GameLevel.txt", "../Level Renderer/Models", log);
		
		log.Log("End Program.");
		
		objectOrientedLoader.RenderLevel(_win, _d3d);
	}
	void Render()
	{

	}
	~RendererManager()
	{
		// ComPtr will auto release so nothing to do here yet 
	}

};