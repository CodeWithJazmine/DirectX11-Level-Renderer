#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib") //needed for runtime shader compilation. Consider compiling shaders before runtime 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 1C 
struct VERTEX
{
	float x, y, z, w;
};
// TODO: Part 2B 
struct SceneData
{
	GW::MATH::GMATRIXF worldMatrix;
// TODO: Part 2G 
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
};

struct MeshData
{
	H2B::MATERIAL material;
};

class Renderer
{
	Level_Data& level;

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		meshConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// TODO: Part 2A 
	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMatrix matrixProxy;

	// TODO: Part 2C 
	SceneData sceneData;
	MeshData meshData;

	// TODO: Part 2D 
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	// TODO: Part 2G 
	GW::MATH::GMATRIXF viewMatrix;

	// TODO: Part 3A 
	GW::MATH::GMATRIXF projectionMatrix;

	// TODO: Part 3C 
	std::vector<GW::MATH::GMATRIXF> worldMatricesForCube;
	GW::MATH::GMATRIXF worldMatrixFront;
	GW::MATH::GMATRIXF worldMatrixBack;
	GW::MATH::GMATRIXF worldMatrixLeft;
	GW::MATH::GMATRIXF worldMatrixRight;
	GW::MATH::GMATRIXF worldMatrixCeiling;

	// TODO: Part 4A 
	GW::INPUT::GInput inputProxy;
	GW::INPUT::GController controllerProxy;

	std::chrono::steady_clock::time_point timePassed;

	GW::MATH::GMATRIXF cameraMatrix;
	GW::MATH::GMATRIXF originalCameraMatrix;
	GW::MATH::GMATRIXF savedCameraMatrix;
	GW::MATH::GMATRIXF translationMatrix;
	GW::MATH::GMATRIXF pitchMatrix;
	GW::MATH::GMATRIXF yawMatrix;

	unsigned int screenHeight, screenWidth;
	float screenAspectRatio;

	float prevMouseX,
		prevMouseY;


public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d, Level_Data& _lvl) : level(_lvl)
	{
		win = _win;
		d3d = _d3d;
		// TODO: Part 2A 
		matrixProxy.Create();
		// TODO: Part 4A 
		inputProxy.Create(_win);
		controllerProxy.Create();

		// TODO: Part 2C 
		InitializeWorldMatrix();
		sceneData.worldMatrix = worldMatrix;

		// TODO: Part 2G 
		InitializeViewMatrix();
		sceneData.viewMatrix = viewMatrix;

		// TODO: Part 3A 
		InitializeProjectionMatrix();
		// TODO: Part 3B 
		sceneData.projectionMatrix = projectionMatrix;

		// TODO: Part 3C 
		//InitializeWorldMatricesForCube();
		
		InitializeMeshData();

		timePassed = std::chrono::steady_clock::now();
		inputProxy.GetMousePosition(prevMouseX, prevMouseY);
		
		InitializeGraphics();
	}
private:
	//Constructor helper functions 
	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		
		InitializeVertexBuffer(creator);
		InitializeIndexBuffer(creator);
		//TODO: Part 2D 
		InitializeConstantBuffer(creator, &sceneData);
		InitializeMeshConstantBuffer(creator, &meshData);
		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1B 
		// TODO: Part 1C 
		// TODO: Part 1D

		/*std::vector<VERTEX> verts;
		BuildGrid(verts);
		CreateVertexBuffer(creator, verts.data(), sizeof(VERTEX) * verts.size());*/
		CreateVertexBuffer(creator, level.levelVertices.data(), sizeof(H2B::VERTEX) * level.levelVertices.size());
	}	
	void InitializeIndexBuffer(ID3D11Device* creator)
	{
		CreateIndexBuffer(creator, level.levelIndices.data(), sizeof(unsigned int) * level.levelIndices.size());
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA iData = { data, 0, 0 };
		CD3D11_BUFFER_DESC iDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&iDesc, &iData, indexBuffer.GetAddressOf());
	}

	void InitializeConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA cbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC cbDesc(sizeof(SceneData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&cbDesc, &cbData, constantBuffer.GetAddressOf());
	}
	
	void InitializeMeshConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA mcbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC mcbDesc(sizeof(MeshData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&mcbDesc, &mcbData, meshConstantBuffer.GetAddressOf());
	}

	void InitializePipeline(ID3D11Device* creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);
		CreateVertexInputLayout(creator, vsBlob);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
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

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
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

	void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
	{
		// TODO: Part 1C 
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

	void InitializeMeshData()
	{
		meshData.material = level.levelMaterials[0];
	}
	void BuildGrid(std::vector<VERTEX>& verts)
	{
		float spacing = 1.0f / 25; // Determine spacing between grid lines

		// Build horizontal lines
		for (int i = 0; i <= 25; ++i)
		{
			float y = -0.5f + spacing * i;
			verts.push_back({ -0.5f, y, 0.0f, 1.0f }); // Left point
			verts.push_back({ 0.5f, y, 0.0f, 1.0f });  // Right point
		}

		// Build vertical lines
		for (int i = 0; i <= 25; ++i)
		{
			float x = -0.5f + spacing * i;
			verts.push_back({ x, -0.5f, 0.0f, 1.0f }); // Bottom point
			verts.push_back({ x, 0.5f, 0.0f, 1.0f });  // Top point
		}
	}

	void InitializeWorldMatrix()
	{
		matrixProxy.IdentityF(worldMatrix);
		// Rotate matrix 90 degrees about the x axis
		matrixProxy.RotateXGlobalF(worldMatrix, G_DEGREE_TO_RADIAN_F(90.0f), worldMatrix);

		// Translate matrix down the y axis by 0.5f units
		GW::MATH::GVECTORF translationVector = { 0.0f, -0.5f };
		matrixProxy.TranslateGlobalF(worldMatrix, translationVector, worldMatrix);
	}

	void InitializeWorldMatricesForCube()
	{

		// Initialize matrices for each side of cube to identity
		matrixProxy.IdentityF(worldMatrixFront);
		matrixProxy.IdentityF(worldMatrixBack);
		matrixProxy.IdentityF(worldMatrixLeft);
		matrixProxy.IdentityF(worldMatrixRight);
		matrixProxy.IdentityF(worldMatrixCeiling);

		// Perform transformations for each matrix in the vector
		GW::MATH::GVECTORF translationVector;
		// Front wall
		translationVector = { 0.0f, 0.0f, 0.5f };
		matrixProxy.TranslateGlobalF(worldMatrixFront, translationVector, worldMatrixFront);

		// Back wall
		translationVector = { 0.0f, 0.0f, -0.5f };
		matrixProxy.TranslateGlobalF(worldMatrixBack, translationVector, worldMatrixBack);

		// Left wall
		translationVector = { -0.5f, 0.0f, 0.0f };
		matrixProxy.RotateYGlobalF(worldMatrixLeft, G_DEGREE_TO_RADIAN_F(90.0f), worldMatrixLeft);
		matrixProxy.TranslateGlobalF(worldMatrixLeft, translationVector, worldMatrixLeft);

		// Right wall
		translationVector = { 0.5f, 0.0f, 0.0f };
		matrixProxy.RotateYGlobalF(worldMatrixRight, G_DEGREE_TO_RADIAN_F(90.0f), worldMatrixRight);
		matrixProxy.TranslateGlobalF(worldMatrixRight, translationVector, worldMatrixRight);

		// Ceiling
		translationVector = { 0.0f, 0.5f, 0.0f };
		matrixProxy.RotateXGlobalF(worldMatrixCeiling, G_DEGREE_TO_RADIAN_F(90.0f), worldMatrixCeiling);
		matrixProxy.TranslateGlobalF(worldMatrixCeiling, translationVector, worldMatrixCeiling);


		worldMatricesForCube.push_back(worldMatrix);
		worldMatricesForCube.push_back(worldMatrixFront);
		worldMatricesForCube.push_back(worldMatrixBack);
		worldMatricesForCube.push_back(worldMatrixLeft);
		worldMatricesForCube.push_back(worldMatrixRight);
		worldMatricesForCube.push_back(worldMatrixCeiling);
	}


	void InitializeViewMatrix()
	{
		GW::MATH::GVECTORF eyePos = { 0.25f, -0.125f, -0.25f };
		GW::MATH::GVECTORF lookAtPos = { 0.0f, 0.0f, 0.0f };
		GW::MATH::GVECTORF upPos = { 0.0f, 1.0f, 0.0f };

		matrixProxy.LookAtLHF(eyePos, lookAtPos, upPos, viewMatrix);
	}

	void InitializeProjectionMatrix()
	{
		float fov = G2D_DEGREE_TO_RADIAN_F(65.0f);
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		float nearPlane = 0.1f;
		float farPlane = 100.0f;

		matrixProxy.ProjectionDirectXLHF(fov, aspectRatio, nearPlane, farPlane, projectionMatrix);
	}

	void CreateWorldMatrices()
	{

	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);
		// TODO: Part 1B 
		// TODO: Part 1D 
		// TODO: Part 3D 
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		// Draw an instance of every object
		/*for (const auto& object : level.blenderObjects)
		{
			curHandles.context->DrawIndexedInstanced(level.levelModels[object.modelIndex].indexCount, 1, level.levelModels[object.modelIndex].indexStart, level.levelModels[object.modelIndex].vertexStart, object.modelIndex);
		}*/
		
		//// For testing purposes, draw one object
		//meshData.material.attrib = level.levelMaterials[0].attrib;

		//curHandles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//memcpy(mappedResource.pData, &meshData, sizeof(MeshData));
		//curHandles.context->Unmap(meshConstantBuffer.Get(), 0);

		//curHandles.context->DrawIndexedInstanced(level.levelModels[0].indexCount, 1, level.levelModels[0].indexStart, level.levelModels[0].vertexStart, 0);


		for (const auto& model : level.levelModels)
		{
			for (int msh = model.meshStart; msh < model.meshCount + model.materialStart; ++msh)
			{
				auto& mesh = level.levelMeshes[msh];
				curHandles.context->DrawIndexedInstanced(mesh.drawInfo.indexCount, 1, model.indexStart + mesh.drawInfo.indexOffset, model.vertexStart, 0);
			}
		}

		ReleasePipelineHandles(curHandles);
	}

	 //TODO: Part 4B 
	void UpdateCamera()
	{
		// Get delta time (the time passed since last function call)
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - timePassed).count();
		timePassed = currentTime;

		matrixProxy.IdentityF(originalCameraMatrix);
		matrixProxy.IdentityF(savedCameraMatrix);

		// TODO: Part 4C 
		// Inverse viewMatrix to set it into world space
		matrixProxy.InverseF(viewMatrix, cameraMatrix);

		// TODO: Part 4D 
		float totalYChange = 0.0f,
			totalZChange = 0.0f,
			totalXChange = 0.0f,
			totalPitch = 0.0f,
			totalYaw = 0.0f;

		const float cameraSpeed = 0.3f;
		float perFrameSpeed = 0.0f;
		float thumbStickSpeed = 0.0f;
		float fov = G2D_DEGREE_TO_RADIAN_F(65.0f);

		// Define keyboard and mouse input states
		float spaceKeyState = 0.0f,
			leftShiftState = 0.0f,
			wKeyState = 0.0f,
			sKeyState = 0.0f,
			aKeyState = 0.0f,
			dKeyState = 0.0f,
			mouseXDelta = 0.0f,
			mouseYDelta = 0.0f;

		float mouseX = 0.0f,
			mouseY = 0.0f;

		// Define controller input states
		float rightTriggerState = 0.0f,
			leftTriggerState = 0.0f,
			leftStickYAxisState = 0.0f,
			leftStickXAxisState = 0.0f,
			rightStickYAxisState = 0.0f,
			rightStickXAxisState = 0.0f;  

		// Get window size information
		screenHeight, screenWidth = 0;
		screenAspectRatio = 0.0f;
		win.GetHeight(screenHeight);
		win.GetWidth(screenWidth);
		d3d.GetAspectRatio(screenAspectRatio);

		// Read keyboard input states
  		inputProxy.GetState(G_KEY_SPACE, spaceKeyState);
		inputProxy.GetState(G_KEY_LEFTSHIFT, leftShiftState);
		inputProxy.GetState(G_KEY_W, wKeyState);
		inputProxy.GetState(G_KEY_S, sKeyState);
		inputProxy.GetState(G_KEY_A, aKeyState);
		inputProxy.GetState(G_KEY_D, dKeyState);
		inputProxy.GetMousePosition(mouseX, mouseY);
		mouseXDelta = mouseX - prevMouseX;
		mouseYDelta = mouseY - prevMouseY;
		prevMouseX = mouseX;
		prevMouseY = mouseY;
	
		// Read controller input states
		controllerProxy.GetState(0, G_RIGHT_TRIGGER_AXIS, rightTriggerState);
		controllerProxy.GetState(0, G_LEFT_TRIGGER_AXIS, leftTriggerState);
		controllerProxy.GetState(0, G_LY_AXIS, leftStickYAxisState);
		controllerProxy.GetState(0, G_LX_AXIS, leftStickXAxisState);
		controllerProxy.GetState(0, G_RY_AXIS, rightStickYAxisState);
		controllerProxy.GetState(0, G_RX_AXIS, rightStickXAxisState);

		// Update vertical movements
 		totalYChange = spaceKeyState - leftShiftState + rightTriggerState - leftTriggerState;
		matrixProxy.TranslateGlobalF(cameraMatrix, GW::MATH::GVECTORF{ 0, totalYChange * cameraSpeed * deltaTime, 0, 1 }, cameraMatrix);

		// TODO: Part 4E 
		// Update horizontal movements
		perFrameSpeed = cameraSpeed * deltaTime;
		totalZChange = wKeyState - sKeyState + leftStickYAxisState;
		totalXChange = dKeyState - aKeyState + leftStickXAxisState;
		// A direction vector for the camera's local space
		GW::MATH::GVECTORF localMoveDirection = { totalXChange, 0.0f, totalZChange, 1.0f };
		// Transform the direction vector from local space to world space
		GW::MATH::GVector::TransformF(localMoveDirection, cameraMatrix, localMoveDirection);
		// Translate the camera in the direction of the transformed vector
		matrixProxy.TranslateGlobalF(cameraMatrix, GW::MATH::GVECTORF{ localMoveDirection.x * perFrameSpeed, 0.0f, localMoveDirection.z * perFrameSpeed, 1.0f }, cameraMatrix);

		// TODO: Part 4F 
		// Update pitch movements
		thumbStickSpeed = G2D_PI * deltaTime;
		totalPitch = fov * mouseYDelta / screenHeight + rightStickYAxisState * thumbStickSpeed * -1;
		matrixProxy.RotateXLocalF(cameraMatrix, totalPitch, cameraMatrix);

		// TODO: Part 4G
		// Update yaw movements
		totalYaw = fov * screenAspectRatio * mouseXDelta / screenWidth + rightStickXAxisState * thumbStickSpeed;
		matrixProxy.RotateYGlobalF(cameraMatrix, totalYaw, cameraMatrix);

		// Return the cameraMatrix to view space
		// then send the new viewMatrix to the GPU
		matrixProxy.InverseF(cameraMatrix, viewMatrix);
		sceneData.viewMatrix = viewMatrix;

	}

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};
	//Render helper functions
	PipelineHandles GetCurrentPipelineHandles()
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
		//TODO: Part 2E 
		handles.context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf() );
		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //TODO: Part 1B 
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		// TODO: Part 1C 
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
		
		handles.context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

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
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet
	}
};
