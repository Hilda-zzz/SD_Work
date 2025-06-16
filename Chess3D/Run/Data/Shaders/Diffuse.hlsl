struct VertexInput
{
	float3 a_modelPosition : POSITION;
	float4 a_color : COLOR;
	float2 a_uv : TEXCOORD;
 	float3 a_modelTangent : TANGENT;
 	float3 a_modelBitangent : BITANGENT;
 	float3 a_modelNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct VertexOutPixelIn
{
	float4 v_clipPosition : SV_Position;
	float4 v_color : COLOR;
	float2 v_uv : TEXCOORD;
 	float4 v_worldTangent : WORLD_TANGENT;
 	float4 v_worldBitangent : WORLD_BITANGENT;
 	float4 v_worldNormal : WORLD_NORMAL;

 	float4 v_modelTangent : MODEL_TANGENT;      
 	float4 v_modelBitangent : MODEL_BITANGENT;  
 	float4 v_modelNormal : MODEL_NORMAL;        
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 c_sunDirection;
	float c_sunIntensity;
	float c_ambientIntensity;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 c_worldToCameraTransform;	// View transform
	float4x4 c_cameraToRenderTransform;	// Non-standard transform from game to DirectX conventions
	float4x4 c_renderToClipTransform;		// Projection transform
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 c_modelToWorldTransform;		// Model transform
	float4 c_modelColor;
};

//------------------------------------------------------------------------------------------------
cbuffer PerFrameConstants: register(b7)
{
	float	c_time;
	int		c_debugInt;
	float	c_debugFloat;
	int		EMPTY_PADDING;
};

//------------------------------------------------------------------------------------------------
Texture2D t_diffuseTexture : register(t0);
Texture2D<float4> t_normalTexture : register(t1);
//------------------------------------------------------------------------------------------------
SamplerState s_samplerState : register(s0);
SamplerState s_normalSampler : register(s1);
//------------------------------------------------------------------------------------------------
VertexOutPixelIn VertexMain(VertexInput input)
{
	float4 modelPosition = float4(input.a_modelPosition, 1);
	float4 worldPosition = mul(c_modelToWorldTransform, modelPosition);      
	float4 cameraPosition = mul(c_worldToCameraTransform, worldPosition);    
	float4 renderPosition = mul(c_cameraToRenderTransform, cameraPosition);  
	float4 clipPosition = mul(c_renderToClipTransform, renderPosition);      

 	float4 worldTangent = mul(c_modelToWorldTransform, float4(input.a_modelTangent, 0.0f));     
 	float4 worldBitangent = mul(c_modelToWorldTransform, float4(input.a_modelBitangent, 0.0f)); 
 	float4 worldNormal = mul(c_modelToWorldTransform, float4(input.a_modelNormal, 0.0f));       

	VertexOutPixelIn v2p;
	v2p.v_clipPosition = clipPosition;
	v2p.v_color = input.a_color;
	v2p.v_uv = input.a_uv;
 	v2p.v_worldTangent = worldTangent;
 	v2p.v_worldBitangent = worldBitangent;
 	v2p.v_worldNormal = worldNormal;

	v2p.v_modelTangent = float4(input.a_modelTangent, 0.0f);      
	v2p.v_modelBitangent = float4(input.a_modelBitangent, 0.0f);  
	v2p.v_modelNormal = float4(input.a_modelNormal, 0.0f);      

	return v2p;
}
//------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if(inEnd!=inStart)
	{
		float fraction = (inValue - inStart) / (inEnd - inStart);
		float outValue = outStart + fraction * (outEnd - outStart);
		return outValue;
	}
	return outStart;
}

float RangeMapClamped( float inValue, float inStart, float inEnd, float outStart, float outEnd )
{
	float fraction = saturate( (inValue - inStart) / (inEnd - inStart) );
	float outValue = outStart + fraction * (outEnd - outStart);
	return outValue;
}

float3 EncodeXYZToRGB( float3 vec )
{
	return (vec + 1.0) * 0.5;
}

float3 DecodeRGBToXYZ( float3 color )
{
	return (color * 2.0) - 1.0;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(VertexOutPixelIn input) : SV_Target0
{
 	float ambient = c_ambientIntensity;
 	float4 textureColor = t_diffuseTexture.Sample(s_samplerState, input.v_uv);
	float4 normalTextureColor = t_normalTexture.Sample(s_normalSampler, input.v_uv);
 	float4 vertexColor = input.v_color;
 	float4 modelColor = c_modelColor;

 	//float4 color = lightColor * textureColor * vertexColor * modelColor;

	float3 pixelNormalTBNSpace=normalize(DecodeRGBToXYZ(normalTextureColor.rgb));
	float4 diffuseColor=textureColor*modelColor*vertexColor;

	float3 lightDir=normalize(c_sunDirection.xyz);

	float3 surfaceTangentWorldSpace		= normalize( input.v_worldTangent.xyz );
	float3 surfaceBitangentWorldSpace	= normalize( input.v_worldBitangent.xyz );
	float3 surfaceNormalWorldSpace		= normalize( input.v_worldNormal.xyz );

	float3 surfaceTangentModelSpace		= normalize( input.v_modelTangent.xyz );
	float3 surfaceBitangentModelSpace	= normalize( input.v_modelBitangent.xyz );
	float3 surfaceNormalModelSpace		= normalize( input.v_modelNormal.xyz );

	float3x3 tbnToWorldMat = float3x3(surfaceTangentWorldSpace, surfaceBitangentWorldSpace, surfaceNormalWorldSpace);
	float3 pixelNormalWorldSpace = mul(pixelNormalTBNSpace,  tbnToWorldMat);

 	float directionalWithoutNormalMap=c_sunIntensity * saturate(dot(normalize( input.v_worldNormal.xyz ), -c_sunDirection));
 	float4 lightColorWithoutNormalMap = float4((ambient + directionalWithoutNormalMap).xxx, 1);

  	float directional = c_sunIntensity * saturate(dot(normalize(pixelNormalWorldSpace.xyz), -c_sunDirection));
 	float4 lightColor = float4((ambient + directional).xxx, 1);

	//float diffuseLightDot=dot(-lightDir,pixelNormalWorldSpace);
// 	if(c_debugInt==10||c_debugInt==12)
// 	{
// 		diffuseLightDot=dot(-lightDir,surfaceNormalWorldSpace);
// 	}

	//float lightStrength = saturate( RangeMapClamped( diffuseLightDot, -1.0, 1.0, -1 + 2*ambient, 1.0 ) );
	//float4 finalColor = float4( diffuseColor.rgb * lightStrength, diffuseColor.a ); 
// 	if( finalColor.a <= 0.001 ) // a.k.a. "clip" in HLSL
// 	{
// 		discard;
// 	}
	
	 float4 finalColor = float4( diffuseColor.rgb * lightColor.rgb, diffuseColor.a ); 

	if (c_debugInt == 1)
	{
		finalColor = textureColor; 
	}
	else if (c_debugInt == 2)
	{
		finalColor = vertexColor;
	}
	else if (c_debugInt == 3)
	{
		finalColor = float4(input.v_uv.x, input.v_uv.y, 0.f, 1.f);
	}
	else if (c_debugInt == 4)
	{
		// MODEL SPACE
	    finalColor.rgb =  EncodeXYZToRGB( surfaceTangentModelSpace);
	}
	else if(c_debugInt == 5)
	{
		// MODEL SPACE
		finalColor.rgb = EncodeXYZToRGB( surfaceBitangentModelSpace); 
	}
	else if(c_debugInt == 6)
	{
		// MODEL SPACE
		finalColor.rgb = EncodeXYZToRGB( surfaceNormalModelSpace);
	}
	else if(c_debugInt == 7)  
	{
		// NORMAL TEX
  		finalColor = normalTextureColor;
	}
	else if(c_debugInt == 8 )
	{
		//?
		finalColor.rgb = EncodeXYZToRGB( pixelNormalTBNSpace );
	}
	else if(c_debugInt == 9 )
	{
		finalColor.rgb = EncodeXYZToRGB( pixelNormalWorldSpace );
	}
	else if(c_debugInt == 10 )
	{
		// Lit, but ignore normal maps (use surface normals only) -- see above
		finalColor=float4( diffuseColor.rgb * lightColorWithoutNormalMap.rgb, diffuseColor.a ); 
	}
	else if(c_debugInt == 11 || c_debugInt == 12 )
	{
		//finalColor.rgb = lightStrength.xxx;
	}
	else if(c_debugInt == 13)
	{
		// unused, available
	}
	else if(c_debugInt == 14)
	{
		finalColor.rgb = EncodeXYZToRGB(surfaceTangentWorldSpace);
	}
	else if(c_debugInt == 15)
	{
		finalColor.rgb = EncodeXYZToRGB(surfaceBitangentWorldSpace);
	}
	else if(c_debugInt == 16)
	{
		finalColor.rgb = EncodeXYZToRGB(surfaceNormalWorldSpace);
	}
 	else if(c_debugInt == 17)
 	{
 		float3 modelIBasisWorld = mul(c_modelToWorldTransform, float4(1,0,0,0) ).xyz;
 		finalColor.rgb = EncodeXYZToRGB(normalize( modelIBasisWorld.xyz ));
 	}
 	else if(c_debugInt == 18)
 	{
 		float3 modelJBasisWorld = mul(c_modelToWorldTransform, float4(0,1,0,0) ).xyz;
 		finalColor.rgb = EncodeXYZToRGB(normalize( modelJBasisWorld.xyz ));
 	}
 	else if(c_debugInt == 19)
 	{
 		float3 modelKBasisWorld = mul(c_modelToWorldTransform, float4(0,0,1,0) ).xyz;
 		finalColor.rgb = EncodeXYZToRGB(normalize( modelKBasisWorld.xyz ));
 	}


 	clip(finalColor.a - 0.01f);
 	return finalColor;
}
