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
struct SHADER_VARS
{
	GW::MATH::GMATRIXF worldMatrix;
// TODO: Part 2G 
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
};

class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// TODO: Part 2A 
	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMatrix matrixProxy;
	// TODO: Part 2C 
	SHADER_VARS shaderVars;
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


public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
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
		shaderVars.worldMatrix = worldMatrix;

		// TODO: Part 2G 
		InitializeViewMatrix();
		shaderVars.viewMatrix = viewMatrix;

		// TODO: Part 3A 
		InitializeProjectionMatrix();
		// TODO: Part 3B 
		shaderVars.projectionMatrix = projectionMatrix;

		// TODO: Part 3C 
		InitializeWorldMatricesForCube();
		
		InitializeGraphics();
	}

	void UpdateCamera()
	{
		
	}
private:
	//Constructor helper functions 
	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		
		InitializeVertexBuffer(creator);
		//TODO: Part 2D 
		InitializeConstantBuffer(creator, &shaderVars);
		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1B 
		// TODO: Part 1C 
		// TODO: Part 1D

		std::vector<VERTEX> verts;
		BuildGrid(verts);
		CreateVertexBuffer(creator, verts.data(), sizeof(VERTEX) * verts.size());
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
		

		worldMatricesForCube.push_back(worldMatrixFront);
		worldMatricesForCube.push_back(worldMatrixBack);
		worldMatricesForCube.push_back(worldMatrixLeft);
		worldMatricesForCube.push_back(worldMatrixRight);
		worldMatricesForCube.push_back(worldMatrixCeiling);
		worldMatricesForCube.push_back(worldMatrix);
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
		float fov = 65.0f;
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		float nearPlane = 0.1f;
		float farPlane = 100.0f;

		matrixProxy.ProjectionDirectXLHF(fov, aspectRatio, nearPlane, farPlane, projectionMatrix);
	}


	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	void InitializeConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA cbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC cbDesc(sizeof(SHADER_VARS), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&cbDesc, &cbData, constantBuffer.GetAddressOf());
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
		D3D11_INPUT_ELEMENT_DESC attributes[1];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
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

		for (size_t i = 0; i < worldMatricesForCube.size(); i++)
		{
			curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			shaderVars.worldMatrix = worldMatricesForCube[i];
			memcpy(mappedResource.pData, &shaderVars, sizeof(SHADER_VARS));
			curHandles.context->Unmap(constantBuffer.Get(), 0);
			curHandles.context->Draw(104, 0);
		}

		ReleasePipelineHandles(curHandles);
	}

	// TODO: Part 4B 
	// TODO: Part 4C 
	// TODO: Part 4D 
	// TODO: Part 4E 
	// TODO: Part 4F 
	// TODO: Part 4G 

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
		SetShaders(handles);
		//TODO: Part 2E 
		handles.context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf() );
		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); //TODO: Part 1B 
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		// TODO: Part 1C 
		const UINT strides[] = { sizeof(VERTEX) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
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
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet
	}
};
