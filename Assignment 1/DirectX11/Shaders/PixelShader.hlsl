// an ultra simple hlsl pixel shader
//float4 main() : SV_TARGET
//{
//	return float4(160/255.0f, 82/255.0f, 45/255.0f, 0); // TODO: Part 1A (optional) 
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
        VECTOR Ke;
        unsigned int
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


float4 main(float4 posH : SV_POSITION, float3 posW : WORLD, float3 normW : NORMAL) : SV_TARGET
{
    //// calculate lighting using lambertian shading
    //float lightRatio = saturate(dot(-sunDirection.xyz, normalize(normW)));
    ////add ambient lighting
    //lightRatio = saturate(lightRatio + sunAmbient);
    //float3 result = lightRatio * sunColor.xyz * material.Kd;
    
    //// calculate specular reflection
    //float3 viewDir = normalize(mul(cameraPos.xyz, posW) - posW);
    //float3 halfVector = normalize((-sunDirection.xyz) + viewDir);
    //float intensity = dot(normalize(normW), halfVector);
    //intensity = clamp(intensity, 0, 1);
    //intensity = pow(intensity, (material.Ns + 0.000001f));
    //intensity = max(intensity, 0);

    //float3 reflectedLight = sunColor.xyz * material.Ks * intensity;
    //result = result + reflectedLight;

    //return float4(result, 1.0f);
    
    return float4(material.Kd.x, material.Kd.y, material.Kd.z, 0);
}