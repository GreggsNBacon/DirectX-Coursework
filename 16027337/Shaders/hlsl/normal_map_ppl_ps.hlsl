
//
// Model a simple light
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
	float4				Attenuation;// x=constant,y=linear,z=quadratic,w=cutOff
};


//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float3				tangent			:TANGENT;
	float4				matDiffuse		: DIFFUSE; // a represents alpha.
	float4				matSpecular		: SPECULAR; // a represents specular power. 
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState linearSampler : register(s0);


//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) {

	FragmentOutputPacket outputFragment;
	float att = 1.0;
	float3 N = normalize(v.normalW);
	float4 baseColour = v.matDiffuse;
	float3 lightDir = -lightVec.xyz; // Directional light
	if (lightVec.w == 1.0) lightDir = lightVec.xyz - v.posW;
	lightDir = normalize(lightDir);
	baseColour = diffuseTexture.Sample(linearSampler, v.texCoord);
	float dist = length(lightDir);
	float cutOff = Attenuation.w;

	if (!(dist > cutOff))
	{
		float con = Attenuation.x; 
		float lin = Attenuation.y; 
		float quad = Attenuation.z;
		att = 1.0 / (con + lin * dist + quad * dist * dist);
	}
	//Load normal from normal map
	float4 normalMap = normalTexture.Sample(linearSampler, v.texCoord);
	//Change normal map range from [0, 1] to [-1, 1]
	normalMap = (2.0f * normalMap) - 1.0f;
	//Make sure tangent is completely orthogonal to normal
	v.tangent = normalize(v.tangent - dot(v.tangent, N) * N);
	//Create the biTangent
	float3 biTangent = cross(N, v.tangent);
	//Create the "Texture Space"
	float3x3 texSpace = float3x3(v.tangent, biTangent, N);
	//Convert normal from normal map to texture space and store in input.normal
	N = normalize(mul(normalMap, texSpace));
	float3 finalColor;


	//specular
	float specPower = max(v.matSpecular.a * 1000.0, 1.0f);
	float3 eyeDir = normalize(eyePos - v.posW);
	float3 R = reflect(-lightDir, N);
	float specFactor = pow(max(dot(R, eyeDir), 0.0f), specPower);


	finalColor = baseColour.xyz * lightAmbient;
	finalColor += att *  max(dot(lightDir, N), 0.0f) * lightDiffuse * baseColour.xyz;
	finalColor += specFactor * v.matSpecular.xyz * lightSpecular;
	outputFragment.fragmentColour = float4(finalColor, baseColour.a);
	return outputFragment;
}


