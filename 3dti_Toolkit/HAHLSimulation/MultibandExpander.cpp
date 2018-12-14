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

namespace HAHLSimulation {

	//////////////////////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR

	//CMultibandExpander::CMultibandExpander()
	//{
	//}

	////////////////////////////////////////////////
	void CMultibandExpander::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber)
	{
		bandFrequencies_Hz.clear();
		bandExpanders.clear();
		bandGains_dB.clear();
		bandAttenuations.clear();

		// Setup equalizer	
		float bandsPerOctave = 1.0f;	// Currently fixed to one band per octave, but could be a parameter
		float bandFrequencyStep = std::pow(2, 1.0f / (float)bandsPerOctave);
		float bandFrequency = iniFreq_Hz;
		
		for (int band = 0; band < bandsNumber; band++)
		{
			// Add band to list of frequencies and gains
			bandFrequencies_Hz.push_back(bandFrequency);
			bandGains_dB.push_back(0.0f);
			bandFrequency *= bandFrequencyStep;

			// Create atenuation for band
			bandAttenuations.push_back(0.0f);
		}

		filterBank.RemoveFilters();
		filterBank.SetSamplingFreq(samplingRate);
		filterBank.InitWithFreqRangeOverlap(20, 20000, 0.0, Common::CGammatoneFilterBank::EAR_MODEL_DEFAULT);

		// Setup expanders
		for (int filterIndex = 0; filterIndex < filterBank.GetNumFilters(); filterIndex++)
		{
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			bandExpanders.push_back(expander);

			float filterFrequency = filterBank.GetFilter(filterIndex)->GetCenterFrequency();
			int lowerBandIndex, higherBandIndex;
			float lowerBandFrequency = GetLowerBandFrequency(filterFrequency, lowerBandIndex);
			lowerBandIndices.push_back(lowerBandIndex);
			float higherBandFrequency = GetHigherBandFrequency(filterFrequency, higherBandIndex);
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

	}

	//////////////////////////////////////////////
	// Get the first and last index in the filter bank for the internal bands corresponding to a given band index
	void CMultibandExpander::GetBandFiltersFirstAndLastIndex(int bandIndex, int &firstInternalBand, int &lastInternalBand)
	{
		int centralInternalBand = bandIndex * GetNumberOfFiltersPerBand() + trunc(GetNumberOfFiltersPerBand() / 2);
		firstInternalBand = centralInternalBand - trunc(GetNumberOfFiltersPerBand() / 2);
		lastInternalBand = centralInternalBand + trunc(GetNumberOfFiltersPerBand() / 2);
	}

	//////////////////////////////////////////////
	// Get configured number of filters per band, to increase bandwidth
	int CMultibandExpander::GetNumberOfFiltersPerBand()
	{
		return (filterBank.GetNumFilters() / bandFrequencies_Hz.size());
	}

	//////////////////////////////////////////////
	void CMultibandExpander::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		// Initialization of output buffer
		outputBuffer.Fill(outputBuffer.size(), 0.0f);
		////////////////////////////////////////////////////////////////// vvv---- esto es porque mike hace un filtro más del que reporta!!!!!
		for (int filterIndex = 0; filterIndex < filterBank.GetNumFilters(); filterIndex++)
		{
			// Declaration of buffer for each filter output
			CMonoBuffer<float> oneFilterBuffer(inputBuffer.size(), 0.0f);

			// Process input buffer for each filter
			filterBank.GetFilter(filterIndex)->Process(inputBuffer, oneFilterBuffer);

			// Process expander for each filter
			bandExpanders[filterIndex]->Process(oneFilterBuffer);

			// Apply attenuation for each filter
			oneFilterBuffer.ApplyGain(GetFilterGain(filterIndex));

			// Mix into output buffer
			outputBuffer += oneFilterBuffer;
		}

	}

	//////////////////////////////////////////////
	// Specifies the gain for each band in dB
	//void CMultibandExpander::SetGains_dB(vector<float> gains_dB)
	//{
	//	if (gains_dB.size() != bandFrequencies_Hz.size())
	//	{
	//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "number of elements must agree ( gains_dB Vs number of bands in the equalizer)");
	//		return;
	//	}
	//
	//	SET_RESULT(RESULT_OK, "");
	//
	//	for (int c = 0; c < gains_dB.size(); c++)
	//		SetBandGain_dB(c, gains_dB[c]);
	//}

	//--------------------------------------------------------------
	// Specifies the gain for each band in dB
	//void CMultibandExpander::ResetGains_dB()
	//{
	//	for (int c = 0; c < bandGains_dB.size(); c++)
	//		SetBandGain_dB(c, 0.0f);
	//}

	//--------------------------------------------------------------
	//void CMultibandExpander::SetBandGain_dB(int bandIndex, float gain_dB)
	//{
	//	if (bandIndex < 0 || bandIndex >= bandFrequencies_Hz.size())
	//	{
	//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
	//		return;
	//	}
	//
	//	SET_RESULT(RESULT_OK, "");
	//	bandGains_dB[bandIndex] = gain_dB;
	//
	//	// Set gain for all internal bands	
	//	float gain = pow(10.0, gain_dB / 20.0);
	//	int firstInternalBand, lastInternalBand;
	//	GetBandFiltersFirstAndLastIndex(bandIndex, firstInternalBand, lastInternalBand);
	//	for (int i=firstInternalBand; i < lastInternalBand; i++)
	//	{
	//		filterBank.GetFilter(i)->generalGain = gain;
	//	}
	//}

	//--------------------------------------------------------------
	float CMultibandExpander::GetBandFrequency(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= bandFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0;
		}
		SET_RESULT(RESULT_OK, "");
		return bandFrequencies_Hz[bandIndex];
	}

	//////////////////////////////////////////////
	//float CMultibandExpander::GetBandGain_dB(int bandIndex)
	//{
	//	if (bandIndex < 0 || bandIndex >= bandFrequencies_Hz.size())
	//	{
	//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
	//		return 0.0f;
	//	}
	//	SET_RESULT(RESULT_OK, "");
	//	return bandGains_dB[bandIndex];
	//}

	//////////////////////////////////////////////
	void CMultibandExpander::SetAttenuationForBand(int bandIndex, float attenuation)
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
	float CMultibandExpander::GetAttenuationForBand(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= bandAttenuations.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0.0f;
		}
		SET_RESULT(RESULT_OK, "");
		return bandAttenuations[bandIndex];
	}

	//////////////////////////////////////////////
	float CMultibandExpander::CalculateAttenuationFactor(float attenuation)
	{
		return std::pow(10.0f, -attenuation / 20.0f);
	}

	float CMultibandExpander::GetFilterGain(float filterIndex) // en veces directamente
	{
		if		(lowerBandFactors[filterIndex] < 0 && higherBandFactors[filterIndex] > 0)
		{
			return	CalculateAttenuationFactor(bandAttenuations[higherBandIndices[filterIndex]]);
		}
		else if (lowerBandFactors[filterIndex] > 0 && higherBandFactors[filterIndex] < 0)
		{
			return	CalculateAttenuationFactor(bandAttenuations[lowerBandIndices [filterIndex]]);
		}
		else if (lowerBandFactors[filterIndex] > 0 && higherBandFactors[filterIndex] > 0)
		{
			return	CalculateAttenuationFactor(bandAttenuations[lowerBandIndices [filterIndex]]) * lowerBandFactors [filterIndex] +
					CalculateAttenuationFactor(bandAttenuations[higherBandIndices[filterIndex]]) * higherBandFactors[filterIndex];
		}
		else
		{
			return 0.0f;
		}

		
	}

	float CMultibandExpander::GetLowerBandFrequency(float filterFrequency, int & lowerBandIndex)
	{
		for (int i = 0; i < GetNumBands(); i++)
		{
			if (filterFrequency < bandFrequencies_Hz[i])
			{
				if (i == 0)
				{
					lowerBandIndex = -1;
					return 0.0f;
				}
				else
				{
					lowerBandIndex = i - 1;
					return bandFrequencies_Hz[lowerBandIndex];
				}
			}
		}

		lowerBandIndex = GetNumBands() - 1;
		return bandFrequencies_Hz[lowerBandIndex];

	}

	float CMultibandExpander::GetHigherBandFrequency(float filterFrequency, int & higherBandIndex)
	{
		for (int i = 0; i < GetNumBands(); i++)
		{
			if (filterFrequency < bandFrequencies_Hz[i])
			{
				if (i == 0)
				{
					higherBandIndex = 0;
					return bandFrequencies_Hz[0];
				}
				else
				{
					higherBandIndex = i;
					return bandFrequencies_Hz[higherBandIndex];
				}
			}
		}

		higherBandIndex = -1;
		return 30000.0f;
	}

}// end namespace HAHLSimulation