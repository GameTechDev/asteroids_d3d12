#include "sprite_vs.hlsl"

Texture2D Tex : register(t0);
sampler Sampler : register(s0);

float4 sprite_ps(Font_VSOut input) : SV_Target
{   
    return Tex.Sample(Sampler, input.uv);
}
