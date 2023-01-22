//-----------------------------------------------------
//  Global Variables
//-----------------------------------------------------
float gPi = 3.14159265359f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : WorldMatrix;
float4x4 gViewInverse : ViewInverse;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

RasterizerState gRasterizerState
{
    CullMode = back;
};

BlendState gBlendState
{
    BlendEnable[0] = false;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = 1;
    DepthFunc = less;
    StencilEnable = false;
};

//-----------------------------------------------------
//  Input/Output Structs
//-----------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 Uv: TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : COLOR;
    float2 Uv: TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

//-----------------------------------------------------
//  Vertex Shader
//-----------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = float4(input.Position, 1.0f);
    output.WorldPosition = mul(output.Position, gWorld);
    output.Position = mul(output.Position, gWorldViewProj);
    output.Uv = input.Uv;
    output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
    output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
    return output;
}

//-----------------------------------------------------
//  Pixel Shader
//-----------------------------------------------------
float4 Diffuse(float kd, float4 color)
{
    return (color * kd) / gPi * gLightIntensity;
}

float4 Phong(float4 specularColor, float ks, float exp, float3 lightDir, float3 viewDir, float3 normal)
{
    float dot = lightDir - (normal * (2.0f * (normal * lightDir))) * viewDir;

    if (dot < 0)
    {
        return specularColor * 0;
    }

    return specularColor * ks * pow(dot, exp);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 binormal = cross(input.Normal, input.Tangent);
    float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.f), float4(binormal, 0.f), float4(input.Normal, 0.f), float4(0.f, 0.f, 0.f, 0.f));
    float3 sampledNormal = gNormalMap.Sample(samPoint, input.Uv).rgb;
    sampledNormal = 2.f * sampledNormal - float3(1.f, 1.f, 1.f);
    sampledNormal = mul(sampledNormal, tangentSpaceAxis);

    float observedArea = saturate(dot(sampledNormal, -gLightDirection));
    float4 lambert = Diffuse(1.f, gDiffuseMap.Sample(samPoint, input.Uv));

    float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
    float phongExp = gShininess * gGlossinessMap.Sample(samPoint, input.Uv).r;
    float phong = Phong(gSpecularMap.Sample(samPoint, input.Uv), 1.f, phongExp, gLightDirection, viewDirection, sampledNormal);

    return (lambert + phong) * observedArea;
}

//-----------------------------------------------------
//  Technique
//-----------------------------------------------------
technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
