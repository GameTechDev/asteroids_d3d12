cbuffer DrawConstantBuffer : register(b0)
{
	float4x4 mWorld;
	float4x4 mViewProjection;
	float4 mSurfaceColor;
	float4 mDeepColor;
	uint mTextureIndex;
};

struct VSIn
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VSOut
{
	float4 position      : SV_Position;
	float3 positionModel : POSITIONMODEL;
	float3 normalWorld   : NORMAL;
	float3 albedo        : ALBEDO; // Alternatively, can pass just "ao" to PS and read cbuffer in PS
};

float linstep(float min, float max, float s)
{
    return saturate((s - min) / (max - min));
}


VSOut asteroid_vs(VSIn input)
{
    VSOut output;

    float3 positionWorld = mul(mWorld, float4(input.position, 1.0f)).xyz;
    output.position = mul(mViewProjection, float4(positionWorld, 1.0f));

    output.positionModel = input.position;
    output.normalWorld = mul(mWorld, float4(input.normal, 0.0f)).xyz; // No non-uniform scaling
    
    float depth = linstep(0.5f, 0.7f, length(input.position.xyz));
    output.albedo = lerp(mDeepColor.xyz, mSurfaceColor.xyz, depth);

    return output;
}
