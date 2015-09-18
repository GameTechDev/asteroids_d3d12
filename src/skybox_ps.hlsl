#include "skybox_vs.hlsl"

TextureCube Tex : register(t0);
sampler Sampler : register(s0);

float4 skybox_ps(Skybox_VSOut input) : SV_Target
{   
    //return input.coords.xyzx;
    float4 tex = Tex.Sample(Sampler, input.coords);
    return float4(tex.xyz, 1.0f);
}
