// This is a sample of how to load a level in a object oriented fashion.
// Feel free to use this code as a base and tweak it for your needs.
#include <d3dcompiler.h>
// This reads .h2b files which are optimized binary .obj+.mtl files
#include "h2bParser.h"
#pragma comment(lib, "d3dcompiler.lib") 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

struct SceneData
{
	GW::MATH::GVECTORF sunDirection, sunColor; // lighting info
	GW::MATH::GMATRIXF viewMatrix, projectionMatrix;
	GW::MATH::GVECTORF sunAmbient, cameraPos;

};

struct MeshData
{
	GW::MATH::GMATRIXF worldMatrix; // world space transformation
	H2B::ATTRIBUTES material; // material info (color, reflectivity, emissiveness, etc)
};

// class Model contains everyhting needed to draw a single 3D model
class Model {
	// Name of the Model in the GameLevel (useful for debugging)
	std::string name;
	// Loads and stores CPU model data from .h2b file
	H2B::Parser cpuModel; // reads the .h2b format
	// Shader variables needed by this model. 
	GW::MATH::GMATRIXF world;

	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		meshConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	GW::MATH::GMatrix  matrixProxy;
	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
	GW::MATH::GMATRIXF rotationMatrix;

	GW::MATH::GVector  vectorProxy;
	GW::MATH::GVECTORF lightDirection;
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF sunAmbient;

	SceneData sceneData;
	MeshData meshData;


public:
	inline void SetName(std::string modelName) {
		name = modelName;
	}
	inline void SetWorldMatrix(GW::MATH::GMATRIXF worldMatrix) {
		world = worldMatrix;
	}
	bool LoadModelDataFromDisk(const char* h2bPath) {
		// if this succeeds "cpuModel" should now contain all the model's info
		return cpuModel.Parse(h2bPath);
	}
	bool UploadModelData2GPU(GW::SYSTEM::GWindow win, GW::GRAPHICS::GDirectX11Surface d3d) {
		// TODO: Use chosen API to upload this model's graphics data to GPU
		matrixProxy.Create();
		vectorProxy.Create();

		InitializeMatrices(d3d);
		InitializeLight();

		InitializeSceneData();
		InitializeMeshData();

		IntializeGraphics(d3d);

		return true;
	}

private:
	//constructor helper functions
	void IntializeGraphics(GW::GRAPHICS::GDirectX11Surface d3d)
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);

		InitializeIndexBuffer(creator);

		InitializeSceneConstantBuffer(creator, &sceneData);
		InitializeMeshConstantBuffer(creator, &meshData);

		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device * creator)
	{
		CreateVertexBuffer(creator, &cpuModel.vertices[0], sizeof(H2B::VERTEX) * cpuModel.vertices.size());
	}

	void CreateVertexBuffer(ID3D11Device * creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA vbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC vbDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&vbDesc, &vbData, vertexBuffer.GetAddressOf());
	}

	void InitializeIndexBuffer(ID3D11Device * creator)
	{
		CreateIndexBuffer(creator, &cpuModel.indices[0], sizeof(UINT) * cpuModel.indices.size());
	}

	void CreateIndexBuffer(ID3D11Device * creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA ibData = { data, 0, 0 };
		CD3D11_BUFFER_DESC ibDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&ibDesc, &ibData, indexBuffer.GetAddressOf());
	}

	void InitializeSceneConstantBuffer(ID3D11Device * creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA scbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC scbDesc(sizeof(SceneData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&scbDesc, &scbData, sceneConstantBuffer.GetAddressOf());
	}

	void InitializeMeshConstantBuffer(ID3D11Device * creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA mcbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC mcbDesc(sizeof(MeshData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&mcbDesc, &mcbData, meshConstantBuffer.GetAddressOf());
	}

	void InitializeMatrices(GW::GRAPHICS::GDirectX11Surface d3d)
	{
		// World Matrix: An identity matrix that slowly rotates along the Y axis over time.
		matrixProxy.IdentityF(worldMatrix);
		matrixProxy.IdentityF(rotationMatrix);

		// View: A camera positioned at 0.75x +0.25y -1.5z that is rotated to look at +0.15x +0.75y +0z.
		matrixProxy.LookAtLHF(
			GW::MATH::GVECTORF{ 0.75f, 0.25f, -1.5f }, // Camera position
			GW::MATH::GVECTORF{ 0.15f, 0.75f, 0.0f }, // Look at position
			GW::MATH::GVECTORF{ 0.0f, 1.0f, 0.0f }, // Up direction
			viewMatrix); // 

		// Projection: A vertical field of view of 65 degrees, and a near and far plane of 0.1 and 100 respectively.
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		matrixProxy.ProjectionDirectXLHF(
			G2D_DEGREE_TO_RADIAN_F(65.0f),       // Field of view
			aspectRatio,						 // Aspect ratio
			0.1f,                                // Near plane
			100.0f,                              // Far plane
			projectionMatrix);
	}

	void InitializeLight()
	{
		// Light Direction:  Forward with a strong tilt down and to the left. -1x -1y +2z (normalize)
		vectorProxy.NormalizeF(GW::MATH::GVECTORF{ -1.0f, -1.0f, 2.0f }, lightDirection);
		// Light Color: Almost white with a slight blueish tinge. 0.9r 0.9g 1.0b 1.0a
		lightColor = GW::MATH::GVECTORF{ 0.9f, 0.9f, 1.0f, 1.0f };
		// Sun Ambient: 25% red, 25% green, and 35% blue
		sunAmbient = GW::MATH::GVECTORF{ 0.25f, 0.25f, 0.35f };
	}

	void InitializeSceneData()
	{
		sceneData.sunDirection = lightDirection;
		sceneData.sunColor = lightColor;
		sceneData.viewMatrix = viewMatrix;
		sceneData.projectionMatrix = projectionMatrix;
		sceneData.sunAmbient = sunAmbient;
		sceneData.cameraPos = GW::MATH::GVECTORF{ 0.75f, 0.25f, -1.5f };
	}

	void InitializeMeshData()
	{
		meshData.worldMatrix = world;
		//meshData.worldMatrix = worldMatrix; 
		meshData.material = cpuModel.materials[0].attrib;
	}

	void InitializePipeline(ID3D11Device * creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);

		CreateVertexInputLayout(creator, vsBlob);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device * creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device * creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;
	}

	void CreateVertexInputLayout(ID3D11Device * creator, Microsoft::WRL::ComPtr<ID3DBlob>&vsBlob)
	{
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		attributes[1].SemanticName = "UV";
		attributes[1].SemanticIndex = 0;
		attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[1].InputSlot = 0;
		attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[1].InstanceDataStepRate = 0;

		attributes[2].SemanticName = "NORMAL";
		attributes[2].SemanticIndex = 0;
		attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[2].InputSlot = 0;
		attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[2].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

	PipelineHandles GetCurrentPipelineHandles(GW::GRAPHICS::GDirectX11Surface d3d)
	{
		PipelineHandles retval;
		d3d.GetImmediateContext((void**)&retval.context);
		d3d.GetRenderTargetView((void**)&retval.targetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
	}

	void SetUpPipeline(PipelineHandles handles)
	{
		SetRenderTargets(handles);
		SetVertexBuffers(handles);
		SetIndexBuffers(handles);
		SetConstantBuffers(handles);
		SetShaders(handles);

		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		const UINT strides[] = { sizeof(H2B::VERTEX) }; 
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);

	}

	void SetIndexBuffers(PipelineHandles handles)
	{
		handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void SetConstantBuffers(PipelineHandles handles)
	{
		handles.context->VSSetConstantBuffers(0, 1, sceneConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(0, 1, sceneConstantBuffer.GetAddressOf());

		handles.context->VSSetConstantBuffers(1, 1, meshConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(1, 1, meshConstantBuffer.GetAddressOf());

		
	}
	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}

public:
	bool DrawModel(GW::SYSTEM::GWindow win ,GW::GRAPHICS::GDirectX11Surface d3d) {
		// TODO: Use chosen API to setup the pipeline for this model and draw it

			PipelineHandles curHandles = GetCurrentPipelineHandles(d3d);
			SetUpPipeline(curHandles);
			D3D11_MAPPED_SUBRESOURCE mappedResource;

			/*curHandles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &meshData, sizeof(MeshData));
			curHandles.context->Unmap(meshConstantBuffer.Get(), 0);*/

			for (auto& m : cpuModel.meshes)
			{
				curHandles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				meshData.material = cpuModel.materials[m.materialIndex].attrib;
				memcpy(mappedResource.pData, &meshData, sizeof(MeshData));
				curHandles.context->Unmap(meshConstantBuffer.Get(), 0);
				curHandles.context->DrawIndexed(m.drawInfo.indexCount, m.drawInfo.indexOffset, 0);
			}

			ReleasePipelineHandles(curHandles);
				
			return false;

		//PipelineHandles curHandles = GetCurrentPipelineHandles(d3d);
		//SetUpPipeline(curHandles);

		//// Set the scene constant buffer for the vertex shader
		//curHandles.context->VSSetConstantBuffers(0, 1, sceneConstantBuffer.GetAddressOf());
		//// Set the scene constant buffer for the pixel shader
		//curHandles.context->PSSetConstantBuffers(0, 1, sceneConstantBuffer.GetAddressOf());

		//for (auto& m : cpuModel.meshes) {
		//	// Update the material properties in the constant buffer
		//	meshData.material = cpuModel.materials[m.materialIndex].attrib;

		//	D3D11_MAPPED_SUBRESOURCE mappedResource;
		//	if (SUCCEEDED(curHandles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
		//		MeshData* buffer = reinterpret_cast<MeshData*>(mappedResource.pData);
		//		*buffer = meshData;
		//		curHandles.context->Unmap(meshConstantBuffer.Get(), 0);
		//	}

		//	// Set the mesh constant buffer for the vertex shader
		//	curHandles.context->VSSetConstantBuffers(1, 1, meshConstantBuffer.GetAddressOf());
		//	// Set the mesh constant buffer for the pixel shader
		//	curHandles.context->PSSetConstantBuffers(1, 1, meshConstantBuffer.GetAddressOf());

		//	// Draw the mesh
		//	curHandles.context->DrawIndexed(m.drawInfo.indexCount, m.drawInfo.indexOffset, 0);
		//}

		//ReleasePipelineHandles(curHandles);

		//return false;
	}

	bool FreeResources(PipelineHandles toRelease) {
		// TODO: Use chosen API to free all GPU resources used by this model

		return false;
	}
};

// * NOTE: *
// Unlike the DOP version, this class was not designed to reuse data in anyway or process it efficiently.
// You can find ways to make it more efficient by sharing pointers to resources and sorting the models.
// However, this is tricky to implement and can be prone to errors. (OOP data isolation becomes an issue)
// A huge positive is that everything you want to draw is totally self contained and easy to see/find.
// This means updating matricies, adding new objects & removing old ones from the world is a breeze. 
// You can even easily load brand new models from disk at run-time without much trouble.
// The last major downside is trying to do things like dynamic lights, shadows and sorted transparency. 
// Effects like these expect your model set to be processed/traversed in unique ways which can be awkward.   

// class Level_Objects is simply a list of all the Models currently used by the level
class Level_Objects {

	// store all our models
	std::list<Model> allObjectsInLevel;
	// TODO: This could be a good spot for any global data like cameras or lights
public:
	
	// Imports the default level txt format and creates a Model from each .h2b
	bool LoadLevel(	const char* gameLevelPath,
					const char* h2bFolderPath,
					GW::SYSTEM::GLog log) {
		
		// What this does:
		// Parse GameLevel.txt 
		// For each model found in the file...
			// Create a new Model class on the stack.
				// Read matrix transform and add to this model.
				// Load all CPU rendering data for this model from .h2b
			// Move the newly found Model to our list of total models for the level 

		log.LogCategorized("EVENT", "LOADING GAME LEVEL [OBJECT ORIENTED]");
		log.LogCategorized("MESSAGE", "Begin Reading Game Level Text File.");

		UnloadLevel();// clear previous level data if there is any
		GW::SYSTEM::GFile file;
		file.Create();
		if (-file.OpenTextRead(gameLevelPath)) {
			log.LogCategorized(
				"ERROR", (std::string("Game level not found: ") + gameLevelPath).c_str());
			return false;
		}
		char linebuffer[1024];
		while (+file.ReadLine(linebuffer, 1024, '\n'))
		{
			// having to have this is a bug, need to have Read/ReadLine return failure at EOF
			if (linebuffer[0] == '\0')
				break;
			if (std::strcmp(linebuffer, "MESH") == 0)
			{
				Model newModel;
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
				// create the model file name from this (strip the .001)
				newModel.SetName(linebuffer);
				std::string modelFile = linebuffer;
				modelFile = modelFile.substr(0, modelFile.find_last_of("."));
				modelFile += ".h2b";

				// now read the transform data as we will need that regardless
				GW::MATH::GMATRIXF transform;
				for (int i = 0; i < 4; ++i) {
					file.ReadLine(linebuffer, 1024, '\n');
					// read floats
					std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
						&transform.data[0 + i * 4], &transform.data[1 + i * 4],
						&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
				}
				std::string loc = "Location: X ";
				loc += std::to_string(transform.row4.x) + " Y " +
					std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
				log.LogCategorized("INFO", loc.c_str());

				// Add new model to list of all Models
				log.LogCategorized("MESSAGE", "Begin Importing .H2B File Data.");
				modelFile = std::string(h2bFolderPath) + "/" + modelFile;
				newModel.SetWorldMatrix(transform);
				// If we find and load it add it to the level
				if (newModel.LoadModelDataFromDisk(modelFile.c_str())) {
					// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
					allObjectsInLevel.push_back(std::move(newModel));
					log.LogCategorized("INFO", (std::string("H2B Imported: ") + modelFile).c_str());
				}
				else {
					// notify user that a model file is missing but continue loading
					log.LogCategorized("ERROR",
						(std::string("H2B Not Found: ") + modelFile).c_str());
					log.LogCategorized("WARNING", "Loading will continue but model(s) are missing.");
				}
				log.LogCategorized("MESSAGE", "Importing of .H2B File Data Complete.");
			}
		}
		log.LogCategorized("MESSAGE", "Game Level File Reading Complete.");
		// level loaded into CPU ram
		log.LogCategorized("EVENT", "GAME LEVEL WAS LOADED TO CPU [OBJECT ORIENTED]");
		return true;
	}
	// Upload the CPU level to GPU
	void UploadLevelToGPU(GW::SYSTEM::GWindow win, GW::GRAPHICS::GDirectX11Surface d3d) {
		// iterate over each model and tell it to draw itself
		for (auto& e : allObjectsInLevel) {
			e.UploadModelData2GPU(win, d3d);
		}
	}
	// Draws all objects in the level
	void RenderLevel(GW::SYSTEM::GWindow win, GW::GRAPHICS::GDirectX11Surface d3d) {
		// iterate over each model and tell it to draw itself
		for (auto &e : allObjectsInLevel) {
			e.DrawModel(win, d3d);
		}

		//auto it = allObjectsInLevel.begin();
		//std::advance(it, 1); // Move the iterator to the second element

		//// Draw the second object
		//it->DrawModel(win, d3d);
	}
	// used to wipe CPU & GPU level data between levels
	void UnloadLevel() {
		allObjectsInLevel.clear();
	}
	// *THIS APPROACH COMBINES DATA & LOGIC* 
	// *WITH THIS APPROACH THE CURRENT RENDERER SHOULD BE JUST AN API MANAGER CLASS*
	// *ALL ACTUAL GPU LOADING AND RENDERING SHOULD BE HANDLED BY THE MODEL CLASS* 
	// For example: anything that is not a global API object should be encapsulated.

};

