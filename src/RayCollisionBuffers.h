#pragma once
#include "Memory.h"
#include <vector>
#include <SFML/System.hpp>
#include "opencl.hpp"
#include <array>

struct RayData 
{
    float originX, originY;
    float dirX, dirY;
    bool finished;
    bool whiteLight;
    float refracIndex;
    sf::Color color;
    float wavelength;
};


class RayCollisionBuffers {
    std::vector<RayData> raysToAdd;  // Temporary storage for new rays before committing

public:
    static constexpr uint maxBufferSize = static_cast<const uint>(5e+5);
    // 10 million rays for floats this is 40MB - not too bad, even with 20 buffers like this we get 800MB VRAM usage which should be okay, if not lower this number 
    // To solve this problem I could implement dynamic buffer resizing however I believe this is slower???? Could make an std::vector for when overflows occur basically having a buffer for our buffers
    // ====================== ALL COUNTS POINT TO THE POSITION AFTER THE LAST ELEMENT IN THE BUFFER ==========================
    // Ray buffers
    uint currRayCount = 0; 
    Memory<float> rayDirsX, rayDirsY;
	Memory<float> reflectedRayDirsX, reflectedRayDirsY; // For reflected rays
    Memory<float> rayOriginsX, rayOriginsY;
    Memory<bool> finishedProcessing;
    Memory<bool> whiteLight;
	Memory<float> collisionPointsX, collisionPointsY; // Can be accessed with currRayCount as all rays need collision point but set to -1,-1 if no collision occurred (shouldn't be possible)
    // Buffers for Entity related things
    Memory<float> entityVerticesX, entityVerticesY;
    Memory<uint> entityVertexCount; 
    Memory<int> entityIndexHit; // Only for reading

    // Buffers only for reading, when white light hits something or some other even occurs we can work out a bunch of new rays to spawn
    // Then we can copy the contents of these buffers to the ray buffers 
    Memory<float> createRayAtX, createRayAtY;
    Memory<float> createRayWithDirX, createRayWithDirY;

    // Special values
    Memory<uint> screenBounds;
    Memory<float> refracIndices;
    Memory<float> wavelengths;
	Memory<float> transmissionCoefficients;

    Memory<float> edgeA_X, edgeA_Y;              // Edge start points (x, y)
    Memory<float> edgeB_X, edgeB_Y;              // Edge end points (x, y)
    Memory<int> edgeEntityIndices;               // Which entity each edge belongs to (index into prismEntities)
	std::vector < sf::Color> rayColours; // For rendering purposes, not used in the kernel

    Memory<float> sellmeierCoefficientsA;
	Memory<float> sellmeierCoefficientsB;
    Memory<int> entitySellmeierProfiles;


    // Constructor to initialise buffers with the correct device
    RayCollisionBuffers(Device& device)
        : rayDirsX(device,maxBufferSize ),
        rayDirsY(device, maxBufferSize),
        rayOriginsX(device, maxBufferSize),
        rayOriginsY(device, maxBufferSize),
        collisionPointsX(device, maxBufferSize),
        collisionPointsY(device, maxBufferSize),
        refracIndices(device, maxBufferSize),
        whiteLight(device, maxBufferSize),
        createRayAtX(device, maxBufferSize),
        createRayAtY(device, maxBufferSize),
        createRayWithDirX(device, maxBufferSize),
        createRayWithDirY(device, maxBufferSize),
        finishedProcessing(device, maxBufferSize),
        screenBounds(device, 2),
        entityVerticesX(device, maxBufferSize),
        entityVerticesY(device, maxBufferSize),
        entityVertexCount(device, maxBufferSize),
        entityIndexHit(device, maxBufferSize),
        wavelengths(device, maxBufferSize),
        transmissionCoefficients(device, maxBufferSize),
		reflectedRayDirsX(device, maxBufferSize),
		reflectedRayDirsY(device, maxBufferSize),
        edgeA_X(device, maxBufferSize),
        edgeA_Y(device, maxBufferSize),
        edgeB_X(device, maxBufferSize),
        edgeB_Y(device, maxBufferSize),
        edgeEntityIndices(device,maxBufferSize),
        rayColours(maxBufferSize),
		sellmeierCoefficientsA(device, maxBufferSize),
        sellmeierCoefficientsB(device, maxBufferSize),
		entitySellmeierProfiles(device, maxBufferSize)
    {}
    
    void updateRayOrigin(float x, float y, uint index )
	{
		if (index >= maxBufferSize) {
			throw std::out_of_range("Index out of range for ray origins buffer");
		}
		rayOriginsX[index] = x;
		rayOriginsY[index] = y;
    }
    void addRayOrigin(float x, float y)
    {
		rayOriginsX[currRayCount] = x;
		rayOriginsY[currRayCount] = y;
		currRayCount++;
    }
    void createRayImmediately(const float x, const float y,const float dirX,const float dirY,const bool isWhiteLight = false,const sf::Color &rayColor = sf::Color(255,255,255,255), const float refractiveIndice = 1.0f)
    {
		rayOriginsX[currRayCount] = x;
		rayOriginsY[currRayCount] = y;
		rayDirsX[currRayCount] = dirX;
		rayDirsY[currRayCount] = dirY;
		finishedProcessing[currRayCount] = false;
		whiteLight[currRayCount] = isWhiteLight;
		refracIndices[currRayCount] = refractiveIndice;
		rayColours[currRayCount] = rayColor; // Store the color for rendering
		currRayCount++;
    }
    void removeRay()
    {
        if (currRayCount > 0)
        {
            currRayCount--;
        }
        else
        {
            std::cerr << "No rays to remove!" << std::endl;
        }
    }
    void clearBuffers()
    {
        currRayCount = 0;
        rayDirsX.reset();
		rayDirsY.reset();
		rayOriginsX.reset();
		rayOriginsY.reset();
		collisionPointsX.reset();
		collisionPointsY.reset();
		refracIndices.reset();
		whiteLight.reset();
		createRayAtX.reset();
		createRayAtY.reset();
		createRayWithDirX.reset();
		createRayWithDirY.reset();
		finishedProcessing.reset();
		screenBounds.reset();
		entityVerticesX.reset();
		entityVerticesY.reset();
		entityVertexCount.reset();
		entityIndexHit.reset();
		rayColours.clear(); // Clear the ray colors vector

	}

    void createRay(const RayData& ray) 
    {
        raysToAdd.push_back(ray);
    }

    void commitNewRays()
    {
        for (const auto& ray : raysToAdd) 
        {
            if (currRayCount >= maxBufferSize)
            {
				std::cerr << "Ray buffer is full! Cannot add more rays." << std::endl;
                break;
            }
            rayOriginsX[currRayCount] = ray.originX;
            rayOriginsY[currRayCount] = ray.originY;
            rayDirsX[currRayCount] = ray.dirX;
            rayDirsY[currRayCount] = ray.dirY;
            finishedProcessing[currRayCount] = ray.finished;
            whiteLight[currRayCount] = ray.whiteLight;
            refracIndices[currRayCount] = ray.refracIndex;
            rayColours[currRayCount] = ray.color;
            wavelengths[currRayCount] = ray.wavelength;
            //Set all default data
            collisionPointsX[currRayCount] = -1.0f;
            collisionPointsY[currRayCount] = -1.0f;
            reflectedRayDirsX[currRayCount] = 0.0f; // Reset reflected ray direction
            reflectedRayDirsY[currRayCount] = 0.0f; // Reset reflected ray direction
            entityIndexHit[currRayCount] = 0;
			finishedProcessing[currRayCount] = false; 
            currRayCount++;
        }
        raysToAdd.clear();
    }

};