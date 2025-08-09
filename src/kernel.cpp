#include "kernel.hpp" // note: unbalanced round brackets () are not allowed and string literals can't be arbitrarily long, so periodically interrupt with )+R(
string opencl_c_container() { return R( // ########################## begin of OpenCL C code ####################################################################



)+R(kernel void add_kernel(global float* A, global float* B, global float* C) { // equivalent to "for(uint n=0u; n<N; n++) {", but executed in parallel
	const uint n = get_global_id(0);
	C[n] = A[n]+B[n];
}

)+R(float3 reflect(const float2 rayDir, const float2 normalDir)
{
	float2 reflectedDir = rayDir + 2.0f * dot(rayDir, -normalDir) * normalDir;
	float3 result = (float3)(reflectedDir.x, reflectedDir.y, 0.0f);
	return result; 
}

)+R(float3 refract(const float2 rayDir, float2 surfaceNormal, float n1, float n2) // eta = n1/n2, n1 is the refractive index of the medium the ray comes from, n2 is the refractive index of the medium the ray goes into
{
	float eta = n1 / n2; // eta is the ratio of refractive indices
	float cosTheta1 = dot(rayDir, -surfaceNormal);
	if (cosTheta1 < 0.0f)
	{
		eta = 1 / eta; // Swap n1 and n2
		surfaceNormal = -surfaceNormal; // flip normal it must point towards the side where the light is coming from
		cosTheta1 = -cosTheta1; // Ensure cosTheta1 is positive for entering ray
	}
	 float sinTheta2Squared = eta * eta * (1.0f - cosTheta1 * cosTheta1);
	if (sinTheta2Squared > 1.0f) return reflect(rayDir,surfaceNormal); // total internal reflection
	const float cosTheta2 = sqrt(1.0f - sinTheta2Squared);

	// Fresnel equations for unpolarised light
	const float rs = (n1 * cosTheta1 - n2 * cosTheta2) / (n1 * cosTheta1 + n2 * cosTheta2);
	const float rp = (n2 * cosTheta1 - n1 * cosTheta2) / (n2 * cosTheta1 + n1 * cosTheta2);

	const float Rs = rs * rs;
	const float Rp = rp * rp;

	const float R = 0.5 * (Rs + Rp); // Unpolarised reflectance
	const float  T = 1.0 - R;


	float2 refractedDir = eta * rayDir + (eta * cosTheta1 - cosTheta2) * surfaceNormal;
	return (float3)(refractedDir.x, refractedDir.y, T); // return the new ray direction and Transmission coefficient
}
) + R(float3 only_refract(const float2 rayDir, float2 surfaceNormal, const float n1, const float n2, const float cosTheta1)
{
	const float eta = n1 / n2; // eta is the ratio of refractive indices
	float sinTheta2Squared = eta * eta * (1.0f - cosTheta1 * cosTheta1);
	float2 refractedDir = (float2)(0.0f,0.0f);
	

	if (sinTheta2Squared > 1.0f)
	{
		return (float3)(0.0f, 0.0f, 0.0f); // No transmission, TIR, return zero vector as no light is transmitted and therefore no light is refracted 
	}

	const float cosTheta2 = sqrt(1.0f - sinTheta2Squared);

	// Fresnel equations for unpolarised light
	const float rs = (n1 * cosTheta1 - n2 * cosTheta2) / (n1 * cosTheta1 + n2 * cosTheta2);
	const float rp = (n2 * cosTheta1 - n1 * cosTheta2) / (n2 * cosTheta1 + n1 * cosTheta2);

	const float Rs = rs * rs;
	const float Rp = rp * rp;

	const float R = 0.5 * (Rs + Rp); // Unpolarised reflectance
	const float  T = 1.0 - R;


	refractedDir = eta * rayDir + (eta * cosTheta1 - cosTheta2) * surfaceNormal;
	return (float3)(refractedDir.x, refractedDir.y, T); // return the new ray direction and Transmission coefficient
}
)+R( float2 only_reflect(const float2 rayDir, const float2 normalDir)
{
	float2 reflectedDir = rayDir + 2.0f * dot(rayDir, -normalDir) * normalDir;
	return reflectedDir;
}
)+R(float3 boundary_logic(const float2 rayDir, const float2 normalDir, const float n1, const float n2)
{ 
	const float3 refractedRay = refract(rayDir, normalDir, n1, n2);
	return refractedRay; // return the new ray direction
}

// COLLISION LOGIC DEFINITELY WORKS
// WHAT TO REMOVE
// WE DONT NEED ENTITYINDEXHIT TO RETURN BACK
// WHAT TO ADD
// some way of transferring eta values/ refractive indices 
// Send back if a ray is finished processing
// Spawn new rays from white light 
)+R(kernel void ray_entity_intersection(
global const float* rayOriginsX,
global const float* rayOriginsY,
global  float* rayDirsX,
global  float* rayDirsY,
global const float* entityVerticesX,
global const float* entityVerticesY,
global const uint * entityVertexCount,
const uint numEntities,
global float * collisionPointsX,
global float* collisionPointsY,
global int* entityIndexHit,
global  float* refracIndices,
global bool* whiteLight,
global float* createRayAtX,
global float* createRayAtY,
global float* createRayWithDirX,
global float* createRayWithDirY,
global bool* finishedProcessing,
global  uint* screenBounds,
global float* transmissionCoefficients
) {
	const uint n = get_global_id(0);

	float2 origin = (float2)(rayOriginsX[n], rayOriginsY[n]);
	float2 dir = (float2)(rayDirsX[n], rayDirsY[n]);

	float minT = 1e20f;
	int hitEntity = -1;
	
	float2 finalEdge;

	// Iterate over all entities
	for (uint e = 0; e < numEntities; e++)
	{
		// Compute starting offset for this entity
		uint start = 0;
		for (uint i = 0; i < e; ++i) {
			start += entityVertexCount[i];
		}

		uint count = entityVertexCount[e];

		for (uint i = 0; i < count; ++i) {
			uint j = start + i;
			uint k = start + ((i + 1) % count);

			float2 a = (float2)(entityVerticesX[j], entityVerticesY[j]);
			float2 b = (float2)(entityVerticesX[k], entityVerticesY[k]);

			float2 edge = b - a;
			float2 pa = a - origin;
			float det = dir.x * edge.y - dir.y * edge.x;

			if (fabs(det) < 1e-6f) continue;

			float t = (pa.x * edge.y - pa.y * edge.x) / det;
			float u = (pa.x * dir.y - pa.y * dir.x) / det;

			if (t > 0.0f && u >= 0.0f && u <= 1.0f && t < minT) {
				minT = t;
				hitEntity = e;
				finalEdge = edge;
			}
		}
	}

	if (hitEntity != -1) {
		float2 collisionPoint = origin + minT * dir;
		entityIndexHit[n] = hitEntity; // Store the index of the entity hit
		collisionPointsX[n] = collisionPoint.x;
		collisionPointsY[n] = collisionPoint.y;
		// Calculate the side vector
		if (whiteLight[n])
		{
			// TO DO: send a signal to CPU process to create rays ideas:
			// 1. Create new rays with origin just before collision point and the same direction
			// 2. When they hit the boundary they will refract and continue
			// Tell the program to create rays just before the collision point 
			createRayAtX[n] = collisionPoint.x - rayDirsX[n];
			createRayAtY[n] = collisionPoint.y - rayDirsY[n]; 	

			createRayWithDirX[n] = rayDirsX[n];
			createRayWithDirY[n] = rayDirsY[n];

			finishedProcessing[n] = true; // Mark this ray as finished processing

			return; 
		}

		// Assume going into air always for now but we can improve this
		float2 surfaceNormal = normalize((float2)(-finalEdge.y, finalEdge.x)); // Perpendicular to the edge


		float3 refractedRay = boundary_logic(dir, surfaceNormal, refracIndices[n] , 1.0f); // Get the new ray direction after refraction
		rayDirsX[n] = refractedRay.x; // Update the ray direction
		rayDirsY[n] = refractedRay.y; // Update the ray direction
		transmissionCoefficients[n] = refractedRay.z; // Store the transmission coefficient

		finishedProcessing[n] = false;

	}
	else
	{
		// --- Check intersection with screen bounds ---
		float width = (float)screenBounds[0];
		float height = (float)screenBounds[1];

		// Define screen rectangle as 4 edges (in CCW order)
		float2 screenVerts[4] = {
			(float2)(0.0f, 0.0f),
			(float2)(width, 0.0f),
			(float2)(width, height),
			(float2)(0.0f, height)
		};

		for (int i = 0; i < 4; ++i) 
		{
			int j = (i + 1) % 4;
			float2 a = screenVerts[i];
			float2 b = screenVerts[j];
			float2 edge = b - a;
			float2 pa = a - origin;
			float det = dir.x * edge.y - dir.y * edge.x;

			if (fabs(det) < 1e-6f) continue;

			float t = (pa.x * edge.y - pa.y * edge.x) / det;
			float u = (pa.x * dir.y - pa.y * dir.x) / det;

			if (t > 0.0f && u >= 0.0f && u <= 1.0f && t < minT) 
			{
				minT = t;
				finalEdge = edge;
			}
		}

		// FInished processing should be TRUE 
		entityIndexHit[n] = -1; // No entity hit
		collisionPointsX[n] = origin.x + minT * dir.x; // Collision point on screen bounds
		collisionPointsY[n] = origin.y + minT * dir.y; // Collision point on screen bounds
		finishedProcessing[n] = true; // Mark this ray as finished processing
	}

}

)+R(float getMaterialIndex(
global const float* sellmeierA,
global const float* sellmeierB,
global const int* entitySellmeierProfiles,
int entityIndex,
float wavelength
) {
	// wavelength in micrometers 
	float lambdaSq = wavelength * wavelength;

	int profileIndex = entitySellmeierProfiles[entityIndex];
	int offset = profileIndex * 3;

	float A0 = sellmeierA[offset];
	float A1 = sellmeierA[offset + 1];
	float A2 = sellmeierA[offset + 2];

	float B0 = sellmeierB[offset];
	float B1 = sellmeierB[offset + 1];
	float B2 = sellmeierB[offset + 2];

	float numerator0 = A0 * lambdaSq;
	float numerator1 = A1 * lambdaSq;
	float numerator2 = A2 * lambdaSq;

	float denominator0 = lambdaSq - B0;
	float denominator1 = lambdaSq - B1;
	float denominator2 = lambdaSq - B2;

	float nSquared = 1.0f +
		(numerator0 / denominator0) +
		(numerator1 / denominator1) +
		(numerator2 / denominator2);

	return sqrt(nSquared);
}
) + R(kernel void ray_fresnel_mode(
global const float* rayOriginsX,
global const float* rayOriginsY,
global float* rayDirsX,
global float* rayDirsY,
global float* reflectedRayDirsX,
global float* reflectedRayDirsY,
global const float* edgeA_X,
global const float* edgeA_Y,
global const float* edgeB_X,
global const float* edgeB_Y,
global const int* edgeEntityIndices,
const uint edgeCount, 
global float* collisionPointsX,
global float* collisionPointsY,
global int* entityIndexHit,
global float* refracIndices,
global bool* whiteLight,
global bool* finishedProcessing,
constant const uint * screenBounds,
global float* transmissionCoefficients,
global const float* sellmeierCoefficientsA,
global const float* sellmeierCoefficientsB,
global const int* entitySellmeierProfiles,
global const float* rayWavelengths
) {
	const uint n = get_global_id(0);

	float2 origin = (float2)(rayOriginsX[n], rayOriginsY[n]);
	float2 rayDir = fast_normalize((float2)(rayDirsX[n], rayDirsY[n]));

	float minT = 1e20f;
	int hitEntity = -1;

	float2 finalEdge;

	// Loop over all edges
	for (uint i = 0; i < edgeCount; ++i) {
		float2 a = (float2)(edgeA_X[i], edgeA_Y[i]);
		float2 b = (float2)(edgeB_X[i], edgeB_Y[i]);
		float2 edge = b - a;
		float2 pa = a - origin;

		float det = rayDir.x * edge.y - rayDir.y * edge.x;
		if (fabs(det) < 1e-6f) continue;

		float t = (pa.x * edge.y - pa.y * edge.x) / det;
		float u = (pa.x * rayDir.y - pa.y * rayDir.x) / det;

		if (t > 0.0f && u >= 0.0f && u <= 1.0f && t < minT) {
			minT = t;
			hitEntity = edgeEntityIndices[i];
			finalEdge = edge;
		}
	}

	if (hitEntity != -1) {
		float2 collisionPoint = origin + minT * rayDir;
		entityIndexHit[n] = hitEntity;
		collisionPointsX[n] = collisionPoint.x;
		collisionPointsY[n] = collisionPoint.y;


		float2 surfaceNormal = normalize((float2)(-finalEdge.y, finalEdge.x));
		if (entitySellmeierProfiles[hitEntity] == -1)
		{
			// Then we are dealing with a mirror or something that doesn't have a refractive index, so just reflect the ray
			float2 reflectedRay = only_reflect(rayDir, surfaceNormal);
			reflectedRayDirsX[n] = reflectedRay.x;
			reflectedRayDirsY[n] = reflectedRay.y;
			finishedProcessing[n] = false;
			return;
		}
		float n1 = getMaterialIndex(sellmeierCoefficientsA, sellmeierCoefficientsB, entitySellmeierProfiles, hitEntity, rayWavelengths[n] * 1e-3f);

		float n2 = 1.0f;
		float cosTheta1 = dot(rayDir, -surfaceNormal);

		if (cosTheta1 < 0.0f)
		{
			float temp = n2; n2 = n1; n1 = temp;
			surfaceNormal = -surfaceNormal;
			cosTheta1 = -cosTheta1;
		}

		if (whiteLight[n])
		{
			// For the amount of white rays (most of the time) even if we don't use the reflected Dir it should be trivial
			float2 reflectedRay = only_reflect(rayDir, surfaceNormal);
			reflectedRayDirsX[n] = reflectedRay.x;
			reflectedRayDirsY[n] = reflectedRay.y;
			// We set it here but we can unset it if it hits a mirror entity 
			finishedProcessing[n] = true;
			return;
		}



		float3 refractedRay = only_refract(rayDir, surfaceNormal, n1, n2, cosTheta1);
		rayDirsX[n] = refractedRay.x;
		rayDirsY[n] = refractedRay.y;
		transmissionCoefficients[n] = refractedRay.z;

		float2 reflectedRay = only_reflect(rayDir, surfaceNormal);
		reflectedRayDirsX[n] = reflectedRay.x;
		reflectedRayDirsY[n] = reflectedRay.y;
		finishedProcessing[n] = false;

		refracIndices[n] = n2; // update for next interaction

	}
	else {
		// Fallback: check against screen bounds
		float left = (float)screenBounds[0];
		float top = (float)screenBounds[1];
		float width = (float)screenBounds[2];
		float height = (float)screenBounds[3];

		float2 screenVerts[4] = {
			(float2)(left, top),
			(float2)(left , top + height),
			(float2)(left + width, top + height),
			(float2)(left + width , top)
		};

		for (int i = 0; i < 4; ++i) {
			int j = (i + 1) % 4;
			float2 a = screenVerts[i];
			float2 b = screenVerts[j];
			float2 edge = b - a;
			float2 pa = a - origin;
			float det = rayDir.x * edge.y - rayDir.y * edge.x;

			if (fabs(det) < 1e-12f) continue;

			float t = (pa.x * edge.y - pa.y * edge.x) / det;
			float u = (pa.x * rayDir.y - pa.y * rayDir.x) / det;

			if (t > 0.0f && u >= 0.0f && u <= 1.0f && t < minT) {
				minT = t;
				finalEdge = edge;
			}
		}

		entityIndexHit[n] = -1;
		collisionPointsX[n] = origin.x + minT * rayDir.x;
		collisionPointsY[n] = origin.y + minT * rayDir.y;
		finishedProcessing[n] = true;
	}
}

);} // ############################################################### end of OpenCL C code #####################################################################