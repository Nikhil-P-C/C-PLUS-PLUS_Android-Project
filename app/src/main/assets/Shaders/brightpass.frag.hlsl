cbuffer Context : register(b0, space3) {
    float threshold;
    float3 _pad;
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

PSOutput main(PSInput input) {
    PSOutput output;
    float4 color = u_texture.Sample(u_sampler, input.v_uv) * input.v_color;
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    float contribution = smoothstep(threshold, threshold + 0.2, luminance);
    output.o_color = float4(color.rgb * contribution, color.a);
    return output;
}