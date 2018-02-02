/**
* \class DynamicCompresorMono
*
* \brief Declaration of DynamicCompresorMono class interface.
* \details Base class to apply the effects of audio compression
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

#ifndef _CDYNAMIC_COMPRESSOR_MONO_H_
#define _CDYNAMIC_COMPRESSOR_MONO_H_

#include <Common/EnvelopeDetector.h>
#include <Common/DynamicProcessor.h>
#include <Common/Buffer.h>

namespace Common {

	/** \details This class applies dynamics compression to mono audio buffers
	*/
	class CDynamicCompressorMono : public CDynamicProcessor
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*	\details By default, sets threshold to 0dB and ratio to 1:1
		*   \eh Nothing is reported to the error handler.
		*/
		CDynamicCompressorMono();

		/** \brief Setup the compressor
		*	\param [in] samplingRate sampling rate, in Hz
		*   \param [in] ratio Compression ratio
		*	\param [in] threshold threshold level in dBfs
		*	\param [in] attack attack time, in milliseconds
		*	\param [in] release release time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate, float ratio, float threshold, float attack, float release);

		/** \brief Set the attack time
		*	\param [in] attack attack time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAttack(float attack);

		/** \brief Set the release time
		*	\param [in] release release time, in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetRelease(float release);

		/** \brief Returns attack time, in milliseconds  
		*	\retval attack attack time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAttack();

		/** \brief Returns release time, in milliseconds  
		*	\retval release release time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetRelease();

		/** \brief Returns slope 
		*	\retval slope slope
		*   \eh Nothing is reported to the error handler.
		*/
		float GetSlope();

		//// TODO: The buffer should be mono and the leftChannel parameter should dissapear.
		///**	Apply dynamics compression over an audio buffer
		//*	param [in] buffer input and output buffer
		//*	param [in] leftChannel channel to process (true=left; false=right)
		//*	exception May throw errors to debugger
		//*/
		//void Process(CStereoBuffer<float> &buffer, bool leftChannel);

		/**	\brief Apply dynamics compression over an audio buffer
		*	\param [in] buffer input and output buffer
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> &buffer);

	private:                                                         // PUBLIC ATTRIBUTES

		CEnvelopeDetector envDetector;			// Envelope detector used by the compressor
	};
}//end namespace Common
#endif
