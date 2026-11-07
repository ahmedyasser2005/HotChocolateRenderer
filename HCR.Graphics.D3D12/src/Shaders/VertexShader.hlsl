struct VSIn
{
    float3 pos : POSITION;
    float3 col : COLOR;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float3 col : COLOR;
};

struct Transformation
{
    matrix WVP;
};
ConstantBuffer<Transformation> transformation : register(b0);

VSOut main(VSIn vsin)
{
    VSOut vsout;
    vsout.pos = mul(float4(vsin.pos, 1.0f), transformation.WVP);
    vsout.col = vsin.col;
    return vsout;
}