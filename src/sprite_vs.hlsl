struct Font_VSIn
{
	float2 position : POSITION;
	float2 uv       : UV;
};

struct Font_VSOut
{
	float4 position : SV_Position;
	float2 uv       : UV;
};

Font_VSOut sprite_vs(Font_VSIn input)
{
    Font_VSOut output;
    output.position = float4(input.position, 0, 1);
    output.uv = input.uv;
    return output;
}
