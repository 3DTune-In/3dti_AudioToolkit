/*
* Implementation of the Fig6 Algoritm
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

#ifndef _FIG6_ALG_H_
#define _FIG6_ALG_H_

namespace HAHLSimulation {

	/** \brief This function implements the Fig6 algorithm which allows to set up hearing aids from hearing loss data.
	*	\details It will be applied for sounds with 40 dB SPL.
	*	\param [in] hearingLoss_dB hearing loss in dBs    
	*	\retval gainDB gain for one band of the hearing aid, in dBHL
	*   \eh Nothing is reported to the error handler.
	*/
	extern float GetFig6AlgorithmGainFor40dB_SPL(float hearingLoss_dB);

	/** \brief This function implements the Fig6 algorithm which allows to set up hearing aids from hearing loss data.
	*	\details It will be applied for sounds with 65 dB SPL.
	*	\param [in] hearingLoss_dB hearing loss in dBs            
	*	\retval gainDB gain for one band of the hearing aid, in dBHL
	*   \eh Nothing is reported to the error handler.
	*/
	extern float GetFig6AlgorithmGainFor65dB_SPL(float hearingLoss_dB);

	/** \brief This function implements the Fig6 algorithm which allows to set up hearing aids from hearing loss data.
	*	\details It will be applied for sounds with 95 dB SPL.
	*	\param [in] hearingLoss_dB hearing loss in dBs            
	*	\retval gainDB gain for one band of the hearing aid, in dBHL
	*   \eh Nothing is reported to the error handler.
	*/
	extern float GetFig6AlgorithmGainFor95dB_SPL(float hearingLoss_dB);

}// end namespace HAHLSimulation

#endif
