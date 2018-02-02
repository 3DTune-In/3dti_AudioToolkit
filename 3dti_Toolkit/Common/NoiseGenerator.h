/**
* \class CNoiseGenerator
*
* \brief Declaration of CNoiseGenerator class interface. 
*
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

#ifndef _CNOISE_GENERATOR_H_
#define _CNOISE_GENERATOR_H_

#include <Common/Buffer.h>
#include <Common/BiquadFilter.h>
#include <random>

#define DEFAULT_GAUSSIAN_MEAN 0

namespace Common {

	/** \details This class implements a gaussian noise generator. White noise can be generated, 
	* but also autocorrelated noise with a configurable low pass filter. 
	*/
	class CNoiseGenerator
	{
	public:					// PUBLIC METHODS

		/** \brief Setup the noise generator
		*	\details By default, creates a gaussian white (non-correlated) noise with mean in 0
		*	\param [in] deviation standard deviation of gaussian distribution
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(float deviation);

		/** \brief Set standard deviation of gaussian distribution
		*	\param [in] deviation New standard deviation value
		*   \eh Nothing is reported to the error handler.
		*/
		void SetDeviation(float deviation);

		/** \brief Enable autocorrelation filter
		*	\details By default, noise is white (non-correlated). This enables a low-pass-filter for auto-correlation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableAutocorrelationFilter();

		/** \brief Disable autocorrelation filter
		*	\details Disables the auto-correlation low-pass-filter
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableAutocorrelationFilter();

		/** \brief Setup parameters of autocorrelation filter
		*	\details Setup cutoff and q-factor of low-pass autocorrelation filter
		*	\param [in] samplingRate Sampling rate, in Hzs
		*	\param [in] cutoff Cutoff frequency of low-pass filter, in Hzs
		*	\param [in] q Q-factor of low-pass filter
		*	\pre If autocorrelation filter is not enabled, this has no effect
		*   \eh Nothing is reported to the error handler.
		*/
		void SetupAutocorrelationFilter(float samplingRate, float cutoff, float q);

		/** \brief Set cutoff frequency of low-pass autocorrelation filter
		*	\param [in] cutoff Cutoff frequency in Hzs
		*	\pre If autocorrelation filter is not enabled, this has no effect
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAutocorrelationFilterCutoff(float cutoff);

		/** \brief Process and generate an output buffer
		*	\details Generate an output buffer full of noise samples
		*	\param [out] outputBuffer Output buffer with noise samples
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & outputBuffer);

	private:						// PRIVATE ATTRIBUTES
		// Random generator with gaussian distribution
		std::default_random_engine randomEngine;			// Random number generator
		std::normal_distribution<float> normalDistribution;	// Normal distribution post-processor
		float standardDeviation;	// Standard deviation of gaussian distribution
		
		// Autocorrelation filter (optional)
		bool doAutocorrelation;		// Autocorrelation low-pass filter is enabled or not
		Common::CBiquadFilter autocorrelationFilter;	// Autocorrelation low-pass filter
		float autocorrelationQ;		// Q factor of autocorrelation filter
	};
}//end namespace Common
#endif


