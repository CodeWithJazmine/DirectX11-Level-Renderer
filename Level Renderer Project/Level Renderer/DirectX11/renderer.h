#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#include "../Assets/FSLogo.h"
#pragma comment(lib, "d3dcompiler.lib") 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 2B 
struct SceneData
{
	GW::MATH::GVECTORF sunDirection, sunColor; // lighting info
	GW::MATH::GMATRIXF viewMatrix, projectionMatrix;
// TODO: Part 4E 
	GW::MATH::GVECTORF sunAmbient, cameraPos;

};

struct MeshData
{
	GW::MATH::GMATRIXF worldMatrix; // world space transformation
	OBJ_ATTRIBUTES material; // material info (color, reflectivity, emissiveness, etc)
};

// Creation, Rendering & Cleanup
class Renderer
{
	// handle to renderer
	//Level_Data& level;

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		meshConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// TODO: Part 2A 
	GW::MATH::GMatrix  matrixProxy;
	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
	GW::MATH::GMATRIXF rotationMatrix;

	GW::MATH::GVector  vectorProxy;
	GW::MATH::GVECTORF lightDirection;
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF sunAmbient;

	// TODO: Part 2B 
	SceneData sceneData;
	MeshData meshData;
	

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d/*, Level_Data& _lvl*/)
	{
		win = _win;
		d3d = _d3d;

		matrixProxy.Create();
		vectorProxy.Create();

		// TODO: Part 2A 
		InitializeMatrices();
		InitializeLight();

		// TODO: Part 2B 
		InitializeSceneData();
		InitializeMeshData();

		// TODO: Part 4E 
		IntializeGraphics();

	}

private:
	//constructor helper functions
	void IntializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);
		// TODO: Part 1G 
		InitializeIndexBuffer(creator);
		
		// TODO: Part 2C 
		InitializeSceneConstantBuffer(creator, &sceneData);
		InitializeMeshConstantBuffer(creator, &meshData);

		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1C 
		CreateVertexBuffer(creator, &FSLogo_vertices[0], sizeof(OBJ_VERT) * FSLogo_vertexcount);
	}
	
	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}
	void InitializeIndexBuffer(ID3D11Device* creator)
	{
		CreateIndexBuffer(creator, &FSLogo_indices[0], sizeof(UINT) * FSLogo_indexcount);
	}

	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, indexBuffer.GetAddressOf());
	}

	void InitializeSceneConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA cbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC cbDesc(sizeof(SceneData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&cbDesc, &cbData, sceneConstantBuffer.GetAddressOf());
	}

	void InitializeMeshConstantBuffer(ID3D11Device* creator, const void* data)
	{
		D3D11_SUBRESOURCE_DATA cbData = { data, 0, 0 };
		CD3D11_BUFFER_DESC cbDesc(sizeof(MeshData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&cbDesc, &cbData, meshConstantBuffer.GetAddressOf());
	}

	void InitializeMatrices()
	{
		// World Matrix: An identity matrix that slowly rotates along the Y axis over time.
		matrixProxy.IdentityF(worldMatrix);
		matrixProxy.IdentityF(rotationMatrix);

		// View: A camera positioned at 0.75x +0.25y -1.5z that is rotated to look at +0.15x +0.75y +0z.
		matrixProxy.LookAtLHF(
			GW::MATH::GVECTORF{ 0.75f, 0.25f, -1.5f }, // Camera position
			GW::MATH::GVECTORF{ 0.15f, 0.75f, 0.0f }, // Look at position
			GW::MATH::GVECTORF{ 0.0f, 1.0f, 0.0f }, // Up direction
			viewMatrix ); 

		// Projection: A vertical field of view of 65 degrees, and a near and far plane of 0.1 and 100 respectively.
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		matrixProxy.ProjectionDirectXLHF(
			G2D_DEGREE_TO_RADIAN_F(65.0f),       // Field of view
			aspectRatio,						 // Aspect ratio
			0.1f,                                // Near plane
			100.0f,                              // Far plane
			projectionMatrix );
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
		sceneData.projectionMatrix = projectionMatrix;
		sceneData.viewMatrix = viewMatrix;
		sceneData.sunAmbient = sunAmbient;
		sceneData.cameraPos = viewMatrix.row1;
	}

	void InitializeMeshData()
	{
		meshData.worldMatrix = worldMatrix;
		meshData.material = FSLogo_materials[0].attrib;
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
		// TODO: Part 1E 
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
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);
		// TODO: Part 1H 
		// TODO: Part 3B 
		// TODO: Part 3C 
		// TODO: Part 4D
		D3D11_MAPPED_SUBRESOURCE mappedResource;
	
		// Loop through each mesh to draw separately
		for (int i = 0; i < 2; i++) {
			
			// update world matrix for rotation
			if (i == 0) // text
				meshData.worldMatrix = worldMatrix;
			else if (i == 1) // logo
				meshData.worldMatrix = rotationMatrix;

			// update material
			meshData.material = FSLogo_materials[i].attrib;

			// send updated mesh buffer to pixel shader
			curHandles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &meshData, sizeof(MeshData));
			curHandles.context->Unmap(meshConstantBuffer.Get(), 0);

			// draw the mesh
			curHandles.context->DrawIndexed(FSLogo_meshes[i].indexCount, FSLogo_meshes[i].indexOffset, 0);
		}

		ReleasePipelineHandles(curHandles);
	}

	

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

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
		const UINT strides[] = { sizeof(OBJ_VERT) }; // TODO: Part 1E 
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
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet 
	}

	void Update()
	{
		static auto start = std::chrono::steady_clock::now();
		double timePassed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
		double rotationSpeed = timePassed * 35.0f;
		matrixProxy.RotateYGlobalF(rotationMatrix, G2D_DEGREE_TO_RADIAN_F(rotationSpeed), rotationMatrix);
		start = std::chrono::steady_clock::now();
	}
};
