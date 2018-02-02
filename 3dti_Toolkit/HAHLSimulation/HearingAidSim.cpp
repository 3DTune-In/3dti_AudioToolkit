/**
* \class CHearingAidSim
*
* \brief Definition of the hearing aid simulator
* \date	February 2016
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

#include <HAHLSimulation/HearingAidSim.h>
#include <HAHLSimulation/Fig6Algorithm.h>
#include <Common/ErrorHandler.h>
#include <cmath>

#define NUM_STEPS_TO_INTEGRATE_CARDIOID_FOR_REVERB 100

namespace HAHLSimulation {

	CHearingAidSim::CHearingAidSim()
	{
		Reset(Common::T_ear::BOTH);
	}

	//////////////////////////////////////////////
		/* Determines the number of bands of the equalizer as well as the center frequencies.
		It also configures the internal filters.
		iniFreq_Hz must be any of the 31 center frequency values for the 31 ISO bands:
		20, 25, 31, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000
		2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000
		octaveBandStep especifies the frequency step to determine the band with of each band.

		for instance, if octaveBandStep is equal to 1 an octave will be used:
		setup(  125, 7, 1 ); will produce center frequencies of: 125, 250, 500, 1000, 2000, 4000, 8000

		If octaveBandStep is equal to 3 a third of octave will be use instead:
		setup(  125, 7, 3 ); will produce center frequencies of: 125, 160, 200, 250, 315, 400, 500

		In addition to the equalizer, a biquad low pass filter and a biquad high pass filter are used to model the HA
		Q parameter for low, band and high pass filter is also considered.
		*/
	void CHearingAidSim::Setup(int samplingRate, int numLevels, float iniFreq_Hz, int bandsNumber, int octaveBandStep, float lpf_CutoffFreqHz, float hpf_CutoffFreqHz, float Q_LPF, float Q_BPF, float Q_HPF)
	{
		if (octaveBandStep <= 0)
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "octaveBandStep must be greater than 0");
			return;
		}

		lowPassFilter.left.AddFilter();
		lowPassFilter.right.AddFilter();
		highPassFilter.left.AddFilter();
		highPassFilter.right.AddFilter();

		SetLowPassFilter(lpf_CutoffFreqHz, Q_LPF);
		SetHighPassFilter(hpf_CutoffFreqHz, Q_HPF);

		dynamicEqualizer.left.Setup(samplingRate, numLevels, iniFreq_Hz, bandsNumber, octaveBandStep, Q_BPF);
		dynamicEqualizer.right.Setup(samplingRate, numLevels, iniFreq_Hz, bandsNumber, octaveBandStep, Q_BPF);
	}

	//////////////////////////////////////////////
		// Specifies the gain for each band in dB
	void CHearingAidSim::SetAllBandGains_dB(Common::T_ear ear, vector<float> gains_dB)
	{
		if (ear == Common::T_ear::BOTH)
		{
			SetAllBandGains_dB(Common::T_ear::LEFT, gains_dB);
			SetAllBandGains_dB(Common::T_ear::RIGHT, gains_dB);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			dynamicEqualizer.left.SetGains_dB(gains_dB);
		if (ear == Common::T_ear::RIGHT);
			dynamicEqualizer.right.SetGains_dB(gains_dB);
	}

	//////////////////////////////////////////////
	
	void CHearingAidSim::Reset(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			Reset(Common::T_ear::LEFT);
			Reset(Common::T_ear::RIGHT);
			return;
		}

		enableQuantizationBeforeEqualizer = false;
		enableQuantizationAfterEqualizer = false;
		quantizationBits = 16;

		if (ear == Common::T_ear::LEFT)
		{
			overallGain.left = 1.0f;
			normalizationReference.left = 0;
			normalizationEnabled.left = false;
			dynamicEqualizer.left.ResetGains_dB();
		}

		if (ear == Common::T_ear::RIGHT)
		{
			overallGain.right = 1.0f;
			normalizationReference.right = 0;
			normalizationEnabled.right = false;
			dynamicEqualizer.right.ResetGains_dB();
		}
	}

	//////////////////////////////////////////////
		// Specifies the gain for a specific level and band
	void CHearingAidSim::SetDynamicEqualizerBandGain_dB(Common::T_ear ear, int levelIndex, int bandIndex, float gain_dB)
	{
		if (ear == Common::T_ear::BOTH)
		{
			SetDynamicEqualizerBandGain_dB(Common::T_ear::LEFT, levelIndex, bandIndex, gain_dB);
			SetDynamicEqualizerBandGain_dB(Common::T_ear::RIGHT, levelIndex, bandIndex, gain_dB);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			dynamicEqualizer.left.SetLevelBandGain_dB(levelIndex, bandIndex, gain_dB);
		if (ear == Common::T_ear::RIGHT)
			dynamicEqualizer.right.SetLevelBandGain_dB(levelIndex, bandIndex, gain_dB);
	}

	//////////////////////////////////////////////
		// Specifies the gain for a specific level and band
	void CHearingAidSim::SetDynamicEqualizerLevelThreshold(Common::T_ear ear, int levelIndex, float threshold_dBfs)
	{
		if (ear == Common::T_ear::BOTH)
		{
			SetDynamicEqualizerLevelThreshold(Common::T_ear::LEFT, levelIndex, threshold_dBfs);
			SetDynamicEqualizerLevelThreshold(Common::T_ear::RIGHT, levelIndex, threshold_dBfs);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			dynamicEqualizer.left.SetLevelThreshold(levelIndex, threshold_dBfs);
		if (ear == Common::T_ear::RIGHT)
			dynamicEqualizer.right.SetLevelThreshold(levelIndex, threshold_dBfs);
	}

	//////////////////////////////////////////////

	// Process the samples in the inputBuffer through the hearing aid simulator
	void CHearingAidSim::Process(Common::CEarPair <CMonoBuffer<float>> &inputBuffer, Common::CEarPair <CMonoBuffer<float>> &outputBuffer)
	{
		//SET_RESULT(RESULT_OK, "");

		outputBuffer.left.resize(inputBuffer.left.size());
		outputBuffer.right.resize(inputBuffer.right.size());

		// Bypass channel/s
		if (!enableHearingAidSimulation.left) 		
			outputBuffer.left = inputBuffer.left;		
		if (!enableHearingAidSimulation.right)
			outputBuffer.right = inputBuffer.right;
					
		// Process normalization
		if (normalizationEnabled.left)  
			ProcessNormalization(Common::T_ear::LEFT, normalizationReference.left);
		else                        
			ResetNormalization(Common::T_ear::LEFT);

		if (normalizationEnabled.right)  
			ProcessNormalization(Common::T_ear::RIGHT, normalizationReference.right);
		else                        
			ResetNormalization(Common::T_ear::RIGHT);

		// Process quantization BEFORE equalizer
		if (enableQuantizationBeforeEqualizer)
		{
			if (enableHearingAidSimulation.left)
				ProcessQuantizationNoise(inputBuffer.left);
			if (enableHearingAidSimulation.right)
				ProcessQuantizationNoise(inputBuffer.right);
		}

		// Process dynamic equalizer
		if (enableHearingAidSimulation.left)
			dynamicEqualizer.left.Process(inputBuffer.left, outputBuffer.left);
		if (enableHearingAidSimulation.right)
			dynamicEqualizer.right.Process(inputBuffer.right, outputBuffer.right);

		// Process low and high pass filters
		if (enableHearingAidSimulation.left)
		{
			lowPassFilter.left.Process(outputBuffer.left);			
			highPassFilter.left.Process(outputBuffer.left);			
		}
		if (enableHearingAidSimulation.right)
		{
			lowPassFilter.right.Process(outputBuffer.right);
			highPassFilter.right.Process(outputBuffer.right);
		}

		// Process quantization AFTER equalizer
		if (enableQuantizationAfterEqualizer)
		{
			if (enableHearingAidSimulation.left)
				ProcessQuantizationNoise(outputBuffer.left);
			if (enableHearingAidSimulation.right)
				ProcessQuantizationNoise(outputBuffer.right);
		}

		// Apply overall gain
		if (enableHearingAidSimulation.left)
			outputBuffer.left.ApplyGain(overallGain.left);
		if (enableHearingAidSimulation.right)
			outputBuffer.right.ApplyGain(overallGain.right);

		// WATCHER
		WATCH(WV_HEARINGAID_OUTPUT_LEFT, outputBuffer.left, CMonoBuffer<float>);
		WATCH(WV_HEARINGAID_OUTPUT_RIGHT, outputBuffer.right, CMonoBuffer<float>);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::ProcessQuantizationNoise(CMonoBuffer<float> &buffer)
	{
		float numValues = std::pow(2, round(quantizationBits));
		float tmp;

		if (numValues > 0)
		{
			for (int c = 0; c < buffer.size(); c ++)
			{
				float sample = buffer[c];				

				if (sample > 1.0f) 
					sample = 1.0f;
				else if (sample < -1.0f) 
					sample = -1.0f;

				tmp = (int)(numValues * (0.5 + 0.5f * sample));
				buffer[c] = (tmp / numValues) * 2.0f - 1.0f;
			}
		}
	}

	//////////////////////////////////////////////

	void CHearingAidSim::SetLowPassFilter(float cutoffFreq_hz, float Q)
	{
		//SET_RESULT(RESULT_OK, "");

		for (int c = 0; c < lowPassFilter.left.GetNumFilters(); c++)
			lowPassFilter.left.GetFilter(c)->SetCoefficients(cutoffFreq_hz, Q, Common::T_filterType::LOWPASS);

		for (int c = 0; c < lowPassFilter.right.GetNumFilters(); c++)
			lowPassFilter.right.GetFilter(c)->SetCoefficients(cutoffFreq_hz, Q, Common::T_filterType::LOWPASS);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::SetHighPassFilter(float cutoffFreq_hz, float Q)
	{
		//SET_RESULT(RESULT_OK, "");

		for (int c = 0; c < highPassFilter.left.GetNumFilters(); c++)
			highPassFilter.left.GetFilter(c)->SetCoefficients(cutoffFreq_hz, Q, Common::T_filterType::HIGHPASS);

		for (int c = 0; c < highPassFilter.right.GetNumFilters(); c++)
			highPassFilter.right.GetFilter(c)->SetCoefficients(cutoffFreq_hz, Q, Common::T_filterType::HIGHPASS);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::SetDynamicEqualizerUsingFig6(Common::T_ear ear, vector <float> &earLoss, float dBs_SPL_for_0_dBs_fs)
	{
		if (dynamicEqualizer.left.GetNumLevels() != 3)
		{
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "The current number of levels in the HA must be 3 to apply the Fig6 algorithm");
			return;
		}

		if (dynamicEqualizer.left.GetNumBands() != earLoss.size())
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The number of values in earLoss does not agrre with the current number of bands");
			return;
		}

		// We need tha the level 0 is the central curve because the level 0 is taken as reference to
		// apply the compression percentage in the dynamic equalizer
		if (ear == Common::T_ear::BOTH)
		{
			SetDynamicEqualizerUsingFig6(Common::T_ear::LEFT, earLoss, dBs_SPL_for_0_dBs_fs);
			SetDynamicEqualizerUsingFig6(Common::T_ear::RIGHT, earLoss, dBs_SPL_for_0_dBs_fs);
			return;
		}
		if (ear == Common::T_ear::LEFT)
		{
			dynamicEqualizer.left.SetLevelThreshold(1, 40.0f - dBs_SPL_for_0_dBs_fs);
			dynamicEqualizer.left.SetLevelThreshold(0, 65.0f - dBs_SPL_for_0_dBs_fs);
			dynamicEqualizer.left.SetLevelThreshold(2, 95.0f - dBs_SPL_for_0_dBs_fs);
		}
		if (ear == Common::T_ear::RIGHT)
		{
			dynamicEqualizer.right.SetLevelThreshold(1, 40.0f - dBs_SPL_for_0_dBs_fs);
			dynamicEqualizer.right.SetLevelThreshold(0, 65.0f - dBs_SPL_for_0_dBs_fs);
			dynamicEqualizer.right.SetLevelThreshold(2, 95.0f - dBs_SPL_for_0_dBs_fs);
		}

		for (int c = 0; c < earLoss.size(); c++)
		{
			if (ear == Common::T_ear::LEFT)
			{
				dynamicEqualizer.left.SetLevelBandGain_dB(1, c, GetFig6AlgorithmGainFor40dB_SPL(earLoss[c]));
				dynamicEqualizer.left.SetLevelBandGain_dB(0, c, GetFig6AlgorithmGainFor65dB_SPL(earLoss[c]));
				dynamicEqualizer.left.SetLevelBandGain_dB(2, c, GetFig6AlgorithmGainFor95dB_SPL(earLoss[c]));
			}
			if (ear == Common::T_ear::RIGHT)
			{
				dynamicEqualizer.right.SetLevelBandGain_dB(1, c, GetFig6AlgorithmGainFor40dB_SPL(earLoss[c]));
				dynamicEqualizer.right.SetLevelBandGain_dB(0, c, GetFig6AlgorithmGainFor65dB_SPL(earLoss[c]));
				dynamicEqualizer.right.SetLevelBandGain_dB(2, c, GetFig6AlgorithmGainFor95dB_SPL(earLoss[c]));
			}
		}
	}

	//////////////////////////////////////////////
	void CHearingAidSim::ProcessNormalization(Common::T_ear ear, float referenceValue_dB)
	{
		if (dynamicEqualizer.left.GetNumLevels() <= 0)
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "The number of levels must be greater than 0");
			return;
		}
		if (dynamicEqualizer.left.GetNumBands() <= 0)
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "The number of bands must be greater than 0");
			return;
		}
		if (ear == Common::T_ear::BOTH)
		{
			ProcessNormalization(Common::T_ear::LEFT , referenceValue_dB);
			ProcessNormalization(Common::T_ear::RIGHT, referenceValue_dB);
			return;
		}

		// At first we determine the offset value that must be applied to all the curves:

		float max = ear == Common::T_ear::LEFT ? dynamicEqualizer.left.GetLevelBandGain_dB(0, 0) : dynamicEqualizer.right.GetLevelBandGain_dB(0, 0);
		for (int c = 1; c < dynamicEqualizer.left.GetNumBands(); c++)
		{
			float level = ear == Common::T_ear::LEFT ? dynamicEqualizer.left.GetLevelBandGain_dB(0, c) : dynamicEqualizer.right.GetLevelBandGain_dB(0, c);
			if (level > max)
				max = level;
		}

		float offset = referenceValue_dB - max;

		if (offset > 0)
			offset = 0;

		if (ear == Common::T_ear::LEFT) dynamicEqualizer.left .SetOveralOffset_dB(offset);
		else                            dynamicEqualizer.right.SetOveralOffset_dB(offset);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::ResetNormalization(Common::T_ear ear)
	{
		if (ear == Common::T_ear::LEFT)
			dynamicEqualizer.left.SetOveralOffset_dB(0);
		else
			dynamicEqualizer.right.SetOveralOffset_dB(0);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::SetNormalizationLevel(Common::T_ear ear, float referenceValue_dB)
	{
		if (ear == Common::T_ear::BOTH)
		{
			SetNormalizationLevel(Common::T_ear::LEFT, referenceValue_dB);
			SetNormalizationLevel(Common::T_ear::RIGHT, referenceValue_dB);
			return;
		}
		if (ear == Common::T_ear::LEFT) 
		{ 
			normalizationReference.left = referenceValue_dB; 
			ProcessNormalization(Common::T_ear::LEFT, normalizationReference.left); 
		}
		if (ear == Common::T_ear::RIGHT) 
		{ 
			normalizationReference.right = referenceValue_dB; 
			ProcessNormalization(Common::T_ear::RIGHT, normalizationReference.right); 
		}
	}

	//////////////////////////////////////////////

	void CHearingAidSim::SetEnableNormalization(Common::T_ear ear, bool _enabled)
	{
		if (ear == Common::T_ear::BOTH)
		{
			SetEnableNormalization(Common::T_ear::LEFT, _enabled);
			SetEnableNormalization(Common::T_ear::RIGHT, _enabled);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			normalizationEnabled.left = _enabled;
		if (ear == Common::T_ear::RIGHT)
			normalizationEnabled.right = _enabled;

		if (normalizationEnabled.left)  
			ProcessNormalization(Common::T_ear::LEFT, normalizationReference.left);
		else                        
			ResetNormalization(Common::T_ear::LEFT);

		if (normalizationEnabled.right)  
			ProcessNormalization(Common::T_ear::RIGHT, normalizationReference.right);
		else                        
			ResetNormalization(Common::T_ear::RIGHT);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::EnableNormalization(Common::T_ear ear)
	{
		SetEnableNormalization(ear, true);
	}

	//////////////////////////////////////////////

	void CHearingAidSim::DisableNormalization(Common::T_ear ear)
	{
		SetEnableNormalization(ear, false);
	}

	//////////////////////////////////////////////	
	
	void CHearingAidSim::SetOverallGain(Common::T_ear ear, float gain)
	{
		if (ear == Common::T_ear::BOTH)
		{
			SetOverallGain(Common::T_ear::LEFT, gain);
			SetOverallGain(Common::T_ear::RIGHT, gain);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			overallGain.left = gain;
		if (ear == Common::T_ear::RIGHT)
			overallGain.right = gain;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::EnableQuantizationBeforeEqualizer()
	{
		enableQuantizationBeforeEqualizer = true;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::DisableQuantizationBeforeEqualizer()
	{
		enableQuantizationBeforeEqualizer = false;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::EnableQuantizationAfterEqualizer()
	{
		enableQuantizationAfterEqualizer = true;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::DisableQuantizationAfterEqualizer()
	{
		enableQuantizationAfterEqualizer = false;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::SetQuantizationBits(int nBits)
	{
		quantizationBits = nBits;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::EnableHearingAidSimulation(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			EnableHearingAidSimulation(Common::T_ear::LEFT);
			EnableHearingAidSimulation(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableHearingAidSimulation.left = true;
		if (ear == Common::T_ear::RIGHT)
			enableHearingAidSimulation.right = true;
	}

	//////////////////////////////////////////////	

	void CHearingAidSim::DisableHearingAidSimulation(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableHearingAidSimulation(Common::T_ear::LEFT);
			DisableHearingAidSimulation(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableHearingAidSimulation.left = false;
		if (ear == Common::T_ear::RIGHT)
			enableHearingAidSimulation.right = false;
	}

	//////////////////////////////////////////////	

	CDynamicEqualizer* CHearingAidSim::GetDynamicEqualizer(Common::T_ear ear) 
	{ 
		if (ear == Common::T_ear::LEFT)
			return &dynamicEqualizer.left; 
		if (ear == Common::T_ear::RIGHT)
			return &dynamicEqualizer.right;

		SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Attempt to get HA dynamic equalizer for an ear other than LEFT or RIGHT");
		return nullptr;
	}

}// end namespace HAHLSimulation
