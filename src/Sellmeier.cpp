#include "Sellmeier.h"
#include <cmath>
#include <string>
Sellmeier::Sellmeier(const std::vector<double>& aCoefficients ,
	const std::vector<double>& bCoefficients,
	std::string tag ): sellmeierCoefficientsA{aCoefficients}, sellmeierCoefficientsB{bCoefficients}, tag(tag)
{
}
double Sellmeier::calculateN(const double waveLength)
{
	double lambdaSquared = waveLength * waveLength;
	double n = 1.0;
	for (int i = 0; i < 3; i++)
	{
		n += (sellmeierCoefficientsA[i] * lambdaSquared) / (lambdaSquared - sellmeierCoefficientsB[i]);
	}
	return std::sqrt(n);
}

void Sellmeier::calculateWavelengthToIndex(float startWavelength, float step, int resolution)
{
	for (int i = 0; i < resolution; ++i)
	{
		float wavelength = startWavelength + i * step;
		float n = calculateN(wavelength * 1e-3f); // Convert nm to µm
		wavelengthToIndex[wavelength] = n;
	}
}