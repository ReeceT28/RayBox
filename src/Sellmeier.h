#pragma once
#include <vector>
#include <string>
#include <unordered_map>
class Sellmeier
{
private:
	// Initialise with values for crown glass - by putting in class makes it easier to update these if we want to add alternative materials
	std::vector<double> sellmeierCoefficientsA;
	std::vector<double> sellmeierCoefficientsB;
	// Unordered is generally faster for lookups than ordered map which is weird but works 
	std::unordered_map<float, float> wavelengthToIndex;
	std::string tag;
public:
	Sellmeier(const std::vector<double>& aCoefficients = { 1.03961212, 0.231792344, 1.01146945 },
		const std::vector<double>& bCoefficients = { 0.00600069867, 0.0200179144, 103.560653 },
		std::string tag = "Crown Glass");
	double calculateN(const double);
	void calculateWavelengthToIndex(float startWavelength, float step, int resolution);
	std::string getTag() const
	{
		return tag;
	}
	std::unordered_map<float, float> getWavelengthToIndex() const
	{
		return wavelengthToIndex;
	}

	float getIndexAtWavelength(float wavelength) const
	{
		auto it = wavelengthToIndex.find(wavelength);
		if (it != wavelengthToIndex.end())
		{
			return it->second; 
		}
		// Only happens rarely so fine
		auto closest = wavelengthToIndex.begin();
		float minDiff = std::abs(closest->first - wavelength);

		for (auto& [wl, index] : wavelengthToIndex)
		{
			float diff = std::abs(wl - wavelength);
			if (diff < minDiff)
			{
				minDiff = diff;
				closest = wavelengthToIndex.find(wl);
			}
		}

		return closest->second;
	}

	std::vector<double> getCoefficientsA() const
	{
		return sellmeierCoefficientsA;
	}
	std::vector<double> getCoefficientsB() const
	{
		return sellmeierCoefficientsB;
	}
};
