cbuffer FrameConstant : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

struct VSInput
{
    float3 position : POSITIONT;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput o;
    float4 posV = mul(float4(input.position, 1.0f), view);
    o.position = mul(posV, projection);
    o.color = input.color;
    return o;
}

float4 PSMain(PSInput input) : SV_Target
{
    return input.color;
    
}