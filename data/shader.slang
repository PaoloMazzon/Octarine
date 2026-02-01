// Per-drawcall values
struct Push {
    // Not needed in pixel shader
    int cameraIndex;

    // Index of the texture to draw
    uint textureIndex;

    // Not needed in pixel shader
    float4 texturePos;

    // Current colour modifier
    float4 colour;

    // Not needed in pixel shader
    float4x4 model;
};
[[vk::push_constant]] Push push;

// Texture and sampler
[[vk::binding(1,1)]] SamplerState sampler;
[[vk::binding(2,2)]] Texture2D<float4> textures[];

// Optional user data
struct UserData {
    float time; // whatever you want
};
[[vk::binding(3,3)]] ConstantBuffer<UserData> userData;

[shader("pixel")]
float4 PixelShader(float2 uv) {
    float a = 1;
    float b = 0.3;
    float val1 = ((a - b) / 2) * sin(((uv.y + uv.x) / 2) + (userData.time)) + (((a - b) / 2) + b);
    float val2 = ((a - b) / 2) * sin(((uv.y + uv.x) / 2) + (userData.time + (3.141592635 * 0.25))) + (((a - b) / 2) + b);
    float val3 = ((a - b) / 2) * sin(((uv.y + uv.x) / 2) + (userData.time + (3.141592635 * 0.5))) + (((a - b) / 2) + b);
    float4 uv_colour = textures[push.textureIndex].Sample(sampler, uv);
    return float4((0.5 * uv_colour.r) * push.colour.r * (val1 * 2),
                  (0.5 * uv_colour.g) * push.colour.g * (val2 * 2),
                  (0.5 * uv_colour.b) * push.colour.b * (val3 * 2),
                  (uv_colour.a) * push.colour.a);
}
