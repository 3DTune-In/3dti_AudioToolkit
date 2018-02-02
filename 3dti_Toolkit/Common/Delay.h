/**
* \class CDelay
*
* \brief Declaration of CDelay class interface.
* \date	September 2017
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

#ifndef _CDELAY_H_
#define _CDELAY_H_

#include <Common/Buffer.h>
#include <Common/FiltersChain.h>
#include <Common/FarDistanceEffects.h>

namespace Common
{
	/** \details Class used to apply a delay to a buffer */
	class CDelay
	{	
	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*/
		CDelay();

		/** \brief Set the sample rate
		*	\param [in] delayInSamples number of samples that the processed signal will be delayed
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int delayInSamples);                              

		/** \brief Set the attack time
		*	\param [in] inBuffer input buffer
		*	\param [out] outBuffer output buffer (contains the delayed samples)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process( CMonoBuffer<float> &inBuffer, CMonoBuffer<float>& outBuffer );

		/** \brief Get the internal buffer with saved samples
		*	\retval buffer internal buffer with saved samples
		*   \eh Nothing is reported to the error handler.
		*/
		CMonoBuffer<float>* GetBuffer();

		/** \brief Fill the saved samples buffer with zeros
		*   \eh Nothing is reported to the error handler.
		*/
		void Reset();

	private:
		CMonoBuffer<float> savedSamples;
	};
}//end namespace Common
#endif
