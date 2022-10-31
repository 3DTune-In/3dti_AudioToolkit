/*
* \class CGraphicEqualizer
*
* \brief Declaration of CGraphicEqualizer class 
*
* Class to handle a set of parallel digital filters whose contributions are added
*
* \authors F. Arebola-Pérez and A. Reyes-Lecuona, members of the 3DI-DIANA Research Group (University of Malaga)
* \b Contact: A. Reyes-Lecuona as head of 3DI-DIANA Research Group (University of Malaga): areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SAVLab (Spatial Audio Virtual Laboratory) ||
* \b Website:
*
* \b Copyright: University of Malaga - 2021
*
* \b Licence: GPLv3
*
* \b Acknowledgement: This project has received funding from Spanish Ministerio de Ciencia e Innovación under the SAVLab project (PID2019-107854GB-I00)
*/
#include <cmath>
#include <Common/GraphicEqualizer.h>
#include <Common/ErrorHandler.h>

//////////////////////////////////////////////
// CONSTRUCTOR/DESTRUCTOR

namespace Common {

	CGraphicEqualizer::CGraphicEqualizer()
	{
		
	}

	//////////////////////////////////////////////
	void CGraphicEqualizer::Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, int octaveBandStep, float Q_BPF)
	{
		bandFrequencies_Hz.clear();

		// clear filterbank
		filterBank.RemoveFilters();
		
		float f = iniFreq_Hz;
		float freqStep = std::pow(2, 1.0f / (float)octaveBandStep);

		for (int c = 0; c < bandsNumber; c++)
		{
			bandFrequencies_Hz.push_back(f);

			filterBank.AddFilter()->Setup(samplingRate, f, Q_BPF, Common::T_filterType::BANDPASS);

			f *= freqStep;
		}
	}

	//////////////////////////////////////////////

	void CGraphicEqualizer::Process(CMonoBuffer<float> &  inputBuffer, CMonoBuffer<float> & outputBuffer)
	{	
		filterBank.Process(inputBuffer, outputBuffer);
	}

	//////////////////////////////////////////////
	// Specifies the gain for each band in dB
    
	void CGraphicEqualizer::SetGains_dB(vector<float> gains_dB)
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
	void CGraphicEqualizer::ResetGains_dB()
	{
		SET_RESULT(RESULT_OK, "");

		for (int c = 0; c < filterBank.GetNumFilters(); c++)
			filterBank.GetFilter(c)->SetGeneralGain(1);
	}

	//--------------------------------------------------------------
	void CGraphicEqualizer::SetFiltersBankBandGain_dB(int bandIndex, float gain_dB)
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
	float CGraphicEqualizer::GetBandFrequency(int bandIndex)
	{
		if (bandIndex < 0 || bandIndex >= bandFrequencies_Hz.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "bad index");
			return 0;
		}
		return bandFrequencies_Hz[bandIndex];
	}
	
}// end namespace Common
