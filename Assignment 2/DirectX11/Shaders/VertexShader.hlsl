// an ultra simple hlsl vertex shader
#pragma pack_matrix(row_major)

// TODO: Part 1F 
// TODO: Part 1H 
// TODO: Part 2B 
// TODO: Part 2D 
// TODO: Part 4A 
// TODO: Part 4B 

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
    //output.position = float4(inputVertex.position, 1.0f);
    // temporary logo position
    output.position = float4(inputVertex.position.x, inputVertex.position.y - 0.75f, inputVertex.position.z + 0.75f, 1.0f);
    return output;
}