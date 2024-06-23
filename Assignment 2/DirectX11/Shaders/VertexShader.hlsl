// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

struct VECTOR
{
    float x, y, z;
};
struct VERTEX
{
    VECTOR pos, uvw, nrm;
};
struct ATTRIBUTES {
    float3 Kd;
    float d;
    float3 Ks;
    float Ns;
    float3 Ka;
    float sharpness;
    float3 Tf;
    float Ni;
    float3 Ke;
    unsigned int illum;
};

cbuffer SceneData : register(b0)
{
    float4 sunDirection;
    float4 sunColor;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 sunAmbient;
    float4 cameraPos;
};

cbuffer MeshData : register(b1)
{
    float4x4 worldMatrix;
    ATTRIBUTES material;
};

cbuffer SHADER_VARS : register(b2)
{
    float4x4 cworldMatrix;
    float4x4 cviewMatrix;
    float4x4 cprojectionMatrix;
};

struct OutputToRasterizer
{
    float4 posH : SV_POSITION; // position in homogenous projection space
    float3 posW : WORLD;       // position in world space (for lighting)
    float3 normW : NORMAL;     // normal in world space (for lighting)
};

struct VERTEX_IN
{
    float3 position : POSITION;
    float3 uv : UV;
    float3 normal : NORMAL;
};

OutputToRasterizer main(VERTEX_IN inputVertex)
{
    OutputToRasterizer output;
    
    output.posH = float4(inputVertex.position, 1.0f);
    //output.posH = mul(output.posH, worldMatrix);
    //output.posH = mul(output.posH, viewMatrix);
    //output.posH = mul(output.posH, projectionMatrix);
    output.posH = mul(output.posH, worldMatrix);
    output.posH = mul(output.posH, cviewMatrix);
    output.posH = mul(output.posH, cprojectionMatrix);
    
    output.posW = mul(inputVertex.position, (float3x3) worldMatrix);
    output.normW = mul(inputVertex.normal, (float3x3) worldMatrix);
    
    
    return output;
}