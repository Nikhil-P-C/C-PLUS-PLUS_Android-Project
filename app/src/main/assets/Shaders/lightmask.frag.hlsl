cbuffer Context : register(b0, space3) {
    float2 center;    // light center, normalized 0..1 screen UV
    float radius;     // inner radius (full brightness), in UV units relative to screen height
    float softness;   // extra UV distance over which it fades to full darkness
    float darkness;   // max alpha (0..1) of the dark overlay far from the light
    float aspect;      // screenWidth / screenHeight, keeps the falloff circular not elliptical
    float2 _pad;
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

    float2 d = input.v_uv - center;
    d.x *= aspect; // undo aspect stretch so the falloff is a circle, not an ellipse

    float dist = length(d);
    float t = smoothstep(radius, radius + softness, dist);
    float alpha = saturate(t * darkness);

    output.o_color = float4(0.0, 0.0, 0.0, alpha);
    return output;
}
