/**
* \class CFarDistanceEffects
*
* \brief Declaration of CFarDistanceEffects class interface.
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

#ifndef _CFAR_DISTACE_EFFECTS_H_
#define _CFAR_DISTACE_EFFECTS_H_

#include <Common/Buffer.h>
#include <Common/CommonDefinitions.h>
#include <Common/FiltersChain.h>

class CMagnitudes;

namespace Common {	

	/** \details This class applies low pas filter to the audio signal to model the effect of far distances from the listener to the sound source */
	class CFarDistanceEffects
	{
	public:
		////////////
		// METHODS
		////////////
		/** \brief Default constructor		
		*/
		CFarDistanceEffects();

		/** \brief Setup the distance attenuator
		*	\details Creates the low-pass filters for far-distance and setup sample rate for each filter
		*	\param [in] samplingRate sampling rate, in Hertzs
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate);

		/** \brief Get the cutoff frequency of the far-distance low-pass filter, for a given distance
		*	\param [in] distance distance, in meters
		*	\retval cutoff cutoff frequency, in Hertzs
		*   \eh Nothing is reported to the error handler.
		*/
		static float CalculateCutoffFrequency(float distance);

		/** \brief Process mono buffer to apply far-distance effect filter
		*	\details For use in binaural spatializer
		*	\param [in,out] inoutbuffer input and output mono buffer
		*	\param [in] distance distance of source, in meters
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & inoutbuffer, float distance);
		
	private:

		///////////////
		// ATTRIBUTES
		///////////////
		Common::CFiltersChain distanceFiltersChain;  // It will be used to model the effect of the distance in the anechoic process.
	};
}//end namespace Common

#endif
