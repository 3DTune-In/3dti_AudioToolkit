/*
* \class CHearingLossSim
*
* \brief Definition of the hearing loss simulator
*
* Class to implement a a hearing loss simulator
*
* \date	July 2017
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: David Poirier-Quinot 
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

#include <HAHLSimulation/HearingLossSim.h>
#include <Common/ErrorHandler.h>
#include <cmath>

//////////////////////////////////////////////

namespace HAHLSimulation {

	void CHearingLossSim::Setup(int samplingRate, float Calibration_dBs_SPL_for_0_dBs_fs, float iniFreq_Hz, int bandsNumber, int filtersPerBand, int bufferSize, CFrequencySmearing::SmearingAlgorithm smearingAlgorithm)
	{
		// Set default switches for each independent process
		enableHearingLossSimulation.left = true;
		enableHearingLossSimulation.right = true;
		enableMultibandExpander.left = true;
		enableMultibandExpander.right = true;
		enableFrequencySmearing.left = true;
		enableFrequencySmearing.right = true;

		// Setup multiband expander
		dBs_SPL_for_0_dBs_fs = Calibration_dBs_SPL_for_0_dBs_fs;
		multibandExpanders.left.Setup(samplingRate, iniFreq_Hz, bandsNumber, filtersPerBand);
		multibandExpanders.right.Setup(samplingRate, iniFreq_Hz, bandsNumber, filtersPerBand);
		audiometries.left.assign(bandsNumber, 0.0f);
		audiometries.right.assign(bandsNumber, 0.0f);

		// Setup temporal distortion
		temporalDistortionSimulator.Setup(samplingRate, bufferSize, 
			DEFAULT_TEMPORAL_DISTORTION_SPLIT_FREQUENCY, 
			DEFAULT_TEMPORAL_DISTORTION_AMOUNT_IN_MS, 
			DEFAULT_TEMPORAL_DISTORTION_LEFTRIGHT_SYNCHRONICITY);

		// Setup frequency smearing
		//ERRORHANDLER3DTI.AddVariableWatch(WV_BUFFER_TEST);
		//ERRORHANDLER3DTI.SetWatcherLogFile(WV_BUFFER_TEST, "smearingFOutput.txt");
		frequencySmearers.left.Setup(bufferSize, samplingRate, smearingAlgorithm);
		frequencySmearers.right.Setup(bufferSize, samplingRate, smearingAlgorithm);
		frequencySmearingBypassDelay.left.Setup(bufferSize);
		frequencySmearingBypassDelay.right.Setup(bufferSize);
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetCalibration(float Calibration_dBs_SPL_for_0_dBs_fs)
	{
		dBs_SPL_for_0_dBs_fs = Calibration_dBs_SPL_for_0_dBs_fs;
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetFromAudiometry_dBHL(Common::T_ear ear, TAudiometry hearingLevels_dBHL)
	{
		// Check sizes
		ASSERT(hearingLevels_dBHL.size() == audiometries.left.size(), RESULT_ERROR_OUTOFRANGE, "Attempt to set an audiometry with wrong number of bands", "Number of bands in audiometry is correct");

		for (size_t i = 0; i < hearingLevels_dBHL.size(); i++)
		{
			SetHearingLevel_dBHL(ear, i, hearingLevels_dBHL[i]);
		}
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetHearingLevel_dBHL(Common::T_ear ear, int bandIndex, float hearingLevel_dBHL)
	{
		if (hearingLevel_dBHL > 99)
			hearingLevel_dBHL = 99;

		// Check band index
		ASSERT((bandIndex >= 0) && (bandIndex < audiometries.left.size()), RESULT_ERROR_OUTOFRANGE, "Attempt to set hearing level for a wrong band number", "Band for hearing level is correct");

		// Compute threshold
		float threshold_dBSPL = CalculateThresholdFromDBHL(hearingLevel_dBHL);
		float threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);

		// Compute ratio
		//float ratio = (DB_HL_TO_RATIO_OFFSET + hearingLevel_dBHL) / (DB_HL_TO_RATIO_OFFSET - DB_HL_TO_RATIO_FACTOR*hearingLevel_dBHL);
		float ratio = CalculateRatioFromDBHL(hearingLevel_dBHL);

		// Compute attenuation
		float attenuation = CalculateAttenuationFromDBHL(hearingLevel_dBHL);

		// Apply to corresponding ears
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			audiometries.left[bandIndex] = hearingLevel_dBHL;
			multibandExpanders.left.GetBandExpander(bandIndex)->SetThreshold(threshold_dBFS);
			multibandExpanders.left.GetBandExpander(bandIndex)->SetRatio(ratio);
			multibandExpanders.left.SetAttenuationForBand(bandIndex, attenuation);
		}
		if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			audiometries.right[bandIndex] = hearingLevel_dBHL;
			multibandExpanders.right.GetBandExpander(bandIndex)->SetThreshold(threshold_dBFS);
			multibandExpanders.right.GetBandExpander(bandIndex)->SetRatio(ratio);
			multibandExpanders.right.SetAttenuationForBand(bandIndex, attenuation);
		}
	}

	//////////////////////////////////////////////

	float CHearingLossSim::GetHearingLevel_dBHL(Common::T_ear ear, int bandIndex)
	{
		if ((bandIndex < 0) || (bandIndex >= audiometries.left.size()))
		{
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to get hearing level for a wrong band number");
			return 0.0f;
		}
		if ((ear != Common::T_ear::LEFT) && (ear != Common::T_ear::RIGHT))
		{
			SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Bad ear specification when attempting to get hearing level");
			return 0.0f;
		}

		if (ear == Common::T_ear::LEFT)
			return audiometries.left[bandIndex];
		if (ear == Common::T_ear::RIGHT)
			return audiometries.right[bandIndex];

		return 0.0f;
	}

	//////////////////////////////////////////////

	int CHearingLossSim::GetNumberOfBands()
	{
		return (int)audiometries.left.size();
	}

	//////////////////////////////////////////////

	float CHearingLossSim::GetBandFrequency(int bandIndex)
	{
		// Correct band index will be checked inside CMultibandExpander::GetBandFrequency
		return multibandExpanders.left.GetBandFrequency(bandIndex);
	}

	//////////////////////////////////////////////

	Common::CDynamicExpanderMono* CHearingLossSim::GetBandExpander(Common::T_ear ear, int bandIndex)
	{
		ASSERT((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::RIGHT), RESULT_ERROR_CASENOTDEFINED, "Attempt to get the hearing loss expander of one band with a wrong ear specification", "");

		if (ear == Common::T_ear::LEFT)
			return multibandExpanders.left.GetBandExpander(bandIndex);
		else
			return multibandExpanders.right.GetBandExpander(bandIndex);
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetAttackForAllBands(Common::T_ear ear, float attack)
	{
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			for (size_t i = 0; i < audiometries.left.size(); i++)
				multibandExpanders.left.GetBandExpander(i)->SetAttack(attack);
		}
		if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			for (size_t i = 0; i < audiometries.right.size(); i++)
				multibandExpanders.right.GetBandExpander(i)->SetAttack(attack);
		}
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetReleaseForAllBands(Common::T_ear ear, float release)
	{
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			for (size_t i = 0; i < audiometries.left.size(); i++)
				multibandExpanders.left.GetBandExpander(i)->SetRelease(release);
		}
		if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			for (size_t i = 0; i < audiometries.right.size(); i++)
				multibandExpanders.right.GetBandExpander(i)->SetRelease(release);
		}
	}

	//////////////////////////////////////////////

	//void CHearingLossSim::Process(T_ear ear, CMonoBuffer<float> &inputBuffer, CMonoBuffer<float> &outputBuffer)
	//{
	//	ASSERT ((ear == T_ear::LEFT) || (ear == T_ear::RIGHT), RESULT_ERROR_CASENOTDEFINED, "Cannot process hearing loss for both ears if input is Mono", "");
	//
	//	if (ear == T_ear::LEFT)
	//	{
	//		multibandExpanders.left.Process(inputBuffer, outputBuffer);		
	//	}
	//	else
	//	{
	//		multibandExpanders.right.Process(inputBuffer, outputBuffer);
	//	}
	//
	//	// TO DO: Process jitterSimulator
	//}

	//////////////////////////////////////////////

	//void CHearingLossSim::Process(CStereoBuffer<float> &inputBuffer, CStereoBuffer<float> &outputBuffer)
	//{
	//	Common::CEarPair<CMonoBuffer<float>> inputPair;		
	//	Common::CEarPair<CMonoBuffer<float>> outputPair;		
	//	inputBuffer.Deinterlace(inputPair.left, inputPair.right);
	//	outputBuffer.Deinterlace(outputPair.left, outputPair.right);

	//	Process(inputPair, outputPair);

	//	outputBuffer.Interlace(outputPair.left, outputPair.right);
	//}

	//////////////////////////////////////////////

	
	void CHearingLossSim::Process(Common::CEarPair<CMonoBuffer<float>> &inputBuffer, Common::CEarPair<CMonoBuffer<float>> &outputBuffer)
	{
		// Bypass all
		if ((!enableHearingLossSimulation.left) && (!enableHearingLossSimulation.right))
		{
			outputBuffer = inputBuffer;
			return;
		}

		// Process temporal distortion 
		Common::CEarPair<CMonoBuffer<float>> asynchronyOutput;
		asynchronyOutput.left.Fill(outputBuffer.left.GetNsamples(), 0.0f);
		asynchronyOutput.right.Fill(outputBuffer.right.GetNsamples(), 0.0f);		
		temporalDistortionSimulator.Process(inputBuffer, asynchronyOutput);

		// Process frequency smearing
		Common::CEarPair<CMonoBuffer<float>> smearingOutput = asynchronyOutput;
		// Left:
		if (enableFrequencySmearing.left && enableHearingLossSimulation.left)
		{
			frequencySmearers.left.Process(asynchronyOutput.left, smearingOutput.left);			
			frequencySmearingBypassDelay.left.Process(smearingOutput.left, asynchronyOutput.left);	// Do a dummy process of the bypass delay buffer, to minimize clicks when enabling/disabling one ear
		}
		else
		{
			if (enableFrequencySmearing.right && enableHearingLossSimulation.right)
				frequencySmearingBypassDelay.left.Process(asynchronyOutput.left, smearingOutput.left);	// If only left ear is bypassed
			//else
			//	smearingOutput.left = asynchronyOutput.left;	// If both ears are bypassed
		}
		// Right:
		if (enableFrequencySmearing.right && enableHearingLossSimulation.right)
		{
			frequencySmearers.right.Process(asynchronyOutput.right, smearingOutput.right);
			frequencySmearingBypassDelay.right.Process(smearingOutput.right, asynchronyOutput.right);	// Do a dummy process of the bypass delay buffer, to minimize clicks when enabling/disabling one ear
		}
		else
		{
			if (enableFrequencySmearing.left && enableHearingLossSimulation.left)
				frequencySmearingBypassDelay.right.Process(asynchronyOutput.right, smearingOutput.right);	// If only right ear is bypassed
			//else
			//	smearingOutput.right = asynchronyOutput.right;	// If both ears are bypassed
		}

		// Process audiogram (multiband expanders)
		Common::CEarPair<CMonoBuffer<float>> audiogramOutput;
		audiogramOutput.left.Fill(outputBuffer.left.GetNsamples(), 0.0f);
		audiogramOutput.right.Fill(outputBuffer.right.GetNsamples(), 0.0f);

		if( enableMultibandExpander.left && enableHearingLossSimulation.left )
			multibandExpanders.left.Process(smearingOutput.left, audiogramOutput.left);
		else
			audiogramOutput.left = smearingOutput.left;
		if (enableMultibandExpander.right && enableHearingLossSimulation.right)
			multibandExpanders.right.Process(smearingOutput.right, audiogramOutput.right);
		else
			audiogramOutput.right = smearingOutput.right;

		// Set final output buffer
		outputBuffer = audiogramOutput;		

		// WATCHER
		WATCH(WV_HEARINGLOSS_OUTPUT_LEFT, outputBuffer.left, CMonoBuffer<float>);
		WATCH(WV_HEARINGLOSS_OUTPUT_RIGHT, outputBuffer.right, CMonoBuffer<float>);
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetAttenuationForBand(Common::T_ear ear, int bandIndex, float attenuation)
	{
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			multibandExpanders.left.SetAttenuationForBand(bandIndex, attenuation);
		}
		else if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			multibandExpanders.right.SetAttenuationForBand(bandIndex, attenuation);
		}
	}

	//////////////////////////////////////////////

	float CHearingLossSim::GetAttenuationForBand(Common::T_ear ear, int bandIndex) 
	{
		if (ear == Common::T_ear::LEFT)
			return multibandExpanders.left.GetAttenuationForBand(bandIndex);
		if (ear == Common::T_ear::RIGHT)
			return multibandExpanders.right.GetAttenuationForBand(bandIndex);
		return 0.0f;
	}

	//////////////////////////////////////////////

	float CHearingLossSim::CalculateDBFSFromDBSPL(float dBSPL)
	{
		return dBSPL - dBs_SPL_for_0_dBs_fs;
	}

	//////////////////////////////////////////////

	float CHearingLossSim::CalculateDBSPLFromDBFS(float dBFS)
	{
		return dBFS + dBs_SPL_for_0_dBs_fs;
	}

	//////////////////////////////////////////////

	float CHearingLossSim::CalculateThresholdFromDBHL(float dBHL)
	{
		float limitedDBHL = dBHL;
		if (limitedDBHL > 120.0f)
			limitedDBHL = 120.0f;

		return T100 - A100 + (A100*limitedDBHL) * 0.01f;
	}

	//////////////////////////////////////////////

	float CHearingLossSim::CalculateRatioFromDBHL(float dBHL)
	{
		float limitedDBHL = dBHL;
		if( limitedDBHL > 100.0f )
			limitedDBHL = 100.0f;

		float den = ( T100 - A100 + (A100 - T100) * limitedDBHL * 0.01f );
		return fabs(den) < 0.0000001 ? 0 : (T100 - A100) / den;
	}

	//////////////////////////////////////////////

	float CHearingLossSim::CalculateAttenuationFromDBHL(float dBHL)
	{
		return A100*dBHL*0.01f;
	}

	//////////////////////////////////////////////

	CTemporalDistortionSimulator* CHearingLossSim::GetTemporalDistortionSimulator()
	{
		return &temporalDistortionSimulator;
	}

	//////////////////////////////////////////////

	CFrequencySmearing* CHearingLossSim::GetFrequencySmearingSimulator(Common::T_ear ear)
	{		
		if (ear == Common::T_ear::LEFT)
			return &frequencySmearers.left;
		if (ear == Common::T_ear::RIGHT)
			return &frequencySmearers.right;
	
		SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Attempt to get frequency smearing simulator for both or none ears");
		return nullptr;
	}

	//////////////////////////////////////////////

	void CHearingLossSim::EnableMultibandExpander(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			EnableMultibandExpander(Common::T_ear::LEFT);
			EnableMultibandExpander(Common::T_ear::RIGHT);
			return;
		}

		if (ear == Common::T_ear::LEFT)
			enableMultibandExpander.left = true;
		if (ear == Common::T_ear::RIGHT)
			enableMultibandExpander.right = true;
	}

	//////////////////////////////////////////////

	void CHearingLossSim::DisableMultibandExpander(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableMultibandExpander(Common::T_ear::LEFT);
			DisableMultibandExpander(Common::T_ear::RIGHT);
		}
		if (ear == Common::T_ear::LEFT)
			enableMultibandExpander.left = false;
		if (ear == Common::T_ear::RIGHT)
			enableMultibandExpander.right = false;
	}

	//////////////////////////////////////////////

	void CHearingLossSim::EnableTemporalDistortion(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			EnableTemporalDistortion(Common::T_ear::LEFT);
			EnableTemporalDistortion(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			temporalDistortionSimulator.EnableTemporalDistortionSimulator(Common::T_ear::LEFT);			
		if (ear == Common::T_ear::RIGHT)
			temporalDistortionSimulator.EnableTemporalDistortionSimulator(Common::T_ear::RIGHT);
	}

	//////////////////////////////////////////////

	void CHearingLossSim::DisableTemporalDistortion(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableTemporalDistortion(Common::T_ear::LEFT);
			DisableTemporalDistortion(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			temporalDistortionSimulator.DisableTemporalDistortionSimulator(Common::T_ear::LEFT);
		if (ear == Common::T_ear::RIGHT)
			temporalDistortionSimulator.DisableTemporalDistortionSimulator(Common::T_ear::RIGHT);
	}

	//////////////////////////////////////////////

	void CHearingLossSim::EnableHearingLossSimulation(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			EnableHearingLossSimulation(Common::T_ear::LEFT);
			EnableHearingLossSimulation(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableHearingLossSimulation.left = true;
		if (ear == Common::T_ear::RIGHT)
			enableHearingLossSimulation.right = true;
	}

	//////////////////////////////////////////////

	void CHearingLossSim::DisableHearingLossSimulation(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableHearingLossSimulation(Common::T_ear::LEFT);
			DisableHearingLossSimulation(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableHearingLossSimulation.left = false;
		if (ear == Common::T_ear::RIGHT)
			enableHearingLossSimulation.right = false;

	}

	//////////////////////////////////////////////

	void CHearingLossSim::EnableFrequencySmearing(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			EnableFrequencySmearing(Common::T_ear::LEFT);
			EnableFrequencySmearing(Common::T_ear::RIGHT);
			return;
		}
		if(ear == Common::T_ear::LEFT ) enableFrequencySmearing.left  = true;
		if(ear == Common::T_ear::RIGHT) enableFrequencySmearing.right = true;
	}

	//////////////////////////////////////////////

	void CHearingLossSim::DisableFrequencySmearing(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableFrequencySmearing(Common::T_ear::LEFT);
			DisableFrequencySmearing(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableFrequencySmearing.left = false;
		if (ear == Common::T_ear::RIGHT)
			enableFrequencySmearing.right = false;
	}
}// end namespace HAHLSimulation

