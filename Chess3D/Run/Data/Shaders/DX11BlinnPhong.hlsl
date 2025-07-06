//#pragma target 5.0

struct VertexInput
{
	float3 a_modelPosition : POSITION;
	float4 a_color : COLOR;
	float2 a_uv : TEXCOORD;
 	float3 a_modelTangent : TANGENT;
 	float3 a_modelBitangent : BITANGENT;
 	float3 a_modelNormal : NORMAL;
};

struct VertexOutPixelIn
{
	float4 v_worldPosition: WORLD_POSITION;
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

cbuffer LightConstants : register(b1)
{
	float3 c_sunDirection;
	float c_sunIntensity;
	float c_ambientIntensity;
};

cbuffer CameraConstants : register(b2)
{
	float4x4 c_worldToCameraTransform;		// View Mat
	float4x4 c_cameraToRenderTransform;		// Non-standard transform from game to DirectX conventions (Game Mat)
	float4x4 c_renderToClipTransform;		// Projection Mat
};

cbuffer ModelConstants : register(b3)
{
	float4x4 c_modelToWorldTransform;		// Model transform
	float4 c_modelColor;
};

// point light and spot light
cbuffer PointLightBuffer : register(b4)
{
	struct PointLight
	{
		float3 c_position;
		float c_innerRadius;
		float4 c_color;       
		float c_outerRadius; 
		float c_intensity;
		float2 c_padding;
	};
	PointLight c_pointLights[10];
    uint c_activePointLightCount;
	float3 c_padding_point;
};

 
cbuffer SpotLightBuffer : register(b5)
{
	struct SpotLight
	{    
		float3 c_position;  
		float c_intensity;                   
		float3 c_direction;     
		float c_padding;	    
		float4 c_color;       
		float c_innerCutoffAngle;   
		float c_outerCutoffAngle;   
		float c_innerRadius;
		float c_outerRadius;	
	};
	SpotLight c_spotLights[10];
	uint c_activeSpotLightCount;
	float3 c_padding_spot;
};

// debug use
cbuffer PerFrameConstants : register(b7)
{
	float		c_time;
	int			c_debugInt;
	float		c_debugFloat;
	float		EMPTY_PADDING;
};

//------------------------------------------------------------------------------------------------
// Help Functions
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

// x,y,z in [-1,1] to rgb [0,1]
float3 EncodeXYZToRGB( float3 vec )
{
	return (vec + 1.0) * 0.5;
}
// rgb in [0,1] to xyz in [-1,1]
float3 DecodeRGBToXYZ( float3 color )
{
	return (color * 2.0) - 1.0;
}

float3 GetCameraWorldPosition(float4x4 viewMatrix)
{
    float3x3 rotation = transpose((float3x3)viewMatrix);
    float3 translation = -viewMatrix._14_24_34;
    return mul(rotation, translation);
}

// float3 CalculatePointLight(PointLight pointLight, float3 worldPos, 
//     float3 pixelNormalWorldSpace, float3 viewDir, 
//     float3 diffuseColor, float3 specularColor, float shininess)
// {
//     float3 lightVector = pointLight.c_position - worldPos;
//     float distance = length(lightVector);
//     
//     if(distance < pointLight.c_range)
//     {
//         lightVector = normalize(lightVector);   
//         float NdotL = saturate(dot(pixelNormalWorldSpace, lightVector));
//         
//         float attenuation = 1.0 / (pointLight.c_attenuation.x + 
//                                    pointLight.c_attenuation.y * distance + 
//                                    pointLight.c_attenuation.z * distance * distance);
//         
//         float3 diffuse = diffuseColor * pointLight.c_color.xyz * pointLight.c_intensity * NdotL * attenuation;
//         
//         float3 halfVector = normalize(lightVector + viewDir);
//         float specularTerm = pow(max(dot(pixelNormalWorldSpace, halfVector), 0), shininess);
//         float3 specular = specularTerm * specularColor * pointLight.c_intensity * attenuation;
//         
//         return diffuse + specular;
//     }
//     
//     return float3(0.0, 0.0, 0.0);
// }

//------------------------------------------------------------------------------------------------
Texture2D t_diffuseTexture : register(t0);
Texture2D<float4> t_normalTexture : register(t1);
Texture2D<float4> t_sgeTexture : register(t2);
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
	v2p.v_worldPosition=worldPosition;
 	v2p.v_worldTangent = worldTangent;
 	v2p.v_worldBitangent = worldBitangent;
 	v2p.v_worldNormal = worldNormal;

	v2p.v_modelTangent = float4(input.a_modelTangent, 0.0f);      
	v2p.v_modelBitangent = float4(input.a_modelBitangent, 0.0f);  
	v2p.v_modelNormal = float4(input.a_modelNormal, 0.0f);      

	return v2p;
}
//------------------------------------------------------------------------------------------------
float4 PixelMain(VertexOutPixelIn input) : SV_Target0
{
	// ambient
 	float ambient = c_ambientIntensity;

	// directional light
	float3 sunDir=normalize(c_sunDirection.xyz);

	// diffuse color
 	float4 textureColor = t_diffuseTexture.Sample(s_samplerState, input.v_uv);
 	float4 vertexColor = input.v_color;
 	float4 modelColor = c_modelColor;
	float4 diffuseColor=textureColor*modelColor*vertexColor;

	// SGE texture
	float4 sgeTexture = t_sgeTexture.Sample(s_samplerState, input.v_uv);
	float specularColor = sgeTexture.r;    
	float gloss = sgeTexture.g;       
	float emission = sgeTexture.b;    

	// pixel normal
	float4 normalTextureColor = t_normalTexture.Sample(s_normalSampler, input.v_uv);
	float3 pixelNormalTBNSpace=normalize(DecodeRGBToXYZ(normalTextureColor.rgb));
	
	// input world && model normal
	float3 surfaceTangentWorldSpace		= normalize( input.v_worldTangent.xyz );
	float3 surfaceBitangentWorldSpace	= normalize( input.v_worldBitangent.xyz );
	float3 surfaceNormalWorldSpace		= normalize( input.v_worldNormal.xyz );

	float3 surfaceTangentModelSpace		= normalize( input.v_modelTangent.xyz );
	float3 surfaceBitangentModelSpace	= normalize( input.v_modelBitangent.xyz );
	float3 surfaceNormalModelSpace		= normalize( input.v_modelNormal.xyz );

	// combine pixel normal with world normal
	float3x3 tbnToWorldMat = float3x3(surfaceTangentWorldSpace, surfaceBitangentWorldSpace, surfaceNormalWorldSpace);
	float3 pixelNormalWorldSpace = mul(pixelNormalTBNSpace, tbnToWorldMat);

	// specular
	float3 cameraWorldPos = GetCameraWorldPosition(c_worldToCameraTransform);
	float3 viewDir = normalize(cameraWorldPos - input.v_worldPosition.xyz);
    float3 halfVector = normalize(-sunDir + viewDir);
	float shininess =lerp(1.0, 32.0, gloss);
	float specularTerm=pow(max(dot(pixelNormalWorldSpace,halfVector),0),shininess);
	float3 specular=specularTerm*specularColor*c_sunIntensity;

	//  emissive
	float3 emissive = textureColor.rgb * emission; 

	// point lights
	float3 pointLighting = float3(0, 0, 0);
	int pointLightCount=c_activePointLightCount;
	for(int i=0;i<pointLightCount;i++)
	{
 		PointLight light = c_pointLights[i];
 		float3 lightVector = light.c_position - input.v_worldPosition.xyz;
 		float distance = length(lightVector);
     
 		if(distance < light.c_outerRadius)
 		{
 			lightVector = normalize(lightVector);   
 			float NdotL = saturate(dot(pixelNormalWorldSpace, lightVector));
         
//  			float attenuation = 1.0 / (light.c_attenuation.x + 
//  									   light.c_attenuation.y * distance + 
//  									   light.c_attenuation.z * distance * distance);

			float falloff=1.0;
			if(distance<light.c_innerRadius)
			{
				falloff=1.0;
			}
			else
			{
				falloff=RangeMapClamped(distance,light.c_innerRadius,light.c_outerRadius,1.0,0.0);
			}
         
 			float3 diffuse = diffuseColor.rgb * light.c_color.xyz * light.c_intensity * NdotL * falloff;
         
 			float3 halfVector = normalize(lightVector + viewDir);
 			float specularTerm = pow(max(dot(pixelNormalWorldSpace, halfVector), 0), shininess);
 			float3 specular = specularTerm * float3(specularColor, specularColor, specularColor) * light.c_intensity * falloff;
         
 			pointLighting += diffuse + specular;
 		}
//  		pointLighting += CalculatePointLight(c_pointLights[i], input.v_worldPosition.xyz, 
//                                             pixelNormalWorldSpace, viewDir, 
//                                             diffuseColor.rgb, float3(specularColor,specularColor,specularColor), shininess);
	}

	// spot light
	float3 spotLighting = float3(0, 0, 0);
	int spotLightCount=c_activeSpotLightCount;
	for(int j=0;j<spotLightCount;j++)
	{
 		SpotLight light = c_spotLights[j];
 		float3 lightVector = light.c_position - input.v_worldPosition.xyz;
 		float distance = length(lightVector);
     
 		if(distance < light.c_outerRadius)
 		{
			if(distance > 0.001) 
			{
				lightVector /= distance; 
			}
			float spotEffect = dot(lightVector, normalize(-light.c_direction));
            float spotFactor = 1.0;
			if (spotEffect > cos(light.c_innerCutoffAngle))
			{
				spotFactor = 1.0;
			}
			else
			{
				spotFactor = RangeMapClamped(spotEffect, 
                            cos(light.c_outerCutoffAngle),  
                            cos(light.c_innerCutoffAngle),  
                            0.0,                            
                            1.0);       
			}

           float diffuseFactor = max(dot(pixelNormalWorldSpace.xyz, lightVector), 0.0);

//             float attenuation = 1.0 / (light.c_attenuation.x + 
//                                      light.c_attenuation.y * distance + 
//                                       light.c_attenuation.z * distance * distance);

			float falloff=1.0;
			if(distance<light.c_innerRadius)
			{
				falloff=1.0;
			}
			else
			{
				falloff=RangeMapClamped(distance,light.c_innerRadius,light.c_outerRadius,1.0,0.0);
			}

 			float3 diffuse = diffuseColor.rgb * light.c_color.xyz * light.c_intensity * falloff * diffuseFactor*spotFactor;
         
 			float3 halfVector = normalize(lightVector + viewDir);
 			float specularTerm = pow(max(dot(pixelNormalWorldSpace, halfVector), 0.0), shininess);
 			float3 specular = specularTerm * float3(specularColor, specularColor, specularColor) * light.c_intensity * falloff* spotFactor;
         
 			spotLighting += diffuse + specular;
 		}
	}

	// final color without pixel normal map
	float diffuseIntensityVertexNormal =c_sunIntensity * saturate(dot(surfaceNormalWorldSpace, -sunDir));
	float4 totalLightingVertexNormal= float4((ambient + diffuseIntensityVertexNormal).xxx, 1);

	float diffuseIntensityPixelNormal =c_sunIntensity * saturate(dot(normalize(pixelNormalWorldSpace.xyz), -sunDir));
	float4 totalLightingPixelNormal=  float4((ambient + diffuseIntensityPixelNormal).xxx, 1);

//+pointLighting         +spotLighting+pointLighting
	float4 finalColor = float4( diffuseColor.rgb * totalLightingPixelNormal.rgb+specular+emissive, diffuseColor.a ); 

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
		// TBN SPACE PIXEL NORMAL
		finalColor.rgb = EncodeXYZToRGB( pixelNormalTBNSpace );
	}
	else if(c_debugInt == 9 )
	{
		// WORLD SPACE PIXEL NORMAL
		finalColor.rgb = EncodeXYZToRGB( pixelNormalWorldSpace );
	}
	else if(c_debugInt == 10 )
	{
		// Lit with vertex normals only (ignore normal maps)
		finalColor = float4( diffuseColor.rgb * totalLightingVertexNormal.rgb, diffuseColor.a ); 
	}
	else if(c_debugInt == 11)
	{
		// Show diffuse intensity with vertex normals
		finalColor.rgb = diffuseIntensityVertexNormal.xxx;
	}
	else if(c_debugInt == 12)
	{
		// Show diffuse intensity with pixel normals
		finalColor.rgb = diffuseIntensityPixelNormal.xxx;
	}
	else if(c_debugInt == 13)
	{
		// Show total lighting with vertex normals
		finalColor = totalLightingVertexNormal;
	}
	else if(c_debugInt == 14)
	{
		// WORLD SPACE TANGENT
		finalColor.rgb = EncodeXYZToRGB(surfaceTangentWorldSpace);
	}
	else if(c_debugInt == 15)
	{
		// WORLD SPACE BITANGENT
		finalColor.rgb = EncodeXYZToRGB(surfaceBitangentWorldSpace);
	}
	else if(c_debugInt == 16)
	{
		// WORLD SPACE SURFACE NORMAL
		finalColor.rgb = EncodeXYZToRGB(surfaceNormalWorldSpace);
	}
 	else if(c_debugInt == 17)
 	{
 		// MODEL I-BASIS IN WORLD SPACE
 		float3 modelIBasisWorld = mul(c_modelToWorldTransform, float4(1,0,0,0) ).xyz;
 		finalColor.rgb = EncodeXYZToRGB(normalize( modelIBasisWorld.xyz ));
 	}
 	else if(c_debugInt == 18)
 	{
 		// MODEL J-BASIS IN WORLD SPACE
 		float3 modelJBasisWorld = mul(c_modelToWorldTransform, float4(0,1,0,0) ).xyz;
 		finalColor.rgb = EncodeXYZToRGB(normalize( modelJBasisWorld.xyz ));
 	}
 	else if(c_debugInt == 19)
 	{
 		// MODEL K-BASIS IN WORLD SPACE
//  		float3 modelKBasisWorld = mul(c_modelToWorldTransform, float4(0,0,1,0) ).xyz;
//  		finalColor.rgb = EncodeXYZToRGB(normalize( modelKBasisWorld.xyz ));
		finalColor = float4(emissive, 1.0);
 	}

 	clip(finalColor.a - 0.01f);
 	return finalColor;
}
