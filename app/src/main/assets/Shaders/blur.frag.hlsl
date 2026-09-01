cbuffer Context : register(b0, space3) {
    float2 texelSize;  // 1.0/width, 1.0/height
    float2 direction;  // (1,0) horizontal, (0,1) vertical
};

Texture2D u_texture : register(t0, space2);
SamplerState u_sampler : register(s0, space2);

struct PSInput {
    float4 v_color : COLOR0;
    float2 v_uv : TEXCOORD0;
};
struct PSOutput {
    float4 o_color : SV_Target;
};

static const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

PSOutput main(PSInput input) {
    PSOutput output;
    float3 result = u_texture.Sample(u_sampler, input.v_uv).rgb * weights[0];
    [unroll]
    for (int i = 1; i < 5; ++i) {
        float2 offset = direction * texelSize * float(i);
        result += u_texture.Sample(u_sampler, input.v_uv + offset).rgb * weights[i];
        result += u_texture.Sample(u_sampler, input.v_uv - offset).rgb * weights[i];
    }
    output.o_color = float4(result, 1.0);
    return output;
}