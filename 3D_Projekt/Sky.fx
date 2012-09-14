cbuffer cbPerFrame
{
	matrix WVP;
};

TextureCube cubeMap;

SamplerState TriLinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_IN
{
	float3 pos : POSITION;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
    float3 tex : TEXCOORD;
};
 
VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
	vOut.pos = mul(float4(vIn.pos, 1.0f), WVP).xyww;
	vOut.tex = vIn.pos;
	
	return vOut;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	return cubeMap.Sample(TriLinearSampler, pIn.tex);
}

RasterizerState NoCull
{
    CullMode = None;
};

DepthStencilState LessEqualDSS
{
    DepthFunc = LESS_EQUAL;
};

technique10 SkyTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
        
        SetRasterizerState(NoCull);
        SetDepthStencilState(LessEqualDSS, 0);
    }
}
