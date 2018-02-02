/**
* \class CDistanceAttenuator
*
* \brief Declaration of CDistanceAttenuator class interface.
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

#ifndef _CDISTANCE_ATTENUATOR_H_
#define _CDISTANCE_ATTENUATOR_H_

#include <Common/Buffer.h>
#include <Common/FiltersChain.h>
#include <Common/FarDistanceEffects.h>

class CMagnitudes;

namespace Common
{
	/** \details This class applies distance effects (attenuation, far-distance low-pass filter and near-distance ILD) depending on the distance
	   from the listener to the sound source */
	class CDistanceAttenuator
	{
	public:                                                             // PUBLIC METHODS
		/** \brief Default constructor
		*	\details Setup reference distance (distance at which attenuation is 0 dB) to 1.95 meters
		*   \eh Nothing is reported to the error handler.
		*/
		CDistanceAttenuator();

		/** \brief Get the attenuation gain due to distance for a given distance attenuation constant
		*	\param [in] attForDuplicateDistance distance attenuation constant, in decibels
		*	\param [in] distance distance, in meters
		*	\param [in] extraAttennuation_dB fixed attenuation (non distance-dependent) to be added, in decibels
		*	\retval attenuation attenuation gain
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetDistanceAttenuation(float attForDuplicateDistance, float distance, float extraAttennuation_dB = 0.0f) const;

		/** \brief Get the reference distance (distance at which attenuation is 0 dB) in meters
		*	\retval distance reference distance, in meters
		*   \eh Nothing is reported to the error handler.
		*/
		float GetReferenceDistance();

		/** \brief Set the reference distance (distance at which attenuation is 0 dB) in meters
		*	\param [in] _referenceDistance reference distance value, in meters
		*   \eh Nothing is reported to the error handler.
		*/
		void SetReferenceDistance(float _referenceDistance);

		/** \brief Process mono input buffer to apply all distance effects (attenuation, far-distance filter and near-distance ILD)
		*	\details For use in loudspeakers spatializer
		*	\param [in,out] buffer input and output buffer
		*	\param [in] distance distance to source, in meters
		*	\param [in] attenuationConstant distance attenuation constant, in decibels
		*	\param [in] bufferSize buffer size, as number of samples
		*	\param [in] sampleRate sample rate, in Hz
		*	\param [in] extraAttennuation_dB fixed attenuation (non distance-dependent) to be added, in decibels (defaults to 0)
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & buffer, float distance,float attenuationConstant, int bufferSize, int sampleRate, float extraAttennuation_dB = 0.0f);
		
	private:

		//  ATTRIBUTES
		float referenceDistance;              // Distance at which the attenuation is 0 dB, in meters.
		float previousAttenuation_Channel;
	};
}//end namespace Common

#endif
