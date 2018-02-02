/**
* \class CEnvelopeDetector
*
* \brief Declaration of CEnvelopeDetector class interface.
* \date	July 2016
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

#ifndef _CENVELOPMENT_DETECTOR_H_
#define _CENVELOPMENT_DETECTOR_H_

#include <Common/Buffer.h>

namespace Common {

	/** \details Class used to detect the envelope of an audio signal
	*/
	class CEnvelopeDetector
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*	\details By default, sets sampling rate to 44100Hz, attack time to 20ms and release time to 100ms
		*/
		CEnvelopeDetector();

		/** \brief Set the sample rate
		*	\param [in] samplingRate sample rate, in Hertzs
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate);

		/** \brief Set the attack time
		*	\param [in] attackTime_ms attack time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAttackTime(float attackTime_ms);

		/** \brief Returns the attack time in ms
		*	\retval attack attack time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAttackTime() { return m_fAttackTime_ms; }

		/** \brief Set the release time
		*	\param [in] releaseTime_ms release time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetReleaseTime(float releaseTime_ms);

		/** \brief Returns the release time in ms
		*	\retval release release time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetReleaseTime() { return m_fReleaseTime_ms; }

		/** \brief Returns the envelope sample for the input sample
		*	\param [in] input_sample input sample value
		*	\retval output_sample output sample value
		*   \eh Nothing is reported to the error handler.
		*/
		float ProcessSample(float input_sample);


	private:
		// PRIVATE ATTRIBUTES
		float envelope;					// Current envelop sample (the last sample returned by ProcessSample).	
		float samplingRate;             // Samplig Rate of the processed audio (samples per second).
		float m_fAttackTime;            // Value used in the difference equation to obtain the envelop
		float m_fReleaseTime;           // Value used in the difference equation to obtain the envelop 
		float m_fAttackTime_ms;         // Attack time in ms
		float m_fReleaseTime_ms;        // Release time in ms
	};
}//end namespace Common
#endif
