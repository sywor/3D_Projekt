cbuffer cbPerFrame
{
	float4 EyePos;
	float4 EmitPos;
	float4 EmitDir;	
	float GameTime;
	float TimeStep;
	matrix ViewProj; 
};

cbuffer cbFixed
{
	float3 Accel = {0.0f, 7.8f, 0.0f};
	float2 QuadTexCoord[4] = 
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};
 
Texture2DArray TexArray;
Texture1D RandomTex;
 
SamplerState TriLinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
 
DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

float3 RandVector(float offset)
{
	float u = (GameTime + offset);	
	float3 v = RandomTex.SampleLevel(TriLinearSampler, u, 0);	
	return normalize(v);
}

#define PT_EMITTER 0
#define PT_FLARE 1
 
struct Particle
{
	float3 initialPos	: POSITION;
	float3 initialVel	: VELOCITY;
	float2 size			: SIZE;
	float age			: AGE;
	uint type			: TYPE;
};
  
Particle StreamOutVS(Particle vIn)
{
	return vIn;
}

[maxvertexcount(2)]
void StreamOutGS(point Particle gIn[1], inout PointStream<Particle> ptStream)
{	
	gIn[0].age += TimeStep;
	
	if( gIn[0].type == PT_EMITTER )
	{	
		if( gIn[0].age > 0.005f )
		{
			float3 rand = RandVector(0.0f);
			rand.x *= 0.5f;
			rand.z *= 0.5f;
			
			Particle p;
			p.initialPos = EmitPos.xyz;
			p.initialVel = 4.0f*rand;
			p.size       = float2(3.0f, 3.0f);
			p.age         = 0.0f;
			p.type        = PT_FLARE;
			
			ptStream.Append(p);
			gIn[0].age = 0.0f;
		}
		
		ptStream.Append(gIn[0]);
	}
	else
	{
		if( gIn[0].age <= 1.0f )
		{
			ptStream.Append(gIn[0]);
		}
	}		
}

GeometryShader gsStreamOut = ConstructGSWithSO( 
	CompileShader( gs_4_0, StreamOutGS() ), 
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x" );
	
technique10 StreamOutTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, StreamOutVS() ) );
        SetGeometryShader( gsStreamOut );
        SetPixelShader(NULL);
        SetDepthStencilState( DisableDepth, 0 );
    }
}

struct VS_OUT
{
	float3 pos		: POSITION;
	float2 size		: SIZE;
	float4 color	: COLOR;
	uint type		: TYPE;
};

VS_OUT DrawVS(Particle vIn)
{
	VS_OUT vOut;
	
	vOut.pos = 0.5f * vIn.age * vIn.age * Accel + vIn.age * vIn.initialVel + vIn.initialPos;

	float opacity = 1.0f - smoothstep(0.0f, 1.0f, vIn.age/1.0f);
	vOut.color = float4(1.0f, 1.0f, 1.0f, opacity);
	
	vOut.size = vIn.size;
	vOut.type  = vIn.type;
	
	return vOut;
}

struct GS_OUT
{
	float4 pos		: SV_Position;
	float4 color	: COLOR;
	float2 texCoord	: TEXCOORD;
};

[maxvertexcount(4)]
void DrawGS(point VS_OUT gIn[1], inout TriangleStream<GS_OUT> triStream)
{	
	if( gIn[0].type != PT_EMITTER )
	{
		float3 look  = normalize(EyePos.xyz - gIn[0].pos);
		float3 right = normalize(cross(float3(0.0f, 1.0f ,0.0f), look));
		float3 up    = cross(look, right);
		
		matrix W;
		W[0] = float4(right,       0.0f);
		W[1] = float4(up,          0.0f);
		W[2] = float4(look,        0.0f);
		W[3] = float4(gIn[0].pos,  1.0f);

		matrix WVP = mul(W, ViewProj);

		float halfWidth  = 0.5f*gIn[0].size.x;
		float halfHeight = 0.5f*gIn[0].size.y;
	
		float4 v[4];
		v[0] = float4(-halfWidth, -halfHeight, 0.0f, 1.0f);
		v[1] = float4(+halfWidth, -halfHeight, 0.0f, 1.0f);
		v[2] = float4(-halfWidth, +halfHeight, 0.0f, 1.0f);
		v[3] = float4(+halfWidth, +halfHeight, 0.0f, 1.0f);

		GS_OUT gOut;
		[unroll]
		for(int i = 0; i < 4; ++i)
		{
			gOut.pos  = mul(v[i], WVP);
			gOut.texCoord  = QuadTexCoord[i];
			gOut.color = gIn[0].color;
			triStream.Append(gOut);
		}	
	}
}

float4 DrawPS(GS_OUT pIn) : SV_TARGET
{
	return TexArray.Sample(TriLinearSampler, float3(pIn.texCoord, 0))*pIn.color;
}

technique10 DrawTech
{
    pass P0
    {
        SetVertexShader(   CompileShader( vs_4_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_4_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_4_0, DrawPS() ) );
        
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
        SetDepthStencilState( NoDepthWrites, 0 );
    }
}