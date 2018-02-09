/**
* \class CTemporalDistortionSimulator
*
* \brief Declaration of CTemporalDistortionSimulator class interface. 
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

#ifndef _CTEMPORAL_DISTORTION_SIMULATOR_H
#define _CTEMPORAL_DISTORTION_SIMULATOR_H

#include <HAHLSimulation/HighOrderButterworthFilter.h>
#include <Common/CommonDefinitions.h>
#include <Common/NoiseGenerator.h>
#include <Common/Delay.h>

// Default values
#define DEFAULT_NOISE_AUTOCORRELATION_CUTOFF 500
#define DEFAULT_NOISE_AUTOCORRELATION_Q 0.707

namespace HAHLSimulation {

	/** \details Class to simulate temporal Distortion effect of hearing loss.
	* Temporal distortion is simulated with a time jitter process in low frequencies, modeling a potential symptom of
	* age-related sensorineural hearing loss, where cortical speech processing may be limited by age-related decreases
	* in the precision of neural synchronization in the midbrain.
	*/
	class CTemporalDistortionSimulator
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Setup the temporal Distortion simulator		
		*	\param [in] samplingRate sampling rate in Hzs
		*	\param [in] bufferSize size of buffers to be processed, expressed as number of (mono) samples
		*	\param [in] bandUpperLimit frequency, in Hzs, to split between low (where jitter occurs) and high frequencies
		*	\param [in] noisePower jitter white noise power, in ms
		*	\param [in] leftRightSynchronicity amount of synchronicity between the two ears (0.0 means non-correlated/non-synched independent jitter for each ear, 1.0 means the same jitter for both ears)
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate, int bufferSize, int bandUpperLimit, float noisePower, float leftRightSynchronicity);

		/** \brief Process an input buffer
		*	\details Jitter is added to the low frequency content of the buffer. Both ears are processed into separate buffers
		*	\param [in] inputBuffer input buffer pair		
		*	\param [out] outputBuffer output buffer pair		
		*   \eh On error, an error code is reported to the error handler.
		*/		
		void Process(Common::CEarPair<CMonoBuffer<float>> & inputBuffer, Common::CEarPair<CMonoBuffer<float>> & outputBuffer);

		/** \brief Set the amount of left-right noise synchronicity
		*	\details This parameter is the correlation between the noise sources for both ears.
		*	A value of 0.0 means non-correlated/non-synched independent jitter for each ear and 1.0 means the same jitter for both ears.
		*	\param [in] leftRightSynchronicity amount of synchronicity between the two ears
		*   \eh Nothing is reported to the error handler.
		*/
		void SetLeftRightNoiseSynchronicity(float leftRightSynchronicity);

		/** \brief Set the power of jitter noise source before autocorrelation
		*	\details This parameter is the amount of time jitter, in milliseconds
		*	\param [in] ear for which ear we want to set noise power (if LeftRightNoiseSynchronicity is enabled, this parameter is ignored and the same value will be set for both ears)
		*	\param [in] noisePower jitter noise power, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		void SetWhiteNoisePower(Common::T_ear ear, float noisePower);

		/** \brief Set the cutoff frequency of jitter noise source autocorrelation filter
		*	\details This parameter affects the total power of jitter noise source after autocorrelation
		*	\param [in] ear for which ear we want to set the cutoff frequency (if LeftRightNoiseSynchronicity is enabled, this parameter is ignored and the same value will be set for both ears)
		*	\param [in] cutoffFrequency cutoff frequency, in Hzs
		*   \eh Nothing is reported to the error handler.
		*/
		void SetNoiseAutocorrelationFilterCutoffFrequency(Common::T_ear ear, float cutoffFrequency);

		/** \brief Set the upper limit for the (low frequency) band where jitter occurs
		*	\details Jitter will be processed only for frequencies below upper limit
		*	\param [in] ear for which ear we want to set the upper limit (if LeftRightNoiseSynchronicity is enabled, this parameter is ignored and the same value will be set for both ears)
		*	\param [in] upperLimit upper limit frequency, in Hzs
		*   \eh Nothing is reported to the error handler.
		*/
		void SetBandUpperLimit(Common::T_ear ear, int upperLimit);

		/** \brief Get autocorrelation coefficient zero for jitter noise source
		*	\param [in] ear for which ear we want to get the coefficient
		*	\retval power autocorrelation coefficient zero (power) for jitter noise source of that ear
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetPower(Common::T_ear ear);

		/** \brief Get normalized autocorrelation coefficient for jitter noise source
		*	\details The autocorrelation coefficient is calculated depending on the time set with method SetAutocorrelationTimeShift_ms		
		*	\param [in] ear for which ear we want to get the coefficient
		*	\retval autocorrelation normalized autocorrelation coefficient for jitter noise source of that ear
		*	\sa SetAutocorrelationTimeShift_ms
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetNormalizedAutocorrelation(Common::T_ear ear);

		/** \brief Enable the possibility of changing the synchronicity between the two ears
		*	\details If synchronicity is enabled, the value of synchronicity (set with method SetLeftRightNoiseSynchronicity) determines the amount of synchronicity
		*	between the two ears. A synchronicity of 1.0 means that the same effect is processed for both ears.
		*	This method will internally call to SetBandUpperLimit, SetNoiseAutocorrelationFilterCutoff and SetWhiteNoisePower to copy the values of these attributes
		*	from left to right ears.
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableLeftRightNoiseSynchronicity();

		/** \brief Disable the possibility of changing the synchronicity between the two ears
		*	\details If synchronicity is disabled, the temporal Distortion effect is processed independently for each ear (same as having 0.0f left-right synchronicity)
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableLeftRightNoiseSynchronicity();

		/** \brief Set shift in ms to calculate the autocorrelation coefficient
		*	\param [in] ms time shift to calculate the autocorrelation
		*	\sa GetNormalizedAutocorrelation
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAutocorrelationTimeShift_ms( float ms ){ autocorrelationTimeShift_ms = ms;  }

		/** \brief Returns the shift in ms that is used to calculate the autocorrelation coefficient
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAutocorrelationTimeShift_ms() { return autocorrelationTimeShift_ms; }

		/** \brief Enable temporal Distortion simulator for one or both ears
		*	\details When simulator is enabled only for one ear, the delay is compensated for both ears
		*	\param [in] ear for which ear we want enable the temporal Distortion simulator
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableTemporalDistortionSimulator(Common::T_ear ear);

		/** \brief Disable temporal Distortion simulator for one or both ears
		*	\details When simulator is enabled only for one ear, the delay is compensated for both ears
		*	\param [in] ear for which ear we want disable the temporal Distortion simulator
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableTemporalDistortionSimulator(Common::T_ear ear);


	private:	// PRIVATE METHODS

		// Internal time jitter process
		void ProcessJitter(Common::CDelay& delayBuffer, CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & noiseSource, CMonoBuffer<float> & outputBuffer);

		// Transform a value in number of samples into a time in miliseconds
		float GetTimeInMilisecondsFromNumberOfSamples( float numberOfSamples );

		// Transform a time in milisecondsinto number of samples 
		float GetNumberOfSamplesFromTimeInMiliseconds(float time_ms);

	private:	// PRIVATE ATTRIBUTES

		// Internal storage
		Common::CEarPair<Common::CNoiseGenerator> noiseGenerators;	// Noise generators for jitter source, for each ear
		Common::CEarPair<Common::CDelay> jitterDelayBuffers;		// Delay buffers for jitter process (in low frequencies), for each ear
		Common::CEarPair<Common::CDelay> highFrequencyDelayBuffers;	// Delay buffers for high frequencies, for each ear				
		Common::CEarPair<CHighOrderButterworthFilter> preLPFFilter;	// Low-pass filter before Jitter process, to get the band over which jitter will be processed
		Common::CEarPair<CHighOrderButterworthFilter> preHPFFilter;	// High-pass filter before Jitter process, to get the band that won't be affected by jitter
		Common::CEarPair<CHighOrderButterworthFilter> postLPFFilter;	// Low-pass filter after Jitter process, to recompose the band over which jitter has been processed
		Common::CEarPair<CHighOrderButterworthFilter> postHPFFilter;	// High-pass filter after Jitter process, to recompose the band over which jitter was not processed

		// Single-ear Bypass
		Common::CEarPair<Common::CDelay> bypassLowDelays;				// Delay buffers for low frequencies to get same delay in both ears when one ear has temporal Distortion bypassed
		Common::CEarPair<Common::CDelay> bypassHighDelays;				// Delay buffers for high frequencies to get same delay in both ears when one ear has temporal Distortion bypassed
		Common::CEarPair<CHighOrderButterworthFilter> bypassPreLPFFilter;	// Copy of the preLPFFilter of the opposite ear
		Common::CEarPair<CHighOrderButterworthFilter> bypassPreHPFFilter;	// Copy of the preHPFFilter of the opposite ear
		Common::CEarPair<CHighOrderButterworthFilter> bypassPostLPFFilter;	// Copy of the postLPFFilter of the opposite ear
		Common::CEarPair<CHighOrderButterworthFilter> bypassPostHPFFilter;	// Copy of the postHPFFilter of the opposite ear

		// Attributes to be set only once
		int processBufferSize;								// Size of (mono) buffers to be processed
		int sampleRate;										// Sample rate, in Hzs
		int jitterDelayBufferSize;							// Size of jitter delay buffers
		int maxSampleOffset;								// Maximum sample offset applied by time jitter process

		// Attributes you can change while processing
		bool doLeftRightNoiseSynchronicity;						// Switch for enabling synchronicity between the two ears
		float leftRightNoiseSynchronicity;						// Amount of synchronicity (jitter correlation) between the two ears	
		Common::CEarPair<bool> doTemporalDistortionSimulator;	// Switch for enabling global temporal Distortion simulator, for each ear

		// Attributes you can get from internal processes
		Common::CEarPair<float> power;		                // Autocorrelation coefficient zero of jitter noise sources, for each ear
		Common::CEarPair<float> autocorrelation;	        // Autocorrelation coefficient one of jitter noise sources, for each ear

		// Internal attributes for handling ear synchronicity
		float bandUpperLimitLastLeftValue;								// Last value given to band upper limit for left ear
		float whiteNoisePowerLastLeftValue;								// Last value given to white noise power for left ear
		float noiseAutocorrelationFilterCutoffFrequencyLastLeftValue;	// Last value given to noise autocorrelation filter cutoff frequency for left ear
		float autocorrelationTimeShift_ms;								// Time in milliseconds to calculate the autocorrelation
	};
}// end namespace HAHLSimulation
#endif