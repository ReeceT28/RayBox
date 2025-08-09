#pragma once
#include <vector>
#include "Sellmeier.h"
#include <memory>
#include <string>

class SellmeierManager
{
	std::vector<std::unique_ptr<Sellmeier>> sellmeierProfiles;
public:
	// Constructor
	SellmeierManager() = default;
	// Destructor
	~SellmeierManager() = default;
	void addProfile(const std::vector<double>& aCoefficients, const std::vector<double>& bCoefficients, std::string tag)
	{
		sellmeierProfiles.push_back(std::make_unique<Sellmeier>(aCoefficients, bCoefficients, tag));
	}
	Sellmeier* getSellmeier(const std::string& tag)
	{
		for (const auto& profile : sellmeierProfiles)
		{
			if (profile->getTag() == tag)
			{
				return profile.get();
			}
		}
		return nullptr; // Return nullptr if no profile with the given tag is found
	}
	// Get all profiles
	std::vector<std::unique_ptr<Sellmeier>>& getProfiles()
	{
		return sellmeierProfiles;
	}
};