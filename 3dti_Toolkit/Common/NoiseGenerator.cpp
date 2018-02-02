/**
* \class CNoiseGenerator
*
* \brief Implementation of CNoiseGenerator class. This class implements a gaussian noise generator. White noise can be generated,
* but also autocorrelated noise with a configurable low pass filter. 
*
* \date	September 2017
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2018
*
* \b Licence: This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#include "Common/NoiseGenerator.h"

namespace Common {

	/////////////////////////////////////////////////////
	// Setup the noise generator
	void CNoiseGenerator::Setup(float deviation)
	{
		// Init random number engine with random seed
		std::random_device randomSeed;
		randomEngine = std::default_random_engine(randomSeed());

		// Setup normal distribution post-processor
		standardDeviation = deviation;
		//normalDistribution = std::normal_distribution<float>(DEFAULT_GAUSSIAN_MEAN, standardDeviation);
		normalDistribution = std::normal_distribution<float>(DEFAULT_GAUSSIAN_MEAN, 1.0f);

		// By default, disable autocorrelation filter and truncation
		doAutocorrelation = false;
	}


	/////////////////////////////////////////////////////
	// Set standard deviation of gaussian distribution
	void CNoiseGenerator::SetDeviation(float deviation)
	{
		standardDeviation = deviation;
		//normalDistribution = std::normal_distribution<float>(DEFAULT_GAUSSIAN_MEAN, standardDeviation);
		normalDistribution = std::normal_distribution<float>(DEFAULT_GAUSSIAN_MEAN, 1.0f);
	}

	/////////////////////////////////////////////////////
	// Enable autocorrelation filter
	void CNoiseGenerator::EnableAutocorrelationFilter()
	{
		doAutocorrelation = true;
	}

	/////////////////////////////////////////////////////
	// Disable autocorrelation filter
	void CNoiseGenerator::DisableAutocorrelationFilter()
	{
		doAutocorrelation = false;
	}

	/////////////////////////////////////////////////////
	// Setup parameters of autocorrelation filter
	void CNoiseGenerator::SetupAutocorrelationFilter(float samplingRate, float cutoff, float q)
	{
		autocorrelationQ = q;
		autocorrelationFilter.Setup(samplingRate, cutoff, q, Common::T_filterType::LOWPASS);
	}

	/////////////////////////////////////////////////////
	// Set cutoff frequency of low-pass autocorrelation filter
	void CNoiseGenerator::SetAutocorrelationFilterCutoff(float cutoff)
	{
		autocorrelationFilter.SetCoefficients(cutoff, autocorrelationQ, T_filterType::LOWPASS);
	}

	/////////////////////////////////////////////////////
	// Process and generate an output buffer
	void CNoiseGenerator::Process(CMonoBuffer<float> & outputBuffer)
	{
		// Check errors
		if (outputBuffer.size() <= 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to generate noise into an empty output buffer");
			return;
		}

		// Generate random values with gaussian distribution
		for (int i = 0; i < outputBuffer.size(); i++)
		{
			float randomValue;
			randomValue = normalDistribution(randomEngine);
			randomValue = randomValue * standardDeviation;
			outputBuffer[i] = randomValue;
		}

		// Apply autocorrelation filter, if enabled
		if (doAutocorrelation)
		{
			autocorrelationFilter.Process(outputBuffer);
		}
	}
} //end namespace Common

