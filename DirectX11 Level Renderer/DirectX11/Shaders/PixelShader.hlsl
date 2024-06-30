// an ultra simple hlsl pixel shader

struct VECTOR
{
    float x, y, z;
};
struct VERTEX
{
    VECTOR pos, uvw, nrm;
};
struct ATTRIBUTES
{
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
    float4 cameraPosition;
};

float4 main(float4 posH : SV_POSITION, float3 posW : WORLD, float3 normW : NORMAL) : SV_TARGET
{
    // calculate lighting using lambertian shading
    float lightRatio = saturate(dot(-sunDirection.xyz, normalize(normW)));
    //add ambient lighting
    lightRatio = saturate(lightRatio + sunAmbient);
    float3 result = lightRatio * sunColor.xyz * material.Kd;
    
    //calculate specular reflection
    float3 viewDir = normalize(cameraPosition.xyz - posW);
    float3 halfVector = normalize((-sunDirection.xyz) + viewDir);
    float intensity = max(pow(saturate(dot(normalize(normW), halfVector)), (material.Ns + 0.000001f)), 0);
    
    float3 reflectedLight = sunColor.xyz * material.Ks * intensity;

    result = result + reflectedLight;
    
    
    return float4(result, 1.0f);
}


