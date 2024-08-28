
#include <commdlg.h>
#include "Utility/load_object_oriented.h"

// Creation, Rendering & Cleanup
class RenderManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	Level_Objects level;
	GW::SYSTEM::GLog log;
	

public:
	RenderManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		
		log.Create("../Levels/LevelLoaderLog.txt");
		log.EnableConsoleLogging(true);

		bool status = level.LoadLevel("../Levels/GameLevel.txt", "../Levels/Models", log.Relinquish());
		
		level.UploadLevelToGPU(win, d3d);
	}
	void Render()
	{
		level.RenderLevel(win, d3d);
	}
	void LoadNewLevel(std::string _filePath)
	{
		level.UnloadLevel();

		size_t lastSlash = _filePath.find_last_of("/\\");

		if (lastSlash != std::string::npos) {
			// Extract file name
			_filePath = _filePath.substr(lastSlash + 1);
		}

		// Assuming the game path is inside a Level Renderer folder
		std::string relativePath = "../Levels/" + _filePath;

		// Other wise I want to just use _filePath (needs to be implented)

		const char* filePath = relativePath.c_str();

		log.Create("../Levels/LevelLoaderLog.txt");
		log.EnableConsoleLogging(true);

		bool status = level.LoadLevel(filePath, "../Levels/Models", log.Relinquish());

		level.UploadLevelToGPU(win, d3d);
	}

	~RenderManager()
	{
		// ComPtr will auto release so nothing to do here yet
		level.UnloadLevel();
	}

};
