// Simple basecode showing how to create a window and attatch a d3d11surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
#define GATEWARE_ENABLE_MATH // enables Gateware's math library
#define GATEWARE_ENABLE_INPUT // enables Gateware's input library
#define GATEWARE_ENABLE_AUDIO // enables Gateware's audio library

// With what we want & what we don't defined we can include the API

#include "../../gateware-main/Gateware.h"
#include <ShObjIdl.h> // allows for dialog boxes
#include "../Source/Utility/FileIntoString.h"
#include "RenderManager.h" // example rendering code (not Gateware code!)
#include "Camera.h"
#include "DialogBox.h" 
#include "Audio.h"

// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	GWindow win;
	GEventResponder msgs;
	GDirectX11Surface d3d11;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		win.SetWindowName("Jazmine Chargualaf - Level Renderer - DirextX11");
		float clr[] = { 1.0f, 192 / 255.0f, 203 / 255.0f, 1 }; 
		msgs.Create([&](const GW::GEvent& e) {
			GW::SYSTEM::GWindow::Events q;
			if (+e.Read(q) && q == GWindow::Events::RESIZE)
			{
				clr[2] += 0.01f; // move towards a cyan as they resize
			}
		});
		win.Register(msgs);
		if (+d3d11.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		{
			RenderManager renderer(win, d3d11);
			Camera camera(win, d3d11); // main camera
			Audio audio;


			while (+win.ProcessWindowEvents())
			{
				// Pressing F1 opens dialog box to load in new level
				if (GetAsyncKeyState(VK_F1) & 0x0001)
				{
					std::cout << "F1 key was pressed. Opening file dialog...\n";
					std::string filePath = FileDialogs::OpenFile("Text File\0*.txt\0");
					if (!filePath.empty())
					{
						renderer.LoadNewLevel(filePath);
					}
				}
				// Pressing ESC quits the program
				if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
				{
					std::cout << "Quitting the program...\n";
					break;
				}
				// Pressing 'C' switches the camera state
				if (GetAsyncKeyState(0x43) & 0x0001)
				{
					int cameraState = camera.GetCameraState();
					camera.SwitchCamera(cameraState);
					audio.PlaySoundEffect();
				}

				IDXGISwapChain* swap;
				ID3D11DeviceContext* con;
				ID3D11RenderTargetView* view;
				ID3D11DepthStencilView* depth;
				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetDepthStencilView((void**)&depth) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, clr);
					con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);

					// Main Viewport
					D3D11_VIEWPORT viewport1;
					viewport1.Width = 800;
					viewport1.Height = 600;
					viewport1.MinDepth = 0.0f;
					viewport1.MaxDepth = 1.0f;
					viewport1.TopLeftX = 0;
					viewport1.TopLeftY = 0;
					con->RSSetViewports(1, &viewport1);

					camera.UpdateCamera();
					camera.UpdateConstantBuffer();
					renderer.Render();

					con->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);

					// MiniMap Viewport
					D3D11_VIEWPORT viewport2;
					viewport2.Width = 256;
					viewport2.Height = 256;
					viewport2.MinDepth = 0.0f;
					viewport2.MaxDepth = 1.0f;
					viewport2.TopLeftX = 0;
					viewport2.TopLeftY = 0;
					con->RSSetViewports(1, &viewport2);

					camera.UpdateMiniMapConstantBuffer();
					renderer.Render();

					swap->Present(1, 0);
					// release incremented COM reference counts
					swap->Release();
					view->Release();
					depth->Release();
					con->Release();
				}

				
			}
		}
	}
	return 0; // that's all folks
}