// an ultra simple hlsl pixel shader
// TODO: Part 3A 
// TODO: Part 4B 
// TODO: Part 4C 
// TODO: Part 4F 

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

cbuffer SceneData : register(b0)
{
    float4 sunColor;
    float3 sunDirection;
    int padding;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer MeshData : register(b1)
{
    float4x4 worldMatrix;
    OBJ_ATTRIBUTES material;
};



float4 main() : SV_TARGET
{
    //return float4(160 / 255.0f, 82 / 255.0f, 45 / 255.0f, 0); // TODO: Part 1A (optional) 
    return float4(material.Kd, 1.0f);
}