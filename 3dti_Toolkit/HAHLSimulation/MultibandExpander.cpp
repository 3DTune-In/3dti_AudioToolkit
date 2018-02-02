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
	void CMultibandExpander::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, int filtersPerBand)
	{
		if ((filtersPerBand % 2) == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Filters per band for multiband expander must be an odd number.");
			return;
		}

		bandsFrequencies_Hz.clear();
		bandExpanders.clear();
		bandsGains_dB.clear();
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
		filterBank.RemoveFilters();
		for (int band = 0; band < bandsNumber; band++)
		{
			// Create filters
			for (int filter = 0; filter < filtersPerBand; filter++)
			{
				filterBank.AddFilter()->Setup(samplingRate, filterFrequency, Q_BPF, Common::T_filterType::BANDPASS);
				filterFrequency *= filterFrequencyStep;
			}

			// Add band to list of frequencies and gains
			bandsFrequencies_Hz.push_back(bandFrequency);
			bandsGains_dB.push_back(0.0f);
			bandFrequency *= bandFrequencyStep;

			// Setup expanders
			Common::CDynamicExpanderMono* expander = new Common::CDynamicExpanderMono();
			expander->Setup(samplingRate, DEFAULT_RATIO, DEFAULT_THRESHOLD, DEFAULT_ATTACK, DEFAULT_RELEASE);
			bandExpanders.push_back(expander);

			// Create atenuation for band
			bandAttenuations.push_back(0.0f);
		}

		//for (int c = 0; c < bandsNumber; c++)
		//{		
		//	// Create internal bands 
		//	for (int internalBand = 1; internalBand <= trunc(FILTERS_PER_MULTIBAND_EXPANDER_BAND/2); internalBand++)
		//	{
		//		//float internalFrequency = f - freqStepInternal*internalBand*iniFreq_Hz;
		//		float internalFrequency = f - internalBandSeparation*internalBand;
		//		filterBank.AddFilter()->Setup(samplingRate, internalFrequency, Q_BPF, Common::T_filterType::BANDPASS);
		//	}		
		//	filterBank.AddFilter()->Setup(samplingRate, f, Q_BPF, Common::T_filterType::BANDPASS);
		//	for (int internalBand = 1; internalBand <= trunc(FILTERS_PER_MULTIBAND_EXPANDER_BAND/2); internalBand++)
		//	{
		//		//float internalFrequency = f + freqStepInternal*internalBand*iniFreq_Hz;			
		//		float internalFrequency = f + internalBandSeparation*internalBand;
		//		filterBank.AddFilter()->Setup(samplingRate, internalFrequency, Q_BPF, Common::T_filterType::BANDPASS);
		//	}		

		//	// Add band to list of frequencies and gains
		//	bandsFrequencies_Hz.push_back(f);
		//	bandsGains_dB.push_back(0.0f);
		//	f *= freqStep;
		// // Expanders...
		//}
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
		return (filterBank.GetNumFilters() / bandsFrequencies_Hz.size());
	}

	//////////////////////////////////////////////
	void CMultibandExpander::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{
		outputBuffer.Fill(outputBuffer.size(), 0.0f);

		// Mix internal band outputs into groups, process expanders of each group, apply attenuations, and mix into output buffer
		for (int band = 0; band < bandsFrequencies_Hz.size(); band++)
		{
			CMonoBuffer<float> oneBandBuffer(inputBuffer.size(), 0.0f);

			// Process eq for each band, mixing eq process of all internal band filters
			int firstBandFilter, lastBandFilter;
			GetBandFiltersFirstAndLastIndex(band, firstBandFilter, lastBandFilter);
			for (int i = firstBandFilter; i <= lastBandFilter; i++)
			{
				CMonoBuffer<float> oneFilterOutputBuffer(inputBuffer.size());
				filterBank.GetFilter(i)->Process(inputBuffer, oneFilterOutputBuffer);
				oneBandBuffer += oneFilterOutputBuffer;
			}

			// Process expander for each band
			bandExpanders[band]->Process(oneBandBuffer);

			// Apply attenuation for each band
			oneBandBuffer.ApplyGain(CalculateAttenuationFactor(bandAttenuations[band]));

			// Mix into output buffer
			outputBuffer += oneBandBuffer;
		}
	}

	//////////////////////////////////////////////
	// Specifies the gain for each band in dB
	//void CMultibandExpander::SetGains_dB(vector<float> gains_dB)
	//{
	//	if (gains_dB.size() != bandsFrequencies_Hz.size())
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
	//	for (int c = 0; c < bandsGains_dB.size(); c++)
	//		SetBandGain_dB(c, 0.0f);
	//}

	//--------------------------------------------------------------
	//void CMultibandExpander::SetBandGain_dB(int bandIndex, float gain_dB)
	//{
	//	if (bandIndex < 0 || bandIndex >= bandsFrequencies_Hz.size())
	//	{
	//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
	//		return;
	//	}
	//
	//	SET_RESULT(RESULT_OK, "");
	//	bandsGains_dB[bandIndex] = gain_dB;
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
		if (bandIndex < 0 || bandIndex >= bandsFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
			return 0;
		}
		SET_RESULT(RESULT_OK, "");
		return bandsFrequencies_Hz[bandIndex];
	}

	//////////////////////////////////////////////
	//float CMultibandExpander::GetBandGain_dB(int bandIndex)
	//{
	//	if (bandIndex < 0 || bandIndex >= bandsFrequencies_Hz.size())
	//	{
	//		SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad band index");
	//		return 0.0f;
	//	}
	//	SET_RESULT(RESULT_OK, "");
	//	return bandsGains_dB[bandIndex];
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
}// end namespace HAHLSimulation