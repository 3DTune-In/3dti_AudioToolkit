/**
*
* \brief Implementation of the Fig6 Algoritm
* \date	November 2016
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

#include <HAHLSimulation/Fig6Algorithm.h>
#include <cmath>

namespace HAHLSimulation {

	//---------------------------------------------------------------------------------------
	float GetFig6AlgorithmGainFor40dB_SPL(float hearingLoss_dB)
	{
		if (hearingLoss_dB < 20) return 0;
		if (hearingLoss_dB <= 60) return hearingLoss_dB - 20.0;
		return hearingLoss_dB * 0.5 + 10.0;
	}
	//---------------------------------------------------------------------------------------
	float GetFig6AlgorithmGainFor65dB_SPL(float hearingLoss_dB)
	{
		if (hearingLoss_dB < 20) return 0;
		if (hearingLoss_dB <= 60) return 0.6 * (hearingLoss_dB - 20.0);
		return hearingLoss_dB * 0.8 - 23.0;
	}
	//---------------------------------------------------------------------------------------
	float GetFig6AlgorithmGainFor95dB_SPL(float hearingLoss_dB)
	{
		return hearingLoss_dB <= 40 ? 0 : 0.1 * std::pow(hearingLoss_dB - 40.0, 1.4);
	}

}// end namespace HAHLSimulation