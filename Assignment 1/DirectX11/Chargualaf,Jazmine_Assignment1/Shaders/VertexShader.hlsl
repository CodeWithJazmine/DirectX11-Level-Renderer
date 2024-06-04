// an ultra simple hlsl vertex shader
// TODO: Part 1C 
// TODO: Part 2B 
// TODO: Part 2F 
#pragma pack_matrix(row_major)
// TODO: Part 2G 
// TODO: Part 3B 

cbuffer SHADER_VARS
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

float4 main(float4 inputVertex : POSITION) : SV_POSITION
{
    
    float4 transformedVertex = mul(inputVertex, world); // apply world matrix
    transformedVertex = mul(transformedVertex, view); // apply view matrix
    transformedVertex = mul(transformedVertex, projection); // applying projection
    return transformedVertex;
}

