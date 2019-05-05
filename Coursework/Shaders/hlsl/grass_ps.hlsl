
//
// Grass effect - Modified a fur technique
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------



cbuffer modelCBuffer : register(b0) {

	float4x4			worldMatrix;
	float4x4			worldITMatrix; // Correctly transform normals to world space
};
cbuffer cameraCbuffer : register(b1) {
	float4x4			viewMatrix;
	float4x4			projMatrix;
	float4				eyePos;
}
cbuffer lightCBuffer : register(b2) {
	float4				lightVec; // w=1: Vec represents position, w=0: Vec  represents direction.
	float4				lightAmbient;
	float4				lightDiffuse;
	float4				lightSpecular;
};
cbuffer sceneCBuffer : register(b3) {
	float4				windDir;
	float				Time;
	float				grassHeight;
};

//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D diffuseTexture : register(t0);
Texture2D grassAlpha: register(t1);
SamplerState anisotropicSampler : register(s0);




//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float4				matDiffuse		: DIFFUSE; // a represents alpha.
	float4				matSpecular		: SPECULAR; // a represents specular power. 
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) {

	FragmentOutputPacket outputFragment;
	float tileRepeat = 10;
	float3 colour=float3(0.0, 0.0, 0.0);
	float3 colour1 = float3(0.25, 0.1, 0.05);
	float3 green = float3(0, 0.5, 0);
	float3 colour2 = float3(-0.1, -0.2, -0.3);
	float alpha = 1.0;
	float3 N = normalize(v.normalW);
	colour = v.matDiffuse.xyz;
	colour *= diffuseTexture.Sample(anisotropicSampler, v.texCoord);

	if (grassHeight > 0.0)
	{
		// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
		float3 lightDir = -lightVec.xyz; // Directional light
		if (lightVec.w == 1.0) lightDir = lightVec.xyz - v.posW; // Positional light
		lightDir = normalize(lightDir);
		colour += max(dot(lightDir, N), 0.0f) *colour * lightDiffuse;

		alpha = grassAlpha.Sample(anisotropicSampler, v.texCoord*tileRepeat).a;
		// Reduce alpha and increase illumination for tips of grass
		colour *=  alpha+(grassHeight);// *(1 - alpha) * 3;
		alpha = (alpha - grassHeight * 40);
	}
	else
		colour *= float3(0.8, 0.8, 0.8);
	float posY = v.posW.y;
	if (posY <= -13 && posY >= -17) {
		float lerpVal = (posY + 17) / 4;
		float3 newColour = colour + colour1;
		//colour = colour1;
		colour = lerp(colour, newColour, 1 - lerpVal);
	}
	if (posY <= -17 && posY >= -20) {
		colour = colour + colour1;
		float lerpVal = (posY + 20) / 3;
		float3 newColour = colour + colour2;
		//colour = colour1;
		colour = lerp(colour, newColour, 1 - lerpVal);
	}
	outputFragment.fragmentColour = float4(colour, alpha);
	return outputFragment;

}
