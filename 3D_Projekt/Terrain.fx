#include "LightHelper.fx"

cbuffer cbPerFrame
{
	matrix world;
	matrix WVP; 
	float4 dirToSun;
};

cbuffer cbFixed
{
	float TexScale = 10;
};
 
Texture2D layer0;
Texture2D layer1;
Texture2D layer2;
Texture2D layer3;
Texture2D layer4;
Texture2D blendMap;

SamplerState TriLinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_IN
{
	float3 pos     : POSITION;
	float3 normal  : NORMAL;
	float2 tex     : TEXCOORD;
};

struct VS_OUT
{
	float4 pos          : SV_POSITION;
	float shade         : SHADE;
    float2 tiledUV      : TEXCOORD0;
    float2 stretchedUV  : TEXCOORD1; 
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
	
	vOut.pos = mul(float4(vIn.pos, 1.0f), WVP);
	
	float4 normal = mul(float4(vIn.normal, 0.0f), world);
	
	vOut.shade = saturate(max(dot(normal, dirToSun), 0.0f) + 0.1f);

	vOut.tiledUV     = TexScale*vIn.tex;
	vOut.stretchedUV = vIn.tex;
	
	return vOut;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	float4 c0 = layer0.Sample( TriLinearSampler, pIn.tiledUV );
	float4 c1 = layer1.Sample( TriLinearSampler, pIn.tiledUV );
	float4 c2 = layer2.Sample( TriLinearSampler, pIn.tiledUV );
	float4 c3 = layer3.Sample( TriLinearSampler, pIn.tiledUV );
	float4 c4 = layer4.Sample( TriLinearSampler, pIn.tiledUV ); 
	
	float4 t = blendMap.Sample( TriLinearSampler, pIn.stretchedUV ); 
    
    float4 C = c0;
    C = lerp(C, c1, t.r);
    C = lerp(C, c2, t.g);
    C = lerp(C, c3, t.b);
    C = lerp(C, c4, t.a);
    
    C *= pIn.shade;
      
    return C;
}

technique10 TerrainTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}