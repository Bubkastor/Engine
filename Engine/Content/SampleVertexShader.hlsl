struct VertexShaderInput
{
	float3 pos : POSITION;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;

	// For this lesson, set the vertex depth value to 0.5, so it is guaranteed to be drawn.
	vertexShaderOutput.pos = float4(input.pos, 1.0f);

	return vertexShaderOutput;
}