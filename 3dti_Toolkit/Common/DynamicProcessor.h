/**
* \class CDynamicProcessor
*
* \brief Declaration of CDynamicProcessor class interface.
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

#ifndef _CDYNAMIC_PROCESSOR_H_
#define _CDYNAMIC_PROCESSOR_H_

#include <Common/EnvelopeDetector.h>
#include <Common/Buffer.h>

namespace Common {

	/** \details This is an abstract class for applying dynamics processing (compression/expansion) to audio buffers
	*/
	class CDynamicProcessor
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*	\details Sets ratio to 1:1 and threshold to 0 dB
		*/
		CDynamicProcessor() : ratio(1.0f), threshold(0.0f), dynamicProcessApplied(false) {}

		/** \brief Setup the processor
		*	\param [in] samplingRate sampling rate, in Hz
		*   \param [in] ratio Compression/expansion ratio
		*	\param [in] threshold threshold level in dBfs
		*	\param [in] attack attack time, in milliseconds
		*	\param [in] release release time, in milliseconds
		*/
		virtual void Setup(int samplingRate, float ratio, float threshold, float attack, float release) = 0;

		/** \brief Set the compression/expansion ratio
		*   \param [in] _ratio Compression/expansion ratio
		*   \eh Nothing is reported to the error handler.
		*/
		void SetRatio(float _ratio) { ratio = _ratio; }

		/** \brief Set the threshold level
		*	\param [in] _threshold threshold level in dBfs
		*   \eh Nothing is reported to the error handler.
		*/
		void SetThreshold(float _threshold) { threshold = _threshold; }

		/** \brief Set the attack time
		*	\param [in] attack attack time, in milliseconds
		*/
		virtual void SetAttack(float attack) = 0;

		/** \brief Set the release time
		*	\param [in] release release time, in milliseconds
		*/
		virtual void SetRelease(float release) = 0;

		/** \brief Returns slope 
		*	\retval slope slope 
		*/
		virtual float GetSlope() = 0;

		/** \brief Returns the compression/expansion ratio  
		*	\retval ratio ratio
		*/
		float GetRatio() { return ratio; }

		/** \brief Returns the threshold level in dBfs  
		*	\retval threshold threshold, in dB
		*/
		float GetThreshold() { return threshold; }

		/** \brief Returns attack time, in milliseconds  
		*	\retval attack attack time, in ms
		*/
		virtual float GetAttack() = 0;

		/** \brief Returns release time, in milliseconds  
		*	\retval release release time, in ms
		*/
		virtual float GetRelease() = 0;

		/** \brief Returns true when compression/expansion was applied in the last call to Process  
		*	\retval isApplied true if compression/expansion was applied to the last processed buffer
		*   \eh Nothing is reported to the error handler.
		*/
		bool IsDynamicProcessApplied() { return dynamicProcessApplied; }

	protected:                                                    // PROTECTED ATTRIBUTES

		float ratio;                            // Compression Ratio 
		float threshold;                        // Threshold, in dBfs
		bool  dynamicProcessApplied;  // True when compression/expansion is applied in the last call to Process
	};
}//end namespace Common
#endif
