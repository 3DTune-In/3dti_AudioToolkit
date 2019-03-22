/*
* \class CMultibandExpander
*
* \brief Implementation of CMultibandExpander class. This class implements a multiband equalizer where each band has an
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
#include <HAHLSimulation/MultibandExpander.h>
#include <Common/ErrorHandler.h>
#include <iostream>
#include <iomanip>

namespace HAHLSimulation {
	void CMultibandExpander::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, int filtersPerBand, TFilterBank _filterBank)
	{
		setupDone = false;
		switch (_filterBank)
		{
		case TFilterBank::GAMMATONE:
			SetupGammatone(samplingRate, iniFreq_Hz, bandsNumber);
			break;
		case TFilterBank::BUTTERWORTH:
			SetupButterworth(samplingRate, iniFreq_Hz, bandsNumber, filtersPerBand);
			break;
		default:
			SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Filterbank options are Gammatone and Butterworth");
		}
		setupDone = true;
	}

	void CMultibandExpander::SetupButterworth(int samplingRate, float iniFreq_Hz, int bandsNumber, int filtersPerBand)
	{
		if ((filtersPerBand % 2) == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Filters per band for multiband expander must be an odd number.");
			return;
		}

		octaveBandFrequencies_Hz.clear();
		bandExpanders.clear();
		bandGains_dB.clear();
		bandAttenuations.clear();

		// Setup equalizer	
		float bandsPerOctave = 1.0f;	// Currently fixed to one band per octave, but could be a parameter
		float bandFrequencyStep = std::pow(2, 1.0f / (float)bandsPerOctave);
		float filterFrequencyStep = std::pow(2, 1.0f / (bandsPerOctave*filtersPerBand));
		float bandFrequency = iniFreq_Hz;
		float filterFrequency = bandFrequency / ((float)(trunc(filtersPerBand / 2))*filterFrequencyStep);

		// Compute Q for all filters
		float octaveStep = 1.0f / ((float)filtersPerBand * bandsPerOctave);
		float octaveStepPow = std::pow(2.0f, octaveStep);
		float Q_BPF = std::sqrt(octaveStepPow) / (octaveStepPow - 1.0f);

		// Create and setup all bands
		butterworthFilterBank.RemoveFilters();
		for (int band = 0; band < bandsNumber; band++)
		{
			// Create filters
			for (int filter = 0; filter < filtersPerBand; filter++)
			{
				butterworthFilterBank.AddFilter()->Setup(samplingRate, filterFrequency, Q_BPF, Common::T_filterType::BANDPASS);
				filterFrequency *= filterFrequencyStep;
			}

			// Add band to list of frequencies and gains
			octaveBandFrequencies_Hz.push_back(bandFrequency);
			bandGains_dB.push_back(0.0f);
			bandFrequency *= bandFrequencyStep;

			// Setup expanders
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			bandExpanders.push_back(expander);

			// Create atenuation for band
			bandAttenuations.push_back(0.0f);
		}

		filterBankUsed = BUTTERWORTH;
	}


	void CMultibandExpander::SetupGammatone(int samplingRate, float iniFreq_Hz, int bandsNumber)
	{
		octaveBandFrequencies_Hz.clear();
		bandExpanders.clear();
		bandGains_dB.clear();
		bandAttenuations.clear();
		lowerBandFactors.clear();
		higherBandFactors.clear();
		expanderBandFrequencies_Hz.clear();
		lowerBandIndices.clear();
		higherBandIndices.clear();

		// Setup equalizer	
		float bandsPerOctave = 1.0f;	// Currently fixed to one band per octave, but could be a parameter
		float bandFrequencyStep = std::pow(2, 1.0f / (float)bandsPerOctave);
		float bandFrequency = iniFreq_Hz;
		
		for (int band = 0; band < bandsNumber; band++)
		{
			// Add band to list of frequencies and gains
			octaveBandFrequencies_Hz.push_back(bandFrequency);
			bandGains_dB.push_back(0.0f);
			bandFrequency *= bandFrequencyStep;

			// Create atenuation for band
			bandAttenuations.push_back(0.0f);
		}

		gammatoneFilterBank.RemoveFilters();
		gammatoneFilterBank.SetSamplingFreq(samplingRate);
		gammatoneFilterBank.InitWithFreqRangeOverlap(20, 20000, 0.0, Common::CGammatoneFilterBank::EAR_MODEL_DEFAULT);
		
		// Setup expanders
		for (int filterIndex = 0; filterIndex < gammatoneFilterBank.GetNumFilters(); filterIndex++)
		{
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			bandExpanders.push_back(expander);

			float filterFrequency = gammatoneFilterBank.GetFilter(filterIndex)->GetCenterFrequency();
			expanderBandFrequencies_Hz.push_back(filterFrequency);

			int lowerBandIndex, higherBandIndex;
			float lowerBandFrequency = GetLowerOctaveBandFrequency(filterFrequency, lowerBandIndex);
			lowerBandIndices.push_back(lowerBandIndex);
			float higherBandFrequency = GetHigherOctaveBandFrequency(filterFrequency, higherBandIndex);
			higherBandIndices.push_back(higherBandIndex);
			float frequencyDistance = higherBandFrequency - lowerBandFrequency;
			float lowerBandFactor, higherBandFactor;
			if (lowerBandFrequency <= 20.0f)
			{
				lowerBandFactor = -1.0f;
			}
			else
			{
				lowerBandFactor = (higherBandFrequency - filterFrequency) / frequencyDistance;
			}

			if (higherBandFrequency >= 20000.0f)
			{
				higherBandFactor = -1.0f;
			}
			else
			{
				higherBandFactor = (filterFrequency - lowerBandFrequency) / frequencyDistance;
			}

			lowerBandFactors.push_back(lowerBandFactor);
			higherBandFactors.push_back(higherBandFactor);

		}

		filterBankUsed = GAMMATONE;
	}

	//////////////////////////////////////////////
	// Get configured number of filters per band, to increase bandwidth
	int CMultibandExpander::GetNumberOfButterworthFiltersPerBand()
	{
		return (butterworthFilterBank.GetNumFilters() / octaveBandFrequencies_Hz.size());
	}


	//////////////////////////////////////////////
	// Get the first and last index in the filter bank for the internal bands corresponding to a given band index
	void CMultibandExpander::GetOctaveBandButterworthFiltersFirstAndLastIndex(int bandIndex, int &firstInternalBand, int &lastInternalBand)
	{
		int centralInternalBand = bandIndex * GetNumberOfButterworthFiltersPerBand() + trunc(GetNumberOfButterworthFiltersPerBand() / 2);
		firstInternalBand = centralInternalBand - trunc(GetNumberOfButterworthFiltersPerBand() / 2);
		lastInternalBand = centralInternalBand + trunc(GetNumberOfButterworthFiltersPerBand() / 2);
	}


	void CMultibandExpander::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		// Initialization of output buffer
		outputBuffer.Fill(outputBuffer.size(), 0.0f);

		if (setupDone) {
			if (filterBankUsed == TFilterBank::GAMMATONE) {

				for (int filterIndex = 0; filterIndex < gammatoneFilterBank.GetNumFilters(); filterIndex++)
				{
					// Declaration of buffer for each filter output
					CMonoBuffer<float> oneFilterBuffer(inputBuffer.size(), 0.0f);

					// Process input buffer for each filter
					gammatoneFilterBank.GetFilter(filterIndex)->Process(inputBuffer, oneFilterBuffer);

					// Process expander for each filter
					bandExpanders[filterIndex]->Process(oneFilterBuffer);

					// Apply attenuation for each filter
					oneFilterBuffer.ApplyGain(GetFilterGain(filterIndex));

					// Mix into output buffer
					outputBuffer += oneFilterBuffer;
				}

				outputBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_GAMMATONE);

			}
			else
			{
				for (int band = 0; band < octaveBandFrequencies_Hz.size(); band++)
				{
					CMonoBuffer<float> oneBandBuffer(inputBuffer.size(), 0.0f);

					// Process eq for each band, mixing eq process of all internal band filters
					int firstBandFilter, lastBandFilter;
					GetOctaveBandButterworthFiltersFirstAndLastIndex(band, firstBandFilter, lastBandFilter);
					for (int i = firstBandFilter; i <= lastBandFilter; i++)
					{
						CMonoBuffer<float> oneFilterOutputBuffer(inputBuffer.size());
						butterworthFilterBank.GetFilter(i)->Process(inputBuffer, oneFilterOutputBuffer);
						oneBandBuffer += oneFilterOutputBuffer;
					}

					// Process expander for each band
					bandExpanders[band]->Process(oneBandBuffer);

					// Apply attenuation for each band
					oneBandBuffer.ApplyGain(CalculateAttenuationFactor(bandAttenuations[band]));

					// Mix into output buffer
					outputBuffer += oneBandBuffer;
				}

				outputBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_BUTTERWORTH);

			}
		}
	}

	float CMultibandExpander::GetOctaveBandFrequency(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0;
		}
		SET_RESULT(RESULT_OK, "");
		return octaveBandFrequencies_Hz[bandIndex];
	}

	float CMultibandExpander::GetExpanderBandFrequency(int bandIndex)
	{
		return expanderBandFrequencies_Hz[bandIndex];
	}

	void CMultibandExpander::SetAttenuationForOctaveBand(int bandIndex, float attenuation)
	{
		if (bandIndex < 0 || bandIndex >= bandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return;
		}
		SET_RESULT(RESULT_OK, "");
		bandAttenuations[bandIndex] = attenuation;
	}

	//////////////////////////////////////////////
	float CMultibandExpander::GetAttenuationForOctaveBand(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= bandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0.0f;
		}
		SET_RESULT(RESULT_OK, "");
		return bandAttenuations[bandIndex];
	}

	bool CMultibandExpander::IsReady()
	{
		return setupDone;
	}

	TFilterBank CMultibandExpander::GetFilterBankType()
	{
		return filterBankUsed;
	}


	float CMultibandExpander::CalculateAttenuationFactor(float attenuation)
	{
		return std::pow(10.0f, -attenuation / 20.0f);
	}

	float CMultibandExpander::GetFilterGain(int filterIndex) // en veces directamente
	{
		return CalculateAttenuationFactor(GetFilterGainDB(filterIndex));
		
		
	}



	float CMultibandExpander::GetFilterGainDB(int filterIndex)
	{
		if (setupDone) {
			if (lowerBandFactors[filterIndex] < 0 && higherBandFactors[filterIndex] > 0)
			{
				return	(bandAttenuations[higherBandIndices[filterIndex]]);
			}
			else if (lowerBandFactors[filterIndex] > 0 && higherBandFactors[filterIndex] < 0)
			{
				return	(bandAttenuations[lowerBandIndices[filterIndex]]);
			}
			else if (lowerBandFactors[filterIndex] > 0 && higherBandFactors[filterIndex] > 0)
			{
				return	(bandAttenuations[lowerBandIndices[filterIndex]]) * lowerBandFactors[filterIndex] +
					(bandAttenuations[higherBandIndices[filterIndex]]) * higherBandFactors[filterIndex];
			}
			else
			{
				return 0.0f;
			}
		}
		else
			return 0.0f;
	}

	float CMultibandExpander::GetNumFilters()
	{
		return expanderBandFrequencies_Hz.size();
	}

	float CMultibandExpander::GetLowerOctaveBandFrequency(float filterFrequency, int & lowerBandIndex)
	{
		for (int i = 0; i < GetNumBands(); i++)
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

		lowerBandIndex = GetNumBands() - 1;
		return octaveBandFrequencies_Hz[lowerBandIndex];

	}

	float CMultibandExpander::GetHigherOctaveBandFrequency(float filterFrequency, int & higherBandIndex)
	{
		for (int i = 0; i < GetNumBands(); i++)
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