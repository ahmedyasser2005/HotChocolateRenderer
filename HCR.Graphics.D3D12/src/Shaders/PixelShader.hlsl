struct PSIn
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
};

float4 main(PSIn psin) : SV_TARGET
{
    return float4(psin.col * 1.2, 1.0f);
}