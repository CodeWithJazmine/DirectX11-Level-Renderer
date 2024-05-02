// an ultra simple hlsl vertex shader
// TODO: Part 1C 
// TODO: Part 2B 
// TODO: Part 2F 
#pragma pack_matrix(row_major)
// TODO: Part 2G 
// TODO: Part 3B 

//cbuffer SHADER_VARS
//{
//    float4x4 world;
//    float4x4 view;
//    float4x4 projection;
//};

//float4 main(float4 inputVertex : POSITION) : SV_POSITION
//{
    
//    float4 transformedVertex = mul(inputVertex, world); // apply world matrix
//    transformedVertex = mul(transformedVertex, view); // apply view matrix
//    transformedVertex = mul(transformedVertex, projection); // applying projection
//    return transformedVertex;
//}

namespace H2B
{

#pragma pack(push,1)
    struct VECTOR
    {
        float x, y, z;
    };
    struct VERTEX
    {
        VECTOR pos, uvw, nrm;
    };
    struct ATTRIBUTES {
        VECTOR Kd;
        float d;
        VECTOR Ks;
        float Ns;
        VECTOR Ka;
        float sharpness;
        VECTOR Tf;
        float Ni;
        VECTOR Ke;unsigned int
        illum;
    };
    
}
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
        H2B::ATTRIBUTES material;
    };
    
    struct OutputToRasterizer
    {
        float4 posH : SV_POSITION; // position in homogenous projection space
        float3 posW : WORLD; // position in world space (for lighting)
        float3 normW : NORMAL; // normal in world space (for lighting)
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
    output.posH = mul(output.posH, worldMatrix);
    //output.posH = mul(output.posH, viewMatrix);
    //output.posH = mul(output.posH, projectionMatrix);
    
    //output.posW = mul(inputVertex.position, (float3x3) worldMatrix);
    //output.normW = mul(inputVertex.normal, (float3x3) worldMatrix);
    
    return output;
}
