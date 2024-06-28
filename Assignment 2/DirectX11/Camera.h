// Camera Controls
struct VERTEX
{
	float x, y, z, w;
};

struct SHADER_VARS
{
	GW::MATH::GMATRIXF worldMatrix;

	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
	GW::MATH::GVECTORF cameraPosition;
};

struct MinimapShaderVars
{
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;

};
class Camera
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;


	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;


	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMatrix matrixProxy;
	GW::MATH::GVector vectorProxy;

	SHADER_VARS shaderVars;
	MinimapShaderVars minimapShaderVars;


	GW::MATH::GMATRIXF viewMatrix;

	GW::MATH::GMATRIXF projectionMatrix;


	GW::INPUT::GInput inputProxy;
	GW::INPUT::GController controllerProxy;

	std::chrono::steady_clock::time_point timePassed;
	GW::MATH::GVECTORF translationVector;
	GW::MATH::GMATRIXF cameraMatrix;
	GW::MATH::GMATRIXF pitchMatrix;
	GW::MATH::GMATRIXF yawMatrix;

	unsigned int screenHeight, screenWidth;
	float screenAspectRatio;

	float prevMouseX,
		prevMouseY;

	int cameraState;

	// For minimap
	GW::MATH::GMATRIXF minimapViewMatrix;
	GW::MATH::GMATRIXF minimapProjectionMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> minimapConstantBuffer;


public:
	Camera(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		
		// default camera state
		cameraState = 0;

		matrixProxy.Create();
		vectorProxy.Create();

		inputProxy.Create(_win);
		controllerProxy.Create();

		shaderVars.worldMatrix = worldMatrix;

		InitializeViewMatrix();
		InitializeProjectionMatrix();

		InitializeMiniMapViewMatrix();
		InitializeMiniMapProjectionMatrix();

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

		InitializeConstantBuffer(creator, &shaderVars);
		InitializeMiniMapConstantBuffer(creator, &minimapShaderVars);

		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeViewMatrix()
	{

		GW::MATH::GVECTORF eyePos = { 0.0f, 1.5f, -1.0f };
		GW::MATH::GVECTORF lookAtPos = { 0.0f, 1.5f, 0.0f };
		GW::MATH::GVECTORF upPos = { 0.0f, 1.0f, 0.0f };

		matrixProxy.LookAtLHF(eyePos, lookAtPos, upPos, viewMatrix);
		shaderVars.viewMatrix = viewMatrix;
	}

	void InitializeProjectionMatrix()
	{
		float fov = G2D_DEGREE_TO_RADIAN_F(65.0f);
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		float nearPlane = 0.1f;
		float farPlane = 100.0f;

		matrixProxy.ProjectionDirectXLHF(fov, aspectRatio, nearPlane, farPlane, projectionMatrix);
		shaderVars.projectionMatrix = projectionMatrix;
	}

	void InitializeConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA cbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC cbDesc(sizeof(SHADER_VARS), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&cbDesc, &cbData, constantBuffer.GetAddressOf());
	}

	// === Minimap ===
	void InitializeMiniMapViewMatrix()
	{
		// temporary values
		GW::MATH::GVECTORF eyePos = { 0.0f, 1.5f, -1.0f };
		GW::MATH::GVECTORF lookAtPos = { 0.0f, 1.5f, 0.0f };
		GW::MATH::GVECTORF upPos = { 0.0f, 1.0f, 0.0f };

		matrixProxy.LookAtLHF(eyePos, lookAtPos, upPos, minimapViewMatrix);
	}

	void InitializeMiniMapProjectionMatrix()
	{
		float fov = G2D_DEGREE_TO_RADIAN_F(65.0f);
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		float nearPlane = 0.1f;
		float farPlane = 100.0f;

		matrixProxy.ProjectionDirectXLHF(fov, aspectRatio, nearPlane, farPlane, minimapProjectionMatrix);
	}
	void InitializeMiniMapConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA mmcbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC mmcbDesc(sizeof(MinimapShaderVars), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&mmcbDesc, &mmcbData, minimapConstantBuffer.GetAddressOf());
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




public:
	void UpdateConstantBuffer()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;

		curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		memcpy(mappedSubresource.pData, &shaderVars, sizeof(SHADER_VARS));
		curHandles.context->Unmap(constantBuffer.Get(), 0);

		ReleasePipelineHandles(curHandles);
	}

	//TODO: Part 4B 
	void UpdateCamera()
	{
		// Get delta time (the time passed since last function call)
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - timePassed).count();
		timePassed = currentTime;

		// Inverse viewMatrix to set it into world space
		matrixProxy.InverseF(viewMatrix, cameraMatrix);

		float totalYChange = 0.0f,
			totalZChange = 0.0f,
			totalXChange = 0.0f,
			totalPitch = 0.0f,
			totalYaw = 0.0f;

		const float cameraSpeed = 3.0f;
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

		// Update pitch movements
		thumbStickSpeed = G2D_PI * deltaTime;
		totalPitch = fov * mouseYDelta / screenHeight + rightStickYAxisState * thumbStickSpeed * -1;
		matrixProxy.RotateXLocalF(cameraMatrix, totalPitch, cameraMatrix);

		// Update yaw movements
		totalYaw = fov * screenAspectRatio * mouseXDelta / screenWidth + rightStickXAxisState * thumbStickSpeed;
		matrixProxy.RotateYGlobalF(cameraMatrix, totalYaw, cameraMatrix);

		// Return the cameraMatrix to view space
		// then send the new viewMatrix to the GPU
		matrixProxy.InverseF(cameraMatrix, viewMatrix);
		shaderVars.viewMatrix = viewMatrix;

		shaderVars.worldMatrix = cameraMatrix;

		GW::MATH::GVECTORF cameraWorldPos = cameraMatrix.row4;
		shaderVars.cameraPosition = cameraWorldPos;


		/*std::cout << "Camera Position: " << cameraWorldPos.x << ", " << cameraWorldPos.y << ", " << cameraWorldPos.z << std::endl;
		std::cout << "Camera Look At: " << cameraMatrix.row3.x << ", " << cameraMatrix.row3.y << ", " << cameraMatrix.row3.z << std::endl;*/

	}

	bool SwitchCamera(int& cameraState)
	{
		std::cout << "Switching Camera\n";

		switch (cameraState) {
		case 0:
			// Set 1st camera position ( puzzel room camera )
		{
			GW::MATH::GVECTORF eyePos1 = { -9.99372, 4.25856, -17.2818 };
			GW::MATH::GVECTORF lookAtPos1 = { 0.649649, -0.177245, 0.739309 };
			GW::MATH::GVECTORF upPos1 = { 0.0f, 1.0f, 0.0f };
			matrixProxy.LookAtLHF(eyePos1, lookAtPos1, upPos1, viewMatrix);
			shaderVars.viewMatrix = viewMatrix;
			break;
		}

		case 1:
			// Set 2nd camera position ( library camera )
		{
			GW::MATH::GVECTORF eyePos2 = { 17.8372, 3.2878, -23.3044 };
			GW::MATH::GVECTORF lookAtPos2 = { -0.776143, -0.279638, 0.565186 };
			GW::MATH::GVECTORF upPos2 = { 0.0f, 1.0f, 0.0f };
			matrixProxy.LookAtLHF(eyePos2, lookAtPos2, upPos2, viewMatrix);
			shaderVars.viewMatrix = viewMatrix;
			break;
		}
		case 2:
		{
			// Set 3rd camera position ( original pos )
			GW::MATH::GVECTORF eyePos3 = { 0.0f, 1.5f, -1.0f };
			GW::MATH::GVECTORF lookAtPos3 = { 0.0f, 1.5f, 0.0f };
			GW::MATH::GVECTORF upPos3 = { 0.0f, 1.0f, 0.0f };
			matrixProxy.LookAtLHF(eyePos3, lookAtPos3, upPos3, viewMatrix);
			shaderVars.viewMatrix = viewMatrix;
			break;
		}

		default:
			// Handle invalid camera state or add more cases as needed
			std::cerr << "Invalid camera state\n";
			return false;
		}

		// Increment camera state for next switch
		cameraState = (cameraState + 1) % 3;

		SetCameraState(cameraState);

		return true;

	}
	const int GetCameraState()
	{
		return cameraState;
	}

	int SetCameraState(int _cameraState)
	{
		return cameraState = _cameraState;
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
		SetShaders(handles);
		
		handles.context->VSSetConstantBuffers(2, 1, constantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(2, 1, constantBuffer.GetAddressOf());
		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); 
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}


public:
	~Camera() {}
};