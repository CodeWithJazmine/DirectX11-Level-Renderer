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
    OBJ_ATTRIBUTES material;
};


float4 main(float4 posH : SV_POSITION, float3 posW : WORLD, float3 normW : NORMAL) : SV_TARGET
{
    // calculate lighting using lambertian shading
    float lightRatio = saturate(dot(-sunDirection.xyz, normalize(normW)));
    //add ambient lighting
    lightRatio = saturate(lightRatio + sunAmbient);
    float3 result = lightRatio * sunColor.xyz * material.Kd;
    
    // calculate specular reflection
    float3 viewDir = normalize(mul(cameraPos.xyz, posW) - posH.xyz);
    float3 halfVector = normalize((-sunDirection.xyz) + viewDir);
    float intensity = dot(normalize(normW), halfVector);
    intensity = clamp(intensity, 0, 1);
    intensity = pow(intensity, (material.Ns + 0.000001f));
    intensity = max(intensity, 0);

    float3 reflectedLight = sunColor.xyz * material.Ks * intensity;
    result = result + reflectedLight;

    return float4(result, 1.0f);
}

