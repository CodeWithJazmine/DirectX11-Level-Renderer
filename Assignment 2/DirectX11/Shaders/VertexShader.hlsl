// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

// TODO: Part 1F 
// TODO: Part 1H 
// TODO: Part 2B 
// TODO: Part 2D 
// TODO: Part 4A 
// TODO: Part 4B
struct OBJ_ATTRIBUTES
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    uint illum; // illumination model
};

struct OutputToRasterizer
{
    float4 posH : SV_POSITION; // position in homogenous projection space
    float3 posW : WORLD;       // position in world space (for lighting)
    float3 normW : NORMAL;     // normal in world space (for lighting)
};

cbuffer SceneData : register(b0)
{
    float4 sunColor;
    float3 sunDirection;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer MeshData : register(b1)
{
    float4x4 worldMatrix;
    OBJ_ATTRIBUTES material;
};

struct VERTEX_IN
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

OutputToRasterizer main(VERTEX_IN inputVertex)
{
    OutputToRasterizer output;
  
    output.posH = float4(inputVertex.position, 1.0f);
    output.posH = mul(output.posH, viewMatrix);
    output.posH = mul(output.posH, projectionMatrix);
    
    
    output.normW = normalize(mul((float3x3)(worldMatrix), inputVertex.normal));
    output.posW = mul((float3x3) worldMatrix, inputVertex.normal);

    return output;
}