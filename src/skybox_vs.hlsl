cbuffer SkyboxConstantBuffer : register(b0)
{
    float4x4 mViewProjection;
};

struct Skybox_VSIn
{
	float3 position : POSITION;
	float3 uvFace   : UVFACE;
};

struct Skybox_VSOut
{
	float4 position : SV_Position;
	float3 coords   : UVFACE;
};

Skybox_VSOut skybox_vs(Skybox_VSIn input)
{
    Skybox_VSOut output;

    // NOTE: Don't translate skybox and make sure depth == 1 (no clipping)
    output.position = mul(mViewProjection, float4(input.position, 0.0f)).xyww;
    output.position.z = 0.0f; // 1-z
    output.coords = input.position.xyz;

    return output;
}
