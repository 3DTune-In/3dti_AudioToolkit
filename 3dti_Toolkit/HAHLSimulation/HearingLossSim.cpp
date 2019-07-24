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

	void CHearingLossSim::Setup(int samplingRate, float Calibration_dBs_SPL_for_0_dBs_fs, int bandsNumber, int bufferSize)
	{
		// Set default switches for each independent process
		enableHearingLossSimulation.left = true;
		enableHearingLossSimulation.right = true;
		enableMultibandExpander.left = true;
		enableMultibandExpander.right = true;
		enableFrequencySmearing.left = false;
		enableFrequencySmearing.right = false;

		// Setup multiband expander calibration and number of bands
		dBs_SPL_for_0_dBs_fs = Calibration_dBs_SPL_for_0_dBs_fs;
		audiometries.left.assign(bandsNumber, 0.0f);
		audiometries.right.assign(bandsNumber, 0.0f);

		// Setup temporal distortion
		temporalDistortionSimulator.Setup(samplingRate, bufferSize, 
			DEFAULT_TEMPORAL_DISTORTION_SPLIT_FREQUENCY, 
			DEFAULT_TEMPORAL_DISTORTION_AMOUNT_IN_MS, 
			DEFAULT_TEMPORAL_DISTORTION_LEFTRIGHT_SYNCHRONICITY);

		// Setup frequency smearing
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

		// Compute attenuation
		float attenuation = CalculateAttenuationFromDBHL(hearingLevel_dBHL);
		
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			audiometries.left[bandIndex] = hearingLevel_dBHL;
			multibandExpanders.left->SetAttenuationForOctaveBand(bandIndex, attenuation);
			CMultibandExpander* multibandExpander = multibandExpanders.left.get();
			SetMultibandExpanderParameters(multibandExpander, bandIndex, audiometries.left, multibandExpanders.left->GetFilterGrouping());
		}
		if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH)) 
		{
			audiometries.right[bandIndex] = hearingLevel_dBHL;
			multibandExpanders.right->SetAttenuationForOctaveBand(bandIndex, attenuation);
			CMultibandExpander* multibandExpander = multibandExpanders.right.get();
			SetMultibandExpanderParameters(multibandExpander, bandIndex, audiometries.right, multibandExpanders.right->GetFilterGrouping());
		}

		
	}

	void CHearingLossSim::SetMultibandExpanderParameters(CMultibandExpander* multibandExpander, int bandIndex, TAudiometry audiometry, bool filterGrouping)
	{

		float threshold_dBSPL, threshold_dBFS, ratio;
		float previousFactor, posteriorFactor;
		float previousBandFrequency, posteriorBandFrequency, previousBandAttenuation, posteriorBandAttenuation, expanderFrequency;

		float bandFrequency = multibandExpander->GetOctaveBandFrequency(bandIndex);
		float bandAttenuation = multibandExpander->GetAttenuationForOctaveBand(bandIndex);

		if (bandIndex == 0)
		{
			posteriorBandFrequency = multibandExpander->GetOctaveBandFrequency(1);
			posteriorBandAttenuation = multibandExpander->GetAttenuationForOctaveBand(1);

			for (int i = 0; i < multibandExpander->GetNumBands(filterGrouping); i++)
			{
				expanderFrequency = multibandExpander->GetBandFrequency(i, filterGrouping);

				if (expanderFrequency < bandFrequency)
				{
					threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(bandAttenuation));
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);

					ratio = CalculateRatioFromDBHL(audiometry[0]);

				}
				else if (expanderFrequency > bandFrequency && expanderFrequency < posteriorBandFrequency)
				{
					previousFactor = (posteriorBandFrequency - expanderFrequency) / (posteriorBandFrequency - bandFrequency);
					posteriorFactor = (expanderFrequency - bandFrequency) / (posteriorBandFrequency - bandFrequency);
				
					if (bandIndex == i)
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * bandAttenuation + posteriorFactor * posteriorBandAttenuation));
					}
					else
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * multibandExpander->GetAttenuationForOctaveBand(i) + posteriorFactor * multibandExpander->GetAttenuationForOctaveBand(i + 1)));
					}
					
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(previousFactor * audiometry[0] + posteriorFactor * audiometry[1]);

				}
				else if (abs(expanderFrequency - bandFrequency) < 0.1f)
				{
					threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(bandAttenuation));
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(audiometry[bandIndex]);
				}
				else break;

				multibandExpander->GetBandExpander(i, filterGrouping)->SetThreshold(threshold_dBFS);
				multibandExpander->GetBandExpander(i, filterGrouping)->SetRatio(ratio);
			}

		}
		else if (bandIndex == audiometry.size() - 1)
		{
			previousBandFrequency = multibandExpander->GetOctaveBandFrequency(bandIndex - 1);
			previousBandAttenuation = multibandExpander->GetAttenuationForOctaveBand(audiometry.size() - 2);

			for (int i = multibandExpander->GetNumBands(filterGrouping) - 1; i >= 0; i--)
			{
				expanderFrequency = multibandExpander->GetBandFrequency(i, filterGrouping);

				if (expanderFrequency > bandFrequency)
				{
					threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(bandAttenuation));
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);

					ratio = CalculateRatioFromDBHL(audiometry[audiometry.size() - 1]);
				}
				else if (expanderFrequency < bandFrequency && expanderFrequency > previousBandFrequency)
				{

					previousFactor = (bandFrequency - expanderFrequency) / (bandFrequency - previousBandFrequency);
					posteriorFactor = (expanderFrequency - previousBandFrequency) / (bandFrequency - previousBandFrequency);
					
					if (bandIndex == i)
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * bandAttenuation + posteriorFactor * previousBandAttenuation));
					}
					else
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * multibandExpander->GetAttenuationForOctaveBand(i) + posteriorFactor * multibandExpander->GetAttenuationForOctaveBand(i - 1)));
					}
					
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(previousFactor * audiometry[audiometry.size() - 2] + posteriorFactor * audiometry[audiometry.size() - 1]);
				}
				else if (abs(expanderFrequency - bandFrequency) < 0.1f)
				{
					threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(bandAttenuation));
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(audiometry[bandIndex]);
				}
				else break;

				multibandExpander->GetBandExpander(i, filterGrouping)->SetThreshold(threshold_dBFS);
				multibandExpander->GetBandExpander(i, filterGrouping)->SetRatio(ratio);
			}
		}
		else
		{
			previousBandFrequency = multibandExpander->GetOctaveBandFrequency(bandIndex - 1);
			posteriorBandFrequency = multibandExpander->GetOctaveBandFrequency(bandIndex + 1);
			previousBandAttenuation = multibandExpander->GetAttenuationForOctaveBand(bandIndex - 1);
			posteriorBandAttenuation = multibandExpander->GetAttenuationForOctaveBand(bandIndex + 1);

			for (int i = 0; i < multibandExpander->GetNumBands(filterGrouping); i++)
			{
				expanderFrequency = multibandExpander->GetBandFrequency(i, filterGrouping);

				if (expanderFrequency >= posteriorBandFrequency) break;

				if (expanderFrequency <= previousBandFrequency) continue;

				if (expanderFrequency < bandFrequency && expanderFrequency > previousBandFrequency)
				{
					previousFactor = (bandFrequency - expanderFrequency) / (bandFrequency - previousBandFrequency);
					posteriorFactor = (expanderFrequency - previousBandFrequency) / (bandFrequency - previousBandFrequency);

					if (bandIndex == i) 
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * bandAttenuation + posteriorFactor * previousBandAttenuation));
					}
					else
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * multibandExpander->GetAttenuationForOctaveBand(i) + posteriorFactor * multibandExpander->GetAttenuationForOctaveBand(i - 1)));
					}

					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(previousFactor * audiometry[bandIndex - 1] + posteriorFactor * audiometry[bandIndex]);
				}
				else if (expanderFrequency > bandFrequency && expanderFrequency < posteriorBandFrequency)
				{
					previousFactor = (posteriorBandFrequency - expanderFrequency) / (posteriorBandFrequency - bandFrequency);
					posteriorFactor = (expanderFrequency - bandFrequency) / (posteriorBandFrequency - bandFrequency);

					if (bandIndex == i) 
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * bandAttenuation + posteriorFactor * posteriorBandAttenuation));
					}
					else
					{
						threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(previousFactor * multibandExpander->GetAttenuationForOctaveBand(i) + posteriorFactor * multibandExpander->GetAttenuationForOctaveBand(i + 1)));
					}
						
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(previousFactor * audiometry[bandIndex] + posteriorFactor * audiometry[bandIndex + 1]);
				}
				else if (abs(expanderFrequency - bandFrequency) < 0.1f)
				{
					threshold_dBSPL = CalculateThresholdFromDBHL(CalculateDBHLFromAttenuation(bandAttenuation));
					threshold_dBFS = CalculateDBFSFromDBSPL(threshold_dBSPL);
					ratio = CalculateRatioFromDBHL(audiometry[bandIndex]);
				}

				multibandExpander->GetBandExpander(i, filterGrouping)->SetThreshold(threshold_dBFS);
				multibandExpander->GetBandExpander(i, filterGrouping)->SetRatio(ratio);

			}
		}
	}

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
		// Correct band index will be checked inside CMultibandExpander::GetOctaveBandFrequency
		return multibandExpanders.left->GetOctaveBandFrequency(bandIndex);
	}

	//////////////////////////////////////////////

	Common::CDynamicExpanderMono* CHearingLossSim::GetBandExpander(Common::T_ear ear, int bandIndex, bool filterGrouping)
	{
		ASSERT((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::RIGHT), RESULT_ERROR_CASENOTDEFINED, "Attempt to get the hearing loss expander of one band with a wrong ear specification", "");

		if (ear == Common::T_ear::LEFT)
			return multibandExpanders.left->GetBandExpander(bandIndex, filterGrouping);
		else
			return multibandExpanders.right->GetBandExpander(bandIndex, filterGrouping);
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetAttackForAllBands(Common::T_ear ear, float attack, bool filterGrouping)
	{
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			CMultibandExpander* multibandExpander = multibandExpanders.left.get();
			int numBands = multibandExpander->GetNumBands(filterGrouping);

			for (size_t i = 0; i < numBands; i++)
				multibandExpander->GetBandExpander(i, filterGrouping)->SetAttack(attack);
		}
		if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			CMultibandExpander* multibandExpander = multibandExpanders.right.get();
			int numBands = multibandExpander->GetNumBands(filterGrouping);

			for (size_t i = 0; i < numBands; i++)
				multibandExpander->GetBandExpander(i, filterGrouping)->SetAttack(attack);
		}
	}

	//////////////////////////////////////////////

	void CHearingLossSim::SetReleaseForAllBands(Common::T_ear ear, float release, bool filterGrouping)
	{
		if ((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::BOTH))
		{
			CMultibandExpander* multibandExpander = multibandExpanders.left.get();
			int numBands = multibandExpander->GetNumBands(filterGrouping);

			for (size_t i = 0; i < numBands; i++)
				multibandExpander->GetBandExpander(i, filterGrouping)->SetRelease(release);
		}
		if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			CMultibandExpander* multibandExpander = multibandExpanders.right.get();
			int numBands = multibandExpander->GetNumBands(filterGrouping);

			for (size_t i = 0; i < numBands; i++)
				multibandExpander->GetBandExpander(i, filterGrouping)->SetRelease(release);
		}
	}
	
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
			frequencySmearers.left->Process(asynchronyOutput.left, smearingOutput.left);			
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
			frequencySmearers.right->Process(asynchronyOutput.right, smearingOutput.right);
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

		if( enableMultibandExpander.left && enableHearingLossSimulation.left && multibandExpanders.left->IsReady() )
			multibandExpanders.left->Process(smearingOutput.left, audiogramOutput.left);
		else
			audiogramOutput.left = smearingOutput.left;
		if (enableMultibandExpander.right && enableHearingLossSimulation.right && multibandExpanders.right->IsReady())
			multibandExpanders.right->Process(smearingOutput.right, audiogramOutput.right);
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
			multibandExpanders.left->SetAttenuationForOctaveBand(bandIndex, attenuation);
		}
		else if ((ear == Common::T_ear::RIGHT) || (ear == Common::T_ear::BOTH))
		{
			multibandExpanders.right->SetAttenuationForOctaveBand(bandIndex, attenuation);
		}
	}

	//////////////////////////////////////////////

	float CHearingLossSim::GetAttenuationForBand(Common::T_ear ear, int bandIndex) 
	{
		if (ear == Common::T_ear::LEFT)
			return multibandExpanders.left->GetAttenuationForOctaveBand(bandIndex);
		if (ear == Common::T_ear::RIGHT)
			return multibandExpanders.right->GetAttenuationForOctaveBand(bandIndex);
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

	float CHearingLossSim::CalculateDBHLFromAttenuation(float attenuation)
	{
		return attenuation/(0.01f * A100);
	}

	//////////////////////////////////////////////

	CTemporalDistortionSimulator* CHearingLossSim::GetTemporalDistortionSimulator()
	{
		return &temporalDistortionSimulator;
	}

	//////////////////////////////////////////////

	shared_ptr<CFrequencySmearing> CHearingLossSim::GetFrequencySmearingSimulator(Common::T_ear ear)
	{		
		if (ear == Common::T_ear::LEFT)
			return frequencySmearers.left;
		if (ear == Common::T_ear::RIGHT)
			return frequencySmearers.right;
	
		SET_RESULT(RESULT_ERROR_CASENOTDEFINED, "Attempt to get frequency smearing simulator for both or none ears");
		return nullptr;
	}

	CMultibandExpander * CHearingLossSim::GetMultibandExpander(Common::T_ear ear)
	{
		return (ear == Common::T_ear::LEFT) ? multibandExpanders.left.get() : multibandExpanders.right.get();
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

	void CHearingLossSim::SetMultibandExpander(Common::T_ear ear, shared_ptr<CMultibandExpander> multibandExpander)
	{
		ASSERT(((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::RIGHT)), RESULT_ERROR_CASENOTDEFINED, "Cannot set the same frequency smearer for both ears", "");

		if (ear == Common::T_ear::LEFT)
			multibandExpanders.left = multibandExpander;
		if (ear == Common::T_ear::RIGHT)
			multibandExpanders.right = multibandExpander;
	}

	void CHearingLossSim::SetFrequencySmearer(Common::T_ear ear, shared_ptr<CFrequencySmearing> frequencySmearer)
	{
		ASSERT(((ear == Common::T_ear::LEFT) || (ear == Common::T_ear::RIGHT)), RESULT_ERROR_CASENOTDEFINED, "Cannot set the same frequency smearer for both ears", "");

		if (ear == Common::T_ear::LEFT)
			frequencySmearers.left = frequencySmearer;
		if (ear == Common::T_ear::RIGHT)
			frequencySmearers.right = frequencySmearer;
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
		if (ear == Common::T_ear::LEFT) {
			ASSERT(frequencySmearers.left.get(), RESULT_ERROR_NULLPOINTER, "Frequency smearing cannot be enabled for left ear because the frequency smearer has not been set", "");
			enableFrequencySmearing.left = true;
		}
		if (ear == Common::T_ear::RIGHT) {
			ASSERT(frequencySmearers.right.get(), RESULT_ERROR_NULLPOINTER, "Frequency smearing cannot be enabled for right ear because the frequency smearer has not been set", "");
			enableFrequencySmearing.right = true;
		}
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

