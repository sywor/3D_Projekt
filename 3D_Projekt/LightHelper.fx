struct Light
{
	float3 pos;
	float3 dir;
	float4 ambient;
	float4 diffuse;
	float4 spec;
	float3 att;
	float  spotPower;
	float  range;
};

struct SurfaceInfo
{
	float3 pos;
    float3 normal;
    float4 diffuse;
    float4 spec;
};

float3 ParallelLight(SurfaceInfo V, Light L, float3 eyePos, float shadowFactor)
{
	float3 litColor = float3(0.0f, 0.0f, 0.0f);
	float3 lightVec = -L.dir;
	litColor += V.diffuse * L.ambient;
		
	float diffuseFactor = dot(lightVec, V.normal);
	[branch]
	if( diffuseFactor > 0.0f )
	{
		float specPower   = max(V.spec.a, 1.0f);
		float3 toEye      = normalize(eyePos - V.pos);
		float3 reflectVec = reflect(-lightVec, V.normal);
		float specFactor  = pow(max(dot(reflectVec, toEye), 0.0f), specPower);
					
		litColor += shadowFactor * diffuseFactor * V.diffuse * L.diffuse;
		litColor += shadowFactor * specFactor * V.spec * L.spec;
	}
	
	return litColor;
}

float3 PointLight(SurfaceInfo V, Light L, float3 eyePos, float shadowFactor)
{
	float3 litColor = float3(0.0f, 0.0f, 0.0f);
	float3 lightVec = L.pos - V.pos;
	float d = length(lightVec);
	
	if( d > L.range )
		return float3(0.0f, 0.0f, 0.0f);
		
	lightVec /= d; 
	litColor += V.diffuse * L.ambient;	
	
	float diffuseFactor = dot(lightVec, V.normal);
	[branch]
	if( diffuseFactor > 0.0f )
	{
		float specPower   = max(V.spec.a, 1.0f);
		float3 toEye      = normalize(eyePos - V.pos);
		float3 reflectVec = reflect(-lightVec, V.normal);
		float specFactor  = pow(max(dot(reflectVec, toEye), 0.0f), specPower);
	
		litColor += shadowFactor * diffuseFactor * V.diffuse * L.diffuse;
		litColor += shadowFactor * specFactor * V.spec * L.spec;
	}
	
	return litColor / dot(L.att, float3(1.0f, d, d*d));
}

float3 Spotlight(SurfaceInfo V, Light L, float3 eyePos, float shadowFactor)
{
	float3 litColor = PointLight(V, L, eyePos, shadowFactor);
	float3 lightVec = normalize(L.pos - V.pos);
	float s = pow(max(dot(-lightVec, L.dir), 0.0f), L.spotPower);	

	return litColor*s;
}

 
 