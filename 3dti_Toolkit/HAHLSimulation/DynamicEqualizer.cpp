/*
* \class CDynamicEqualizer
*
* \brief Declaration of CDynamicEqualizer class that considers multiple levels of equalization.
*        Depending on the level of the signal, it chooses an equalization level or another.
*        It can applies the nearest equalization level or interpolate between two equalization levels.
*
* Class to handle a set of parallel digital filters whose contributions are added
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
#include <cmath>
#include <HAHLSimulation/DynamicEqualizer.h>
#include <Common/ErrorHandler.h>

//////////////////////////////////////////////
// CONSTRUCTOR/DESTRUCTOR

namespace HAHLSimulation {

	CDynamicEqualizer::CDynamicEqualizer()
	{
		updateBandGainsIsPending = false;

		attack_ms = 100;
		release_ms = 100;

		level_db = 0;

		compressionPercentage = 100;

		overalOffset_dB = 0;

		maxGain_dB = 100;
		minGain_dB = -100;
	}

	//////////////////////////////////////////////
	void CDynamicEqualizer::Setup(int samplingRate, int numLevels, float iniFreq_Hz, int bandsNumber, int octaveBandStep, float Q_BPF)
	{
		envelopeDetector.Setup(samplingRate);

		updateBandGainsIsPending = false;

		bandFrequencies_Hz.clear();
		levels.clear();

		float f = iniFreq_Hz;
		float freqStep = std::pow(2, 1.0f / (float)octaveBandStep);

		for (int c = 0; c < bandsNumber; c++)
		{
			bandFrequencies_Hz.push_back(f);

			filterBank.AddFilter()->Setup(samplingRate, f, Q_BPF, Common::T_filterType::BANDPASS);

			f *= freqStep;
		}

		CEqLevel level;

		level.bands.resize(bandsNumber);
		level.threshold = 0;

		for (int c = 0; c < bandsNumber; c++)
			level.bands[c] = 0;

		for (int c = 0; c < numLevels; c++)
			levels.push_back(level);
	}

	//////////////////////////////////////////////
	void CDynamicEqualizer::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		// 1.- Get the RMS value of the signal
		float currentLevel = 0;

		envelopeDetector.SetAttackTime(attack_ms);
		envelopeDetector.SetReleaseTime(release_ms);

		int size = inputBuffer.size();
		for (int c = 0; c + 1 < size; c++)
			currentLevel = envelopeDetector.ProcessSample(inputBuffer[c]);

		level_db = 20 * std::log10(currentLevel);

		// It can be > 0 because of the applied gain (volume or equalizers) 
		if (level_db > 0)
			level_db = 0;

		updateBandGainsIsPending = true;

		// 2.- Get the closest level to the RMS values for each channel
		float closestLevel_dB = std::numeric_limits<float>::max();

		CEqLevel *closestLevelL = NULL;

		for (int l = 0; l < levels.size(); l++)
		{
			CDynamicEqualizer::CEqLevel &level = levels[l];

			if (std::abs(level.threshold - level_db) < closestLevel_dB)
			{
				closestLevel_dB = std::abs(level.threshold - level_db);
				closestLevelL = &level;
			}
		}

		// 3.- If using...
		if (levelsInterpolation)
		{
			// 3.1.- ...If using interpolation of levels, we get the second closest level
			closestLevel_dB = std::numeric_limits<float>::max();

			CEqLevel *secondClosestLevelL = NULL;
			CEqLevel *secondClosestLevelR = NULL;

			for (int l = 0; l < levels.size(); l++)
			{
				CDynamicEqualizer::CEqLevel& level = levels[l];

				if (closestLevelL != &level && std::abs(level.threshold - level_db) < closestLevel_dB)
				{
					closestLevel_dB = std::abs(level.threshold - level_db);
					secondClosestLevelL = &level;
				}
			}

			// At this point we have the closest level and the second closest level for each channel

			if (closestLevelL != NULL && secondClosestLevelL != NULL)
			{
				float closestLevelL_db = closestLevelL->threshold;
				float secondClosestLevelL_db = secondClosestLevelL->threshold;

				bool aboveLevels = level_db > closestLevelL_db &&
					level_db > secondClosestLevelL_db;
				bool belowLevels = level_db < closestLevelL_db &&
					level_db < secondClosestLevelL_db;

				// Check if interpolation makes sense
				if (aboveLevels || belowLevels)
				{
					ApplyLevel(*closestLevelL, true);
				}
				else
				{
					float diff = std::abs(closestLevelL_db - secondClosestLevelL_db);
					if (diff > 0.0001f)
					{
						float min;
						CEqLevel *minLevel, *maxLevel;
						if (closestLevelL_db < secondClosestLevelL_db)
						{
							min = closestLevelL_db;
							minLevel = closestLevelL;
							maxLevel = secondClosestLevelL;
						}
						else
						{
							min = secondClosestLevelL_db;
							minLevel = secondClosestLevelL;
							maxLevel = closestLevelL;
						}

						float alpha = (level_db - min) / diff;

						if (alpha < 0) alpha = 0.0f;
						else if (alpha > 1) alpha = 1.0f;

						float oneMinusAlpha = 1.0f - alpha;

						for (int c = 0; c < closestLevelL->bands.size(); c++)
							SetFiltersBankBandGain_dB(c, GetCompressedGain_dB(*maxLevel, c) * alpha +
								GetCompressedGain_dB(*minLevel, c) * oneMinusAlpha);
					}
					else
						ApplyLevel(*closestLevelL, true);
				}
			}
		}
		// 3.2.- ...If not using interpolation of levels, we apply the gains of the closest level
		else
		{
			if (closestLevelL != NULL)  ApplyLevel(*closestLevelL, true);
		}

		filterBank.Process(inputBuffer, outputBuffer);
	}

	//////////////////////////////////////////////
	// Specifies the gain for each band in dB
	void CDynamicEqualizer::SetGains_dB(vector<float> gains_dB)
	{
		if (gains_dB.size() != filterBank.GetNumFilters())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "number of elements must agree ( gains_dB Vs number of filters in the bank)");
			return;
		}

		SET_RESULT(RESULT_OK, "");

		for (int c = 0; c < filterBank.GetNumFilters(); c++)
			filterBank.GetFilter(c)->SetGeneralGain(std::pow(10.0, gains_dB[c] / 20.0));
	}

	//--------------------------------------------------------------
	// Specifies the gain for each band in dB
	void CDynamicEqualizer::ResetGains_dB()
	{
		SET_RESULT(RESULT_OK, "");

		for (int c = 0; c < filterBank.GetNumFilters(); c++)
			filterBank.GetFilter(c)->SetGeneralGain(1);
	}

	//--------------------------------------------------------------
	void CDynamicEqualizer::SetFiltersBankBandGain_dB(int bandIndex, float gain_dB)
	{
		if (bandIndex < 0 || bandIndex >= filterBank.GetNumFilters())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return;
		}

		SET_RESULT(RESULT_OK, "");

		filterBank.GetFilter(bandIndex)->SetGeneralGain(std::pow(10.0, gain_dB / 20.0));
	}
	//--------------------------------------------------------------
	void CDynamicEqualizer::ApplyLevel(CEqLevel &level, bool applyCompression)
	{
		if (applyCompression)
		{
			for (int c = 0; c < level.bands.size(); c++)
				SetFiltersBankBandGain_dB(c, GetCompressedGain_dB(level, c));
		}
		else
		{
			for (int c = 0; c < level.bands.size(); c++)
				SetFiltersBankBandGain_dB(c, level.bands[c] + overalOffset_dB);
		}
	}
	//--------------------------------------------------------------
	void CDynamicEqualizer::SetUpdateLevelsIsPending()
	{
		updateBandGainsIsPending = true;
	}
	//--------------------------------------------------------------
	float CDynamicEqualizer::GetBandFrequency(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= bandFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return 0;
		}
		return bandFrequencies_Hz[bandIndex];
	}

	//////////////////////////////////////////////
	// Specifies the gain for a specific band
	void CDynamicEqualizer::SetLevelBandGain_dB(int levelIndex, int bandIndex, float gain_dB)
	{
		if (levelIndex < 0 || levelIndex >= levels.size() || bandIndex < 0 ||
			bandIndex >= levels[levelIndex].bands.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return;
		}

		levels[levelIndex].bands[bandIndex] = gain_dB;

		SetUpdateLevelsIsPending();
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetLevelBandGain_dB(int levelIndex, int bandIndex)
	{
		if (levelIndex < 0 || levelIndex >= levels.size() || bandIndex < 0 ||
			bandIndex >= levels[levelIndex].bands.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return 0.0f;
		}

		return levels[levelIndex].bands[bandIndex];
	}

	//////////////////////////////////////////////
	void CDynamicEqualizer::SetLevelThreshold(int levelIndex, float threshold_dBfs)
	{
		if (levelIndex < 0 || levelIndex >= levels.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return;
		}

		levels[levelIndex].threshold = threshold_dBfs;
	}
	//////////////////////////////////////////////
	float CDynamicEqualizer::GetLevelThreshold(int levelIndex)
	{
		if (levelIndex < 0 || levelIndex >= levels.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return 0.0f;
		}

		return levels[levelIndex].threshold;
	}

	//////////////////////////////////////////////

	void CDynamicEqualizer::SetCompressionPercentage(float percentage)
	{
		compressionPercentage = percentage;
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetCompressionPercentage()
	{
		return compressionPercentage;
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetCompressedGain_dB(int levelIndex, int bandIndex)
	{
		if (levelIndex < 0 || levelIndex >= levels.size())
			return 0;

		return GetCompressedGain_dB(levels[levelIndex], bandIndex);
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetCompressedGain_dB(CEqLevel &level, int bandIndex)
	{
		if (levels.size() == 0 || bandIndex < 0)
			return 0.0;

		float baseGain = 0;
		float gain = 0;
		float compressedGain = 0;

		if (bandIndex < levels[0].bands.size() && bandIndex < level.bands.size())
		{
			baseGain = levels[0].bands[bandIndex];
			gain = level.bands[bandIndex];

			compressedGain = baseGain + (compressionPercentage / 100.0f) * (gain - baseGain) + overalOffset_dB;
		}

		if (compressedGain > maxGain_dB) compressedGain = maxGain_dB;
		else if (compressedGain < minGain_dB) compressedGain = minGain_dB;

		return compressedGain;
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetOveralOffset_dB()
	{
		return overalOffset_dB;
	}

	//////////////////////////////////////////////

	void CDynamicEqualizer::SetOveralOffset_dB(float offset_dB)
	{
		overalOffset_dB = offset_dB;
	}

	//////////////////////////////////////////////

	void CDynamicEqualizer::SetMaxGain_dB(float maxGain_dB_)
	{
		maxGain_dB = maxGain_dB_;
	}

	//////////////////////////////////////////////

	void CDynamicEqualizer::SetMinGain_dB(float minGain_dB_)
	{
		minGain_dB = minGain_dB_;
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetMaxGain_dB()
	{
		return maxGain_dB;
	}

	//////////////////////////////////////////////

	float CDynamicEqualizer::GetMinGain_dB()
	{
		return minGain_dB;
	}
}// end namespace HAHLSimulation
