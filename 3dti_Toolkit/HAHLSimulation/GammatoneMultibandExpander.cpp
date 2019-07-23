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
	void CGammatoneMultibandExpander::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, bool filterGrouping)
	{
		setupDone = false;
		
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
		
		// Attributes applying
		this->samplingRate = samplingRate;
		octaveBandFilterGrouping = filterGrouping;

		// Setup equalizer	
		float bandsPerOctave = 1.0f;	// Currently fixed to one band per octave, but could be a parameter
		float bandFrequencyStep = std::pow(2, 1.0f / (float)bandsPerOctave);
		float bandFrequency = iniFreq_Hz;

		for (int band = 0; band < bandsNumber; band++)
		{

			// Add band to list of frequencies and gains
			octaveBandFrequencies_Hz.push_back(bandFrequency);
			octaveBandGains_dB.push_back(0.0f);
			bandFrequency *= bandFrequencyStep;

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
			AddMonoExpander(perFilterGammatoneBandExpanders);

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

		setupDone = true;
	}


	void CGammatoneMultibandExpander::GetBandsFirstAndLastIndex(int bandIndex, int &firstInternalBand, int &lastInternalBand)
	{

		float lowerThreshold = bandIndex > 0 ? bandLimits_Hz[bandIndex - 1] : 0.0f;
		float upperThreshold = bandIndex < bandLimits_Hz.size() ? bandLimits_Hz[bandIndex] : 30000.0f;

		bool firstInternalBandFound = false;
		bool lastInternalBandFound = false;

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
				lastInternalBandFound = true;
				break;
			}
		}

		if (!lastInternalBandFound) lastInternalBand = gammatoneExpanderBandFrequencies_Hz.size() - 1;

	}

	float CGammatoneMultibandExpander::GetBandGain(int bandIndex)
	{
		return CalculateAttenuationFactor(GetBandGainDB(bandIndex));
	}

	float CGammatoneMultibandExpander::GetBandGainDB(int bandIndex)
	{
		if (setupDone) {
			if (gammatoneLowerBandGroupFactors[bandIndex] < 0)
			{
				return	(octaveBandAttenuations[gammatoneHigherBandGroupIndices[bandIndex]]);
			}
			else if (gammatoneHigherBandGroupFactors[bandIndex] < 0)
			{
				return	(octaveBandAttenuations[gammatoneLowerBandGroupIndices[bandIndex]]);
			}
			else if (gammatoneLowerBandGroupFactors[bandIndex] > 0 && gammatoneHigherBandGroupFactors[bandIndex] > 0)
			{
				return	(octaveBandAttenuations[gammatoneLowerBandGroupIndices[bandIndex]]) * gammatoneLowerBandGroupFactors[bandIndex] +
					(octaveBandAttenuations[gammatoneHigherBandGroupIndices[bandIndex]]) * gammatoneHigherBandGroupFactors[bandIndex];
			}
			else return 0.0f;
		}
		else
			return 0.0f;
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
				for (int band = 0; band < bandIndices.size(); band++)
				{
					CMonoBuffer<float> oneBandBuffer(inputBuffer.size(), 0.0f);

					// Process eq for each band, mixing eq process of all internal band filters
					for (int i = bandIndices[band][0]; i <= bandIndices[band][1]; i++)
					{
						CMonoBuffer<float> oneFilterOutputBuffer(inputBuffer.size());
						gammatoneFilterBank.GetFilter(i)->Process(inputBuffer, oneFilterOutputBuffer);
						oneBandBuffer += oneFilterOutputBuffer;
					}

					// Apply gain correction
					oneBandBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_GAMMATONE);

					// Process expander for each band
					perGroupBandExpanders[band]->Process(oneBandBuffer);

					// Apply attenuation for each band
					oneBandBuffer.ApplyGain(GetBandGain(band));

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

					// Apply gain correction
					oneFilterBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_GAMMATONE);

					// Process expander for each filter
					perFilterGammatoneBandExpanders[filterIndex]->Process(oneFilterBuffer);

					// Apply attenuation for each filter
					oneFilterBuffer.ApplyGain(GetFilterGain(filterIndex));

					// Mix into output buffer
					outputBuffer += oneFilterBuffer;
				}
			}
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

	void CGammatoneMultibandExpander::SetGroups(vector<float> bandLimits)
	{
		bandLimits_Hz.clear();
		gammatoneLowerBandGroupIndices.clear();
		gammatoneLowerBandGroupFactors.clear();
		gammatoneHigherBandGroupIndices.clear();
		gammatoneHigherBandGroupFactors.clear();
		perGroupBandExpanders.clear();
		groupBandCentralFrequencies_Hz.clear();

		if (bandLimits.size() > 0)
		{
			for (int i = 0; i < bandLimits.size(); i++)
			{
				if (i!=0) ASSERT(bandLimits[i] > bandLimits[i - 1], RESULT_ERROR_NOTALLOWED, "Band limits should be in ascending order", "");
				ASSERT(bandLimits[i] >= 0.0f, RESULT_ERROR_NOTALLOWED, "Band limits should be positive", "");
			}

			bandLimits_Hz = bandLimits;
			bandIndices.resize(bandLimits_Hz.size() + 1);

			for (int i = 0; i < bandIndices.size(); i++)
			{
				bandIndices[i].resize(2);
				GetBandsFirstAndLastIndex(i, bandIndices[i][0], bandIndices[i][1]);

				float bandCentralFreq;
				if (i == 0) bandCentralFreq = 0;
				else if (i == bandLimits_Hz.size()) bandCentralFreq = 30000;
				else if (i > 0 && i < bandLimits_Hz.size()) bandCentralFreq = sqrtf(bandLimits_Hz[i] * bandLimits_Hz[i - 1]);

				groupBandCentralFrequencies_Hz.push_back(bandCentralFreq);

				// Add lower and higher octave band indices for this frequency
				int lowerBandIndex, higherBandIndex;
				float lowerBandFrequency = GetLowerOctaveBandFrequency(bandCentralFreq, lowerBandIndex);
				gammatoneLowerBandGroupIndices.push_back(lowerBandIndex);
				float higherBandFrequency = GetHigherOctaveBandFrequency(bandCentralFreq, higherBandIndex);
				gammatoneHigherBandGroupIndices.push_back(higherBandIndex);

				// Add lower and higher octave band factors for this frequency
				float frequencyDistance = higherBandFrequency - lowerBandFrequency;
				gammatoneLowerBandGroupFactors.push_back(lowerBandFrequency <= 20.0f ? -1.0f : ((higherBandFrequency - bandCentralFreq) / frequencyDistance));
				gammatoneHigherBandGroupFactors.push_back(higherBandFrequency >= 20000.0f ? -1.0f : ((bandCentralFreq - lowerBandFrequency) / frequencyDistance));

				// Setup per-group expanders
				AddMonoExpander(perGroupBandExpanders);

			}

		}
		else if (bandLimits.size() == 0)
		{
			bandIndices.resize(1);
			bandIndices[0].resize(2);
			bandIndices[0][0] = 0;
			bandIndices[0][1] = gammatoneFilterBank.GetNumFilters() - 1;
			AddMonoExpander(perGroupBandExpanders);
		}

	}

	void CGammatoneMultibandExpander::AddMonoExpander(vector<Common::CDynamicExpanderMono*>& monoExpanderVector)
	{
		Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
		expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
		monoExpanderVector.push_back(expander);
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

	float CGammatoneMultibandExpander::GetBandFrequency(int bandIndex, bool filterGrouping)
	{
		if (filterGrouping)
			return groupBandCentralFrequencies_Hz[bandIndex];
		else
			return gammatoneExpanderBandFrequencies_Hz[bandIndex];
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
		for (int i = 0; i < octaveBandFrequencies_Hz.size(); i++)
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

		lowerBandIndex = octaveBandFrequencies_Hz.size() - 1;
		return octaveBandFrequencies_Hz[lowerBandIndex];

	}

	float CGammatoneMultibandExpander::GetHigherOctaveBandFrequency(float filterFrequency, int & higherBandIndex)
	{
		for (int i = 0; i < octaveBandFrequencies_Hz.size(); i++)
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