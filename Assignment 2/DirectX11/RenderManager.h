
#include "load_object_oriented.h"

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
		
		GW::SYSTEM::GLog log;
		log.Create("../Level Renderer/LevelLoaderLog.txt");
		log.EnableConsoleLogging(true);

		bool status = level.LoadLevel("../Level Renderer/GameLevel.txt", "../Level Renderer/Models", log.Relinquish());
		
		level.UploadLevelToGPU(_win, _d3d);
	}
	void Render()
	{
		level.RenderLevel(win, d3d);
	}
	~RenderManager()
	{
		// ComPtr will auto release so nothing to do here yet 
	}

};