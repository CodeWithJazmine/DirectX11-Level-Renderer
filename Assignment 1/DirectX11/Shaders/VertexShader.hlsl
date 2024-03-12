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
};

float4 main(float4 inputVertex : POSITION) : SV_POSITION
{
    
    float4 transformedVertex = mul(inputVertex, world);
    return transformedVertex;
}

