#include "sprite_vs.hlsl"

Texture2D Tex : register(t0);
sampler Sampler : register(s0);

float4 font_ps(Font_VSOut input) : SV_Target
{   
    //return float4(input.uv.xyxy);
    float alpha = Tex.Sample(Sampler, input.uv).a;
    float4 color = float4(1, 1, 1, 1);
    return alpha * color;
}
