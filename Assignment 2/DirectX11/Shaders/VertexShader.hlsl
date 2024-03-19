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
    float3 normW : NORMAL;     // normal in worls space (for lighting)
};

cbuffer MeshData : register(b1)
{
    float4x4 worldMatrix;
    OBJ_ATTRIBUTES material;
};

cbuffer SceneData : register(b0)
{
    float4 sunColor;
    float3 sunDirection;
    int padding;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VERTEX_IN
{
    float3 position : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};

struct VERTEX_OUT
{
    float4 position : SV_POSITION;
};

VERTEX_OUT main(VERTEX_IN inputVertex)
{
    VERTEX_OUT output;
  
    output.position = float4(inputVertex.position, 1.0f);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    
    
    return output;
}