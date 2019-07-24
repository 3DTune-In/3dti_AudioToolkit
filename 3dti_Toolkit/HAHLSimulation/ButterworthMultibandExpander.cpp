/*
* \class CButterworthMultibandExpander
*
* \brief Implementation of CButterworthMultibandExpander class. This class implements a multiband equalizer where each band has an
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
#include <HAHLSimulation/ButterworthMultibandExpander.h>
#include <Common/ErrorHandler.h>
#include <iostream>
#include <iomanip>

namespace HAHLSimulation {

	void CButterworthMultibandExpander::SetNumberOfFiltersPerBand(int filtersPerBand)
	{
		filterbankSetupDone = false;

		butterworthFilterBank.RemoveFilters();

		if ((filtersPerBand % 2) == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Filters per band for multiband expander must be an odd number.");
			return;
		}

		// Setup equalizer	
		float bandsPerOctave = 1.0f;	// Currently fixed to one band per octave, but could be a parameter
		float bandFrequency = octaveBandFrequencies_Hz[0];
		float filterFrequencyStep = std::pow(2, 1.0f / (bandsPerOctave*filtersPerBand));
		float filterFrequency = bandFrequency / ((float)(trunc(filtersPerBand / 2))*filterFrequencyStep);

		// Compute Q for all filters
		float octaveStep = 1.0f / ((float)filtersPerBand * bandsPerOctave);
		float octaveStepPow = std::pow(2.0f, octaveStep);
		float Q_BPF = std::sqrt(octaveStepPow) / (octaveStepPow - 1.0f);

		for (int band = 0; band < octaveBandFrequencies_Hz.size(); band++)
		{
			// Create filters
			for (int filter = 0; filter < filtersPerBand; filter++)
			{
				butterworthExpanderBandFrequencies_Hz.push_back(filterFrequency);
				butterworthFilterBank.AddFilter()->Setup(samplingRate, filterFrequency, Q_BPF, Common::T_filterType::BANDPASS);
				filterFrequency *= filterFrequencyStep;

				// Setup per-filter Butterworth expanders
				Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
				expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
				perFilterButterworthBandExpanders.push_back(expander);
			}
		}

		filterbankSetupDone = true;
	}

	void CButterworthMultibandExpander::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, bool filterGrouping)
	{
		initialSetupDone = false;
		

		// Internal attributes cleaning
		octaveBandFrequencies_Hz.clear();
		butterworthExpanderBandFrequencies_Hz.clear();
		octaveBandGains_dB.clear();
		perGroupBandExpanders.clear();
		perFilterButterworthBandExpanders.clear();

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

			// Setup per-group expanders
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			perGroupBandExpanders.push_back(expander);

			// Create atenuation for band
			octaveBandAttenuations.push_back(0.0f);
		}
		this->samplingRate = samplingRate;
		octaveBandFilterGrouping = filterGrouping;
		initialSetupDone = true;
	}

	//////////////////////////////////////////////
	// Get configured number of filters per band, to increase bandwidth
	int CButterworthMultibandExpander::GetNumberOfFiltersPerBand()
	{
		return (butterworthFilterBank.GetNumFilters() / octaveBandFrequencies_Hz.size());
	}


	//////////////////////////////////////////////
	// Get the first and last index in the filter bank for the internal bands corresponding to a given band index
	void CButterworthMultibandExpander::GetOctaveBandButterworthFiltersFirstAndLastIndex(int bandIndex, int &firstInternalBand, int &lastInternalBand)
	{
		int centralInternalBand = bandIndex * GetNumberOfFiltersPerBand() + trunc(GetNumberOfFiltersPerBand() / 2);
		firstInternalBand = centralInternalBand - trunc(GetNumberOfFiltersPerBand() / 2);
		lastInternalBand = centralInternalBand + trunc(GetNumberOfFiltersPerBand() / 2);
	}

	void CButterworthMultibandExpander::CleanAllBuffers()
	{
		for (int i = 0; i < butterworthFilterBank.GetNumFilters(); i++)
		{
			CMonoBuffer<float> emptyBuffer = CMonoBuffer<float>(128, 0.0f);
			butterworthFilterBank.GetFilter(i)->Process(emptyBuffer);
		}
	}


	void CButterworthMultibandExpander::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		// Initialization of output buffer
		outputBuffer.Fill(outputBuffer.size(), 0.0f);

		if (IsReady()) {
			if (octaveBandFilterGrouping)
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

					// Apply gain correction
					oneBandBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_BUTTERWORTH);

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
				for (int filter = 0; filter < butterworthFilterBank.GetNumFilters(); filter++)
				{
					CMonoBuffer<float> oneFilterBuffer(inputBuffer.size(), 0.0f);
					butterworthFilterBank.GetFilter(filter)->Process(inputBuffer, oneFilterBuffer);
					oneFilterBuffer.ApplyGain(LINEAR_GAIN_CORRECTION_BUTTERWORTH);
					perFilterButterworthBandExpanders[filter]->Process(oneFilterBuffer);
					oneFilterBuffer.ApplyGain(GetFilterGain(filter)); 
					outputBuffer += oneFilterBuffer;
				}
			}
		}
	}


	float CButterworthMultibandExpander::GetFilterFrequency(int filterIndex)
	{
		return butterworthExpanderBandFrequencies_Hz[filterIndex];
	}

	int CButterworthMultibandExpander::GetNumBands(bool filterGrouping)
	{
		if (filterGrouping) {
			return perGroupBandExpanders.size();
		}
		else
		{
			return perFilterButterworthBandExpanders.size();
		}
	}

	Common::CDynamicExpanderMono * CButterworthMultibandExpander::GetBandExpander(int bandIndex, bool filterGrouping)
	{
		if (filterGrouping) {
			 return perGroupBandExpanders[bandIndex];
		}
		else
		{
			return perFilterButterworthBandExpanders[bandIndex];
		}
	}

	float CButterworthMultibandExpander::GetOctaveBandFrequency(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0;
		}
		SET_RESULT(RESULT_OK, "");
		return octaveBandFrequencies_Hz[bandIndex];
	}

	float CButterworthMultibandExpander::GetBandFrequency(int bandIndex, bool filterGrouping)
	{
		if (filterGrouping)
			return octaveBandFrequencies_Hz[bandIndex];
		else
			return GetFilterFrequency(bandIndex);
	}

	void CButterworthMultibandExpander::SetAttenuationForOctaveBand(int bandIndex, float attenuation)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return;
		}
		SET_RESULT(RESULT_OK, "");
		octaveBandAttenuations[bandIndex] = attenuation;
	}

	float CButterworthMultibandExpander::GetAttenuationForOctaveBand(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= octaveBandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0.0f;
		}
		SET_RESULT(RESULT_OK, "");
		return octaveBandAttenuations[bandIndex];
	}


	bool CButterworthMultibandExpander::IsReady()
	{
		return filterbankSetupDone && initialSetupDone;
	}

	void CButterworthMultibandExpander::SetFilterGrouping(bool filterGrouping)
	{
		octaveBandFilterGrouping = filterGrouping;
		CleanAllBuffers();
	}

	bool CButterworthMultibandExpander::GetFilterGrouping()
	{
		return octaveBandFilterGrouping;
	}

	float CButterworthMultibandExpander::CalculateAttenuationFactor(float attenuation)
	{
		return std::pow(10.0f, -attenuation / 20.0f);
	}

	float CButterworthMultibandExpander::GetFilterGain(int filterIndex)
	{
		return CalculateAttenuationFactor(GetFilterGainDB(filterIndex));
	}

	float CButterworthMultibandExpander::GetFilterGainDB(int filterIndex)
	{
		if (IsReady()) {
			int filtersPerBand = GetNumberOfFiltersPerBand();
			int bandNumber = filterIndex / filtersPerBand;
			int filterIndexInsideBand = filterIndex % filtersPerBand;
			float threshold = ((float)filtersPerBand - 1.0f) / 2.0f;

			if (bandNumber == 0)
			{
				if (filterIndexInsideBand <= threshold)
				{
					return octaveBandAttenuations[bandNumber];
				}
				else
				{
					return ((filtersPerBand - filterIndexInsideBand) / filtersPerBand) * octaveBandAttenuations[bandNumber + 1]
						+ ((float)filterIndexInsideBand / (float)filtersPerBand) * octaveBandAttenuations[bandNumber];
				}
			}
			else if (bandNumber > 0 && bandNumber < GetNumBands(true) - 1)
			{
				if (filterIndexInsideBand < threshold)
				{
					return ((filtersPerBand - filterIndexInsideBand) / filtersPerBand) * octaveBandAttenuations[bandNumber]
						+ ((float)filterIndexInsideBand / (float)filtersPerBand) * octaveBandAttenuations[bandNumber - 1];
				}
				else if(abs(filterIndexInsideBand - threshold) < 0.01f)
				{
					return octaveBandAttenuations[bandNumber];
				}
				else 
				{
					return ((filtersPerBand - filterIndexInsideBand) / filtersPerBand) * octaveBandAttenuations[bandNumber + 1]
						+ ((float)filterIndexInsideBand / (float)filtersPerBand) * octaveBandAttenuations[bandNumber];
				}
			}
			else if (bandNumber == GetNumBands(true) - 1)
			{
				if (filterIndexInsideBand >= threshold)
				{
					return octaveBandAttenuations[bandNumber];
				}
				else
				{
					return ((filtersPerBand - filterIndexInsideBand) / filtersPerBand) * octaveBandAttenuations[bandNumber]
						+ ((float)filterIndexInsideBand / (float)filtersPerBand) * octaveBandAttenuations[bandNumber - 1];
				}
			}

			else return 0.0f;
		}
		else
			return 0.0f;
	}

	float CButterworthMultibandExpander::GetNumFilters()
	{
		return butterworthExpanderBandFrequencies_Hz.size();
	}

	float CButterworthMultibandExpander::GetLowerOctaveBandFrequency(float filterFrequency, int & lowerBandIndex)
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

	float CButterworthMultibandExpander::GetHigherOctaveBandFrequency(float filterFrequency, int & higherBandIndex)
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