/**
* \class CHearingAidSim
*
* \brief Declaration of the CHearingAidSim class interface.
* \date	February 2016
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

#ifndef _CHEARING_AID_SIM_H_
#define _CHEARING_AID_SIM_H_

#include <HAHLSimulation/DynamicEqualizer.h>
#include <Common/FiltersBank.h>
#include <Common/BiquadFilter.h>
#include <Common/Buffer.h>
#include <Common/FiltersChain.h>
#include <Common/CommonDefinitions.h>
#include <vector>
#include <memory>

namespace HAHLSimulation {

	/** \details This class implements hearing aid simulation
	*/
	class CHearingAidSim
	{
	public:

		CHearingAidSim();

		/** \brief Setup the hearing aid simulator equalizer and filters
		*	\details In addition to the equalizer, a biquad low pass filter and a biquad high pass filter are used to model the HA.
		*	\n This method determines the number of bands of the equalizer as well as the center frequencies.
		*	It also configures the internal filters.
		*	\param [in] samplingRate sampleRate in Hertzs
		*	\param [in] numLevels The equalizer is actualy composed by numLevels equalizers and the result is
		*                          obtained from them depending on the level of the signal
		*	\param [in] iniFreq_Hz initial frequency, in Hertzs
		*	\param [in] bandsNumber number of frequency bands
		*	\param [in] octaveBandStepfloat especifies the frequency step to determine the bandwidth of each band.
		*	\param [in] lpf_CutoffFreqHz cutoff frequency of the low-pass filter
		*	\param [in] hpf_CutoffFreqHz cutoff frequency of the how-pass filter
		*	\param [in] Q_LPF Q-factor of the low-pass filter
		*	\param [in] Q_BPF Q-factor of the band-pass filters
		*	\param [in] Q_HPF Q-factor of the high-pass filter
		*	\n for instance, if octaveBandStep is equal to 1, one octave will be used:
		*	setup(  125, 7, 1 ); will produce center frequencies of: 125, 250, 500, 1000, 2000, 4000, 8000
		*	\n If octaveBandStep is equal to 3, a third of octave will be use instead:
		*	setup(  125, 7, 3 ); will produce center frequencies of: 125, 160, 200, 250, 315, 400, 500
		*	\pre iniFreq_Hz must be any of the 31 center frequency values for the 31 ISO bands:
		*	20, 25, 31, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000
		*	2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup( int samplingRate, int numLevels, float iniFreq_Hz, int bandsNumber, int octaveBandStepfloat,
			        float lpf_CutoffFreqHz, float hpf_CutoffFreqHz, float Q_LPF, float Q_BPF, float Q_HPF);

		/** \brief Set the gain for all bands of the equalizer for one ear
		*	\param [in] ear for which ear we want to set gains
		*	\param [in] gains_dB vector with the gains for each band, in decibels		
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAllBandGains_dB(Common::T_ear ear, vector<float> gains_dB);

		/** \brief Reset all parameters of HA of one ear to its default state
		*	\param [in] ear for which ear we want to do the reset
		*   \eh Nothing is reported to the error handler.
		*/
		void Reset(Common::T_ear ear);

		/** \brief Set the gain for a specific band of a specific level of the equalizer
		*	\param [in] ear for which ear we want to set band gain
		*	\param [in] levelIndex index of the level
		*	\param [in] bandIndex ID of the band
		*	\param [in] gain_dB gain, in decibels		
		*   \eh Nothing is reported to the error handler.
		*/
		void SetDynamicEqualizerBandGain_dB(Common::T_ear ear, int levelIndex, int bandIndex, float gain_dB);

		/** \brief Set the threshold signal level to apply the curves of the dynamic equalizer
		*	\param [in] ear for which ear we want to set level threshold
		*	\param [in] levelIndex index of the level
		*	\param [in] threshold_dBfs signal level threshold value in dBfs		
		*   \eh Nothing is reported to the error handler.
		*/
		void SetDynamicEqualizerLevelThreshold(Common::T_ear ear, int levelIndex, float threshold_dBfs);

		/** \brief Process an input buffer through the hearing aid simulator
		*	\param [in] inputBuffer input buffer pair for both channels
		*	\param [out] outputBuffer output buffer pair for both channels
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(Common::CEarPair <CMonoBuffer<float>> &inputBuffer, Common::CEarPair <CMonoBuffer<float>> &outputBuffer);

		/** \brief Configuration of the low-pass filter
		*	\param [in] cutoffFreq_hz cutoff frequency, in Hertzs
		*	\param [in] Q Q-factor
		*   \eh Nothing is reported to the error handler.
		*/
		void SetLowPassFilter(float cutoffFreq_hz, float Q);

		/** \brief Configuration of the high-pass filter
		*	\param [in] cutoffFreq_hz cutoff frequency, in Hertzs
		*	\param [in] Q Q-factor
		*   \eh Nothing is reported to the error handler.
		*/
		void SetHighPassFilter(float cutoffFreq_hz, float Q);

		/** \brief Applies the Fig6 algorithm to callibrate the left or the right channel of the HA.
		*   \param [in] ear for which ear we want to set dynamic equalizer
		*	\param [in] earLoss contains the hearing loss (dB) for each band in the equalizer. Loss dB values
		*               must be >= 0 dB.
		*	\param [in] dBs_SPL_for_0_dBs_fs is intended to establish the relation between dBs SPL (sound pressure
		*               level) and dBs fs (full scale). When this relation has't been meassured using a sound
		*               meter, 0 must be specified as the value of this param.		
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetDynamicEqualizerUsingFig6(Common::T_ear ear, vector <float> &earLoss, float dBs_SPL_for_0_dBs_fs);

		/** \brief Set the reference value to calculate the offset to be applied to all the gains in
		*          the curves so the maximum gain value in the level 0
		*          will be referenceValue_dB
		*	\param [in] ear ear for which the normalization will be applied
		*	\param [in] referenceValue_dB reference value in dBs to apply normalization
		*   \eh Nothing is reported to the error handler.
		*/
		void SetNormalizationLevel(Common::T_ear ear, float referenceValue_dB);

		/** \brief Enables the normalization
		*	\param [in] ear for which the normalization will be enabled/disabled
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableNormalization(Common::T_ear ear);

		/** \brief Enables the normalization
		*	\param [in] ear for which the normalization will be enabled/disabled
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableNormalization( Common::T_ear ear );

		/** \brief Returns a pointer to the CDynamicEqualizer object for one channel
		*	\param [in] ear for which ear we want to get the dynamic equalizer
		*	\retval dynEQ pointer to the CDynamicEqualizer object for that ear 
		*   \eh On error, an error code is reported to the error handler.
		*/
		CDynamicEqualizer *GetDynamicEqualizer(Common::T_ear ear); 

		/** \brief Set overall gain for one or both ears
		*	\param [in] ear for which we want to set overall gain
		*	\param [in] gain gain factor to set (not dBs)
		*   \eh Nothing is reported to the error handler.
		*/
		void SetOverallGain(Common::T_ear ear, float gain);

		/** \brief Enable quantisation before equalizer		
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableQuantizationBeforeEqualizer();

		/** \brief Disable quantisation before equalizer
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableQuantizationBeforeEqualizer();

		/** \brief Enable quantisation after equalizer
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableQuantizationAfterEqualizer();

		/** \brief Disable quantisation after equalizer
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableQuantizationAfterEqualizer();

		/** \brief Set number of bits for processing quantization 		
		*	\param [in] nBits number of bits for quantization 
		*   \eh Nothing is reported to the error handler.
		*/
		void SetQuantizationBits(int nBits);

		/** \brief Enable hearing aid simulation (global switch) for one or both ears
		*	\param [in] ear for which we want to enable hearing aid simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableHearingAidSimulation(Common::T_ear ear);

		/** \brief Disable hearing aid simulation (global switch) for one or both ears
		*	\param [in] ear for which we want to disable hearing aid simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableHearingAidSimulation(Common::T_ear ear);


	private:                                                          // PRIVATE METHODS

		// Add quantization noise to buffer 
		void ProcessQuantizationNoise(CMonoBuffer<float> &buffer);

		// Normalizes the curves applying the reference value
		void ProcessNormalization(Common::T_ear ear, float referenceValue_dB);

		// Reset the normalization
		void ResetNormalization(Common::T_ear ear);

		// Enable normalization
		void SetEnableNormalization(Common::T_ear ear, bool _enabled);


		// ATTRIBUTES

		Common::CEarPair<float> overallGain;							// Volume for each ear channel (gain, not dBs)		
		Common::CEarPair<bool> enableHearingAidSimulation;				// Global switch for whole hearing aid simulation process, for each ear

		bool enableQuantizationBeforeEqualizer;							// When true, quantization is computed at the begining of the process 
		bool enableQuantizationAfterEqualizer;							// When true, quantization is computed at the end of the process
		int  quantizationBits;											// Number of bits to compute quantization 
																		   
		Common::CEarPair <Common::CFiltersChain> lowPassFilter;			// Low pass filter
		Common::CEarPair <Common::CFiltersChain> highPassFilter;		// High pass filter
																		   
		Common::CEarPair<float> normalizationReference;					// Reference gain for normalization
		Common::CEarPair<bool> normalizationEnabled;					// Switch on/off normalization
																		   
		Common::CEarPair<CDynamicEqualizer> dynamicEqualizer;			// Dynamic equalizers for both ears used to compensate the hearing loss		
	};
}// end namespace HAHLSimulation
#endif
