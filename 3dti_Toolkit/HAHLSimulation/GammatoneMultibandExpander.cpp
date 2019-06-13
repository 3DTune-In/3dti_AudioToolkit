/*
* \class CGammatoneMultibandExpander
*
* \brief Implementation of CGammatoneMultibandExpander class. This class implements a multiband equalizer where each band has an
*	independent envelope follower and expander
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
#include <HAHLSimulation/GammatoneMultibandExpander.h>
#include <Common/ErrorHandler.h>
#include <iostream>
#include <iomanip>

namespace HAHLSimulation {
	void CGammatoneMultibandExpander::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, int filtersPerBand, bool filterGrouping)
	{
		setupDone = false;
		
		if ((filtersPerBand % 2) == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Filters per band for multiband expander must be an odd number.");
			return;
		}

		// Internal attributes cleaning
		octaveBandFrequencies_Hz.clear();
		gammatoneExpanderBandFrequencies_Hz.clear();
		octaveBandGains_dB.clear();
		gammatoneLowerBandFactors.clear();
		gammatoneHigherBandFactors.clear();
		gammatoneLowerBandIndices.clear();
		gammatoneHigherBandIndices.clear();
		perGroupBandExpanders.clear();
		perFilterGammatoneBandExpanders.clear();

		// Setup equalizer	
		float bandsPerOctave = 1.0f;	// Currently fixed to one band per octave, but could be a parameter
		float bandFrequencyStep = std::pow(2, 1.0f / (float)bandsPerOctave);
		float bandFrequency = iniFreq_Hz;
		float filterFrequencyStep = std::pow(2, 1.0f / (bandsPerOctave*filtersPerBand));
		float filterFrequency = bandFrequency / ((float)(trunc(filtersPerBand / 2))*filterFrequencyStep);

		// Compute Q for all filters
		float octaveStep = 1.0f / ((float)filtersPerBand * bandsPerOctave);
		float octaveStepPow = std::pow(2.0f, octaveStep);
		float Q_BPF = std::sqrt(octaveStepPow) / (octaveStepPow - 1.0f);

		for (int band = 0; band < bandsNumber; band++)
		{

			// Add band to list of frequencies and gains
			octaveBandFrequencies_Hz.push_back(bandFrequency);
			octaveBandGains_dB.push_back(0.0f);
			bandFrequency *= bandFrequencyStep;

			// Setup per-group expanders
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			perGroupBandExpanders.push_back(expander);

			// Create atenuation for band
			octaveBandAttenuations.push_back(0.0f);
		}
		
		// Setup Gammatone filterbank
		gammatoneFilterBank.RemoveFilters();
		gammatoneFilterBank.SetSamplingFreq(samplingRate);
		gammatoneFilterBank.InitWithFreqRangeOverlap(20, 20000, 0.0, Common::CGammatoneFilterBank::EAR_MODEL_DEFAULT);

		for (int filterIndex = 0; filterIndex < gammatoneFilterBank.GetNumFilters(); filterIndex++)
		{
			// Setup Gammatone expanders
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			perFilterGammatoneBandExpanders.push_back(expander);

			// Add frequency to frequency list
			float filterFrequency = gammatoneFilterBank.GetFilter(filterIndex)->GetCenterFrequency();
			gammatoneExpanderBandFrequencies_Hz.push_back(filterFrequency);

			// Add lower and higher octave band indices for this frequency
			int lowerBandIndex, higherBandIndex;
			float lowerBandFrequency = GetLowerOctaveBandFrequency(filterFrequency, lowerBandIndex);
			gammatoneLowerBandIndices.push_back(lowerBandIndex);
			float higherBandFrequency = GetHigherOctaveBandFrequency(filterFrequency, higherBandIndex);
			gammatoneHigherBandIndices.push_back(higherBandIndex);
			
			// Add lower and higher octave band factors for this frequency
			float frequencyDistance = higherBandFrequency - lowerBandFrequency;
			gammatoneLowerBandFactors.push_back(lowerBandFrequency <= 20.0f ? -1.0f : ((higherBandFrequency - filterFrequency) / frequencyDistance));
			gammatoneHigherBandFactors.push_back(higherBandFrequency >= 20000.0f ? -1.0f : ((filterFrequency - lowerBandFrequency) / frequencyDistance));
		}

		octaveBandFilterGrouping = filterGrouping;
		setupDone = true;
	}


	void CGammatoneMultibandExpander::GetOctaveBandGammatoneFiltersFirstAndLastIndex(int bandIndex, int &firstInternalBand, int &lastInternalBand)
	{

		float thisBandFrequency = octaveBandFrequencies_Hz[bandIndex];
		float previousBandFrequency = bandIndex == 0 ? 0 : octaveBandFrequencies_Hz[bandIndex - 1];
		float posteriorBandFrequency = bandIndex == octaveBandFrequencies_Hz.size() - 1 ? 30000 : octaveBandFrequencies_Hz[bandIndex + 1];

		float lowerThreshold = bandIndex > 0 ? (thisBandFrequency + previousBandFrequency) / 2.0f : 0.0f;
		float upperThreshold = bandIndex < octaveBandFrequencies_Hz.size() - 1 ? (thisBandFrequency + posteriorBandFrequency) / 2.0f : 30000;

		bool firstInternalBandFound = false;

		for (int i = 0; i < gammatoneExpanderBandFrequencies_Hz.size(); i++)
		{
			if (!firstInternalBandFound)
			{
				if (gammatoneExpanderBandFrequencies_Hz[i] > lowerThreshold)
				{
					firstInternalBand = i;
					firstInternalBandFound = true;
				}
				continue;
			}
			
			if (gammatoneExpanderBandFrequencies_Hz[i] > upperThreshold)
			{
				lastInternalBand = i - 1;
				break;
			}
		}
	}

	void CGammatoneMultibandExpander::CleanAllBuffers()
	{
		for (int i = 0; i < gammatoneFilterBank.GetNumFilters(); i++)
		{
			CMonoBuffer<float> emptyBuffer = CMonoBuffer<float>(128, 0.0f);
			gammatoneFilterBank.GetFilter(i)->Process(emptyBuffer);
		}
	}


	void CGammatoneMultibandExpander::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		// Initialization of output buffer
		outputBuffer.Fill(outputBuffer.size(), 0.0f);

		if (setupDone) {
			if (octaveBandFilterGrouping)
			{
				for (int band = 0; band < octaveBandFrequencies_Hz.size(); band++)
				{
					CMonoBuffer<float> oneBandBuffer(inputBuffer.size(), 0.0f);

					// Process eq for each band, mixing eq process of all internal band filters
					int firstBandFilter, lastBandFilter;
					GetOctaveBandGammatoneFiltersFirstAndLastIndex(band, firstBandFilter, lastBandFilter);
					for (int i = firstBandFilter; i <= lastBandFilter; i++)
					{
						CMonoBuffer<float> oneFilterOutputBuffer(inputBuffer.size());
						gammatoneFilterBank.GetFilter(i)->Process(inputBuffer, oneFilterOutputBuffer);
						oneBandBuffer += oneFilterOutputBuffer;
					}

					// Process expander for each band
					perGroupBandExpanders[band]->Process(oneBandBuffer);

					// Apply attenuation for each band
					oneBandBuffer.ApplyGain(CalculateAttenuationFactor(octaveBandAttenuations[band]));

					// Mix into output buffer
					outputBuffer += oneBandBuffer;
				}
			}
			else
			{
				for (int filterIndex = 0; filterIndex < gammatoneFilterBank.GetNumFilters(); filterIndex++)
				{
					// Declaration of buffer for each filter output
					CMonoBuffer<float> oneFilterBuffer(inputBuffer.size(), 0.0f);

					// Process input buffer for each filter
					gammatoneFilterBank.GetFilter(filterIndex)->Process(inputBuffer, oneFilterBuffer);

					// Process expander for each filter
					perFilterGammatoneBandExpanders[filterIndex]->Process(oneFilterBuffer);

					// Apply attenuation for each filter
					oneFilterBuffer.ApplyGain(GetFilterGain(filterIndex));

					// Mix into output buffer
					outputBuffer += oneFilterBuffer;
				}
			}
				outputBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_GAMMATONE);
		}
	}
	

	float CGammatoneMultibandExpander::GetFilterFrequency(int filterIndex)
	{
		return gammatoneExpanderBandFrequencies_Hz[filterIndex];
	}

	int CGammatoneMultibandExpander::GetNumBands(bool filterGrouping)
	{
		if (filterGrouping) {
			return perGroupBandExpanders.size();
		}
		else
		{
			return perFilterGammatoneBandExpanders.size();
		}
	}

	Common::CDynamicExpanderMono * CGammatoneMultibandExpander::GetBandExpander(int bandIndex, bool filterGrouping)
	{
		if (filterGrouping) {
			 return perGroupBandExpanders[bandIndex];
		}
		else
		{
			return perFilterGammatoneBandExpanders[bandIndex];
		}
	}

	float CGammatoneMultibandExpander::GetOctaveBandFrequency(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0;
		}
		SET_RESULT(RESULT_OK, "");
		return octaveBandFrequencies_Hz[bandIndex];
	}

	void CGammatoneMultibandExpander::SetAttenuationForOctaveBand(int bandIndex, float attenuation)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return;
		}
		SET_RESULT(RESULT_OK, "");
		octaveBandAttenuations[bandIndex] = attenuation;
	}

	float CGammatoneMultibandExpander::GetAttenuationForOctaveBand(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0.0f;
		}
		SET_RESULT(RESULT_OK, "");
		return octaveBandAttenuations[bandIndex];
	}

	bool CGammatoneMultibandExpander::IsReady()
	{
		return setupDone;
	}

	void CGammatoneMultibandExpander::SetFilterGrouping(bool filterGrouping)
	{
		octaveBandFilterGrouping = filterGrouping;
		CleanAllBuffers();
	}

	bool CGammatoneMultibandExpander::GetFilterGrouping()
	{
		return octaveBandFilterGrouping;
	}


	float CGammatoneMultibandExpander::CalculateAttenuationFactor(float attenuation)
	{
		return std::pow(10.0f, -attenuation / 20.0f);
	}

	float CGammatoneMultibandExpander::GetFilterGain(int filterIndex)
	{
		return CalculateAttenuationFactor(GetFilterGainDB(filterIndex));
	}

	float CGammatoneMultibandExpander::GetFilterGainDB(int filterIndex)
	{
		if (setupDone) {
			if (gammatoneLowerBandFactors[filterIndex] < 0 && gammatoneHigherBandFactors[filterIndex] > 0)
			{
				return	(octaveBandAttenuations[gammatoneHigherBandIndices[filterIndex]]);
			}
			else if (gammatoneLowerBandFactors[filterIndex] > 0 && gammatoneHigherBandFactors[filterIndex] < 0)
			{
				return	(octaveBandAttenuations[gammatoneLowerBandIndices[filterIndex]]);
			}
			else if (gammatoneLowerBandFactors[filterIndex] > 0 && gammatoneHigherBandFactors[filterIndex] > 0)
			{
				return	(octaveBandAttenuations[gammatoneLowerBandIndices[filterIndex]]) * gammatoneLowerBandFactors[filterIndex] +
					(octaveBandAttenuations[gammatoneHigherBandIndices[filterIndex]]) * gammatoneHigherBandFactors[filterIndex];
			}
			else return 0.0f;	
		}
		else
			return 0.0f;
	}

	float CGammatoneMultibandExpander::GetNumFilters()
	{
		return gammatoneExpanderBandFrequencies_Hz.size();
	}

	float CGammatoneMultibandExpander::GetLowerOctaveBandFrequency(float filterFrequency, int & lowerBandIndex)
	{
		for (int i = 0; i < GetNumBands(true); i++)
		{
			if (filterFrequency < octaveBandFrequencies_Hz[i])
			{
				if (i == 0)
				{
					lowerBandIndex = -1;
					return 0.0f;
				}
				else
				{
					lowerBandIndex = i - 1;
					return octaveBandFrequencies_Hz[lowerBandIndex];
				}
			}
		}

		lowerBandIndex = GetNumBands(true) - 1;
		return octaveBandFrequencies_Hz[lowerBandIndex];

	}

	float CGammatoneMultibandExpander::GetHigherOctaveBandFrequency(float filterFrequency, int & higherBandIndex)
	{
		for (int i = 0; i < GetNumBands(true); i++)
		{
			if (filterFrequency < octaveBandFrequencies_Hz[i])
			{
				if (i == 0)
				{
					higherBandIndex = 0;
					return octaveBandFrequencies_Hz[0];
				}
				else
				{
					higherBandIndex = i;
					return octaveBandFrequencies_Hz[higherBandIndex];
				}
			}
		}

		higherBandIndex = -1;
		return 30000.0f;
	}

}// end namespace HAHLSimulation