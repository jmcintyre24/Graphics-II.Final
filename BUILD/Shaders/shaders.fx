// Constant Buffer Variables
Texture2D txDiffuse : register(t0); // t for shader resource view.
Texture2D meshTXDiffuse : register(t1); // t for shader resource view.
Texture2D crosshairTX : register(t3); // Crosshair Texture
SamplerState samLinear : register(s0); // s for samplers
TextureCube skybox : register(t2);

cbuffer ConstantBuffer : register(b0) // b for constant buffers
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir;
    float4 vLightColor;
    float4 vOutputColor;
    bool popped[3];
    float time;
}
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 worldPos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD1;
    float4 Color : COLOR;
};

struct SKYBOX_VS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 Tex : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.worldPos = mul(input.Pos, World);
    output.Norm = mul(input.Norm, World); //mul(input.Norm, World);
    output.Tex = input.Tex;
    return output;
}

PS_INPUT VSWave(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, Projection);
    output.worldPos = mul(input.Pos, World);
    output.Norm = mul(float4(input.Norm, 1), World).xyz;
    output.Tex = input.Tex;
    return output;
}

SKYBOX_VS_INPUT SKYBOX_VS(SKYBOX_VS_INPUT input)
{
    SKYBOX_VS_INPUT output = (SKYBOX_VS_INPUT) 0;
    matrix WVP = mul(mul(World, View), Projection);
    output.Pos = mul(float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0f), (WVP)).xyww;
    
    output.Tex = input.Pos;
    return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shaders
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void GS(triangle PS_INPUT input[3], inout TriangleStream<PS_INPUT> output)
{
    for(int i = 0; i < 3; i++)
    {
        output.Append(input[i]);
    }
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
    input.Norm = normalize(input.Norm);
    // Ambient Light
    float4 ambientColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    float4 color = meshTXDiffuse.Sample(samLinear, input.Tex) * ambientColor;
    // Directional Light
    color += saturate(dot((float3) -vLightDir, input.Norm)) * vLightColor * meshTXDiffuse.Sample(samLinear, input.Tex);

    color.a = 1;
    return color;
    //return txDiffuse.Sample(samLinear, input.Tex) * vOutputColor;
}

float4 PS_Specular(PS_INPUT input) : SV_Target
{
    input.Norm = normalize(input.Norm);
    // Ambient Light
    float4 ambientColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    float4 color = vOutputColor * ambientColor;
    matrix WVP = mul(mul(World, View), Projection);
    // Directional Light
    color += saturate(dot((float3) -vLightDir, input.Norm)) * vLightColor * vOutputColor;

    // Specular
    float specularPower = 25.0f, specularIntensity = 0.75f;
    float4 viewDir = normalize((WVP)[3] - mul(input.worldPos, View));
    
    float3 halfvector = normalize((-vLightDir) + viewDir);
    
    float intensity = max(pow(saturate(dot(input.Norm, halfvector)), specularPower), 0);
    
    float4 reflectedcolor = vLightColor * specularIntensity * intensity;
    
    color += reflectedcolor;
    color.a = 1;
    return color;
}

float4 PS_SolidTexture(PS_INPUT input) : SV_Target
{    
    float4 color = txDiffuse.Sample(samLinear, input.Tex);
    return color;
}

float4 PS_Crosshair(PS_INPUT input) : SV_Target
{
    float4 color = crosshairTX.Sample(samLinear, input.Tex);
    if(color.a < 0.5f)
        discard;
    return color;
}

// SKYBOX PS
float4 SKYBOX_PS(SKYBOX_VS_INPUT input) : SV_Target
{    
    return skybox.Sample(samLinear, input.Tex);
}