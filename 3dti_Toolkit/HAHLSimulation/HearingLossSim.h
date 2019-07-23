/**
* \class CHearingLossSim
*
* \brief Declaration of CHearingLossSim class interface.
*
* Class to implement a a hearing loss simulator
* \date	July 2017
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: David Poirier-Quinot 
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



#ifndef _CHEARING_LOSS_SIM_H_
#define _CHEARING_LOSS_SIM_H_

#include <Common/Buffer.h>
#include <Common/CommonDefinitions.h>
#include <vector>
#include <memory>
#include <HAHLSimulation/MultibandExpander.h>
#include <HAHLSimulation/TemporalDistortionSimulator.h>
#include <HAHLSimulation/Graf3DTIFrequencySmearing.h>
#include <HAHLSimulation/BaerMooreFrequencySmearing.h>

//#define DB_HL_TO_SPL_OFFSET 80.0f
//#define DB_HL_TO_SPL_FACTOR	9.0f
//#define DB_HL_TO_RATIO_OFFSET 720.0f
//#define DB_HL_TO_RATIO_FACTOR 8.0f
#define AVERAGE_ATTENUATION_FOR_100DB 20.0f
#define AVERAGE_THRESHOLD_FOR_100DB 100.0f

#define DEFAULT_TEMPORAL_DISTORTION_SPLIT_FREQUENCY 1600.0f
#define DEFAULT_TEMPORAL_DISTORTION_AMOUNT_IN_MS 0.0f
#define DEFAULT_TEMPORAL_DISTORTION_LEFTRIGHT_SYNCHRONICITY 0.0

// For convenience...
#define T100 AVERAGE_THRESHOLD_FOR_100DB
#define A100 AVERAGE_ATTENUATION_FOR_100DB

/** \brief Type definition for audiometry of one ear
*/
typedef std::vector<float> TAudiometry;

namespace HAHLSimulation {

	/** \details This class implements hearing loss simulation
	*/
	class CHearingLossSim
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Setup the hearing loss simulator equalizer
		*	\details Specifies the bands (number, initial frequency and number of internal filters per band, to increase bandwidth).
		*	\param [in] samplingRate sampleRate in Hertzs
		*	\param [in] Calibration_dBs_SPL_for_0_dBs_fs equivalence between 0 dBFS and X dBSPL, coming from external calibration
		*	\param [in] iniFreq_Hz initial frequency, in Hertzs
		*	\param [in] bandsNumber number of frequency bands
		*	\param [in] filtersPerBand specifies the number of filters per band
		*	\param [in] _filterBank specifies which type of filterbank to use: butterworth or gammatone
		*	\param [in] filterGrouping specifies whether you want the multiband expander to act on single filters or grouped filters
		*	\param [in] bufferSize size of buffers to be processed
		*	\param [in] smearingAlgorithm algorithm used in frequency smearing processing
		*	\pre parameter filtersPerBand must be an odd number.
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate, float Calibration_dBs_SPL_for_0_dBs_fs, int bandsNumber, int bufferSize);

		/** \brief Set the hearing loss simulator calibration
		*	\details Specifies the equivalence between 0 dBFS and X dBSPL, coming from external calibration
		*	\param [in] Calibration_dBs_SPL_for_0_dBs_fs equivalence between 0 dBFS and X dBSPL, coming from eventual calibration
		*   \eh Nothing is reported to the error handler.
		*/
		void SetCalibration(float Calibration_dBs_SPL_for_0_dBs_fs);

		/** \brief Set a full audiometry for one ear, i.e. hearing levels for all bands of the audiometry
		*	\param [in] ear for which ear we want to set the audiometry
		*	\param [in] hearingLevels_dBHL vector with the hearing levels for each band, in dBHL
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetFromAudiometry_dBHL(Common::T_ear ear, TAudiometry hearingLevels_dBHL);

		/** \brief Set one hearing level for one band of the audiometry for one ear
		*	\param [in] ear for which ear we want to set the level
		*	\param [in] bandIndex ID of the audiometry band
		*	\param [in] hearingLevel_dBHL hearing level, in dBHL
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetHearingLevel_dBHL(Common::T_ear ear, int bandIndex, float hearingLevel_dBHL);

		/** \brief Get one hearing level for one band of the audiometry of one ear
		*	\param [in] ear for which ear we want to get the level
		*	\param [in] bandIndex ID of the audiometry band
		*	\retval level hearing level, in dBHL
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetHearingLevel_dBHL(Common::T_ear ear, int bandIndex);

		/** \brief Get number of bands configured for the audiometry
		*	\retval n number of bands in the audiometry of each ear
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumberOfBands();

		/** \brief returns the frequency in Hertzs of the audiometry band whose index is passed
		*	\param [in] bandIndex index of the band whose frequency is required
		*	\retval frequency frequency, in Hz
		*   \eh Nothing is reported to the error handler.
		*/
		float GetBandFrequency(int bandIndex);

		/** \brief Process an input buffer through the hearing loss simulator
		*	\param [in] inputBuffer input buffer pair
		*	\param [out] outputBuffer output buffer pair
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(Common::CEarPair<CMonoBuffer<float>> &inputBuffer, Common::CEarPair<CMonoBuffer<float>> &outputBuffer);

		/** \brief Calculate equivalence from dB SPL to dB FS with the configured calibration
		*	\param [in] dBSPL value in dB SPL
		*	\retval dBfs value in dB FS
		*   \eh Nothing is reported to the error handler.
		*/
		float CalculateDBFSFromDBSPL(float dBSPL);

		/** \brief Calculate equivalence from dB FS to dB SPL with the configured calibration
		*	\param [in] dBFS value in dB FS
		*	\retval dBSPL value in dB SPL
		*   \eh Nothing is reported to the error handler.
		*/
		float CalculateDBSPLFromDBFS(float dBFS);

		/** \brief Get access to the dynamics expander of one band for one ear
		*	\param [in] ear which ear to get the expander from
		*	\param [in] bandIndex index of the band for which we want to get the expander
		*	\retval expander Pointer to the dynamics expander of specified band and ear
		*   \eh On error, an error code is reported to the error handler.
		*/
		Common::CDynamicExpanderMono* GetBandExpander(Common::T_ear ear, int bandIndex, bool filterGrouping);

		/** \brief Set attack time, in ms, for the dynamics expanders of all bands in one ear
		*	\param [in] ear for which ear we want to set the attack
		*	\param [in] attack attack time in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAttackForAllBands(Common::T_ear ear, float attack, bool filterGrouping);

		/** \brief Set release time, in ms, for the dynamics expanders of all bands in one ear
		*	\param [in] ear for which ear we want to set the release
		*	\param [in] release release time in milliseconds
		*   \eh Nothing is reported to the error handler.
		*/
		void SetReleaseForAllBands(Common::T_ear ear, float release, bool filterGrouping);

		/** \brief Set attenuation for one band in one ear.
		*	\param[in] ear ear for which we want to set attenuation
		*	\param[in] bandIndex index of the band for which attenuation will be set
		*	\param[in] attenuation attenuation value in (positive) decibels
		*   \eh Nothing is reported to the error handler.*/
		void SetAttenuationForBand(Common::T_ear ear, int bandIndex, float attenuation);

		/** \brief Get attenuation applied in one band for one ear.
		*	\param[in] ear ear for which we want to get attenuation
		*	\param[in] bandIndex index of the band
		*	\retval attenuation attenuation in (positive) decibels
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		float GetAttenuationForBand(Common::T_ear ear, int bandIndex);

		/** \brief Get access to the temporal Distortion simulator
		*	\retval temporalDistortionSimulator Pointer to the temporal Distortion simulator
		*   \eh Nothing is reported to the error handler.
		*/
		CTemporalDistortionSimulator* GetTemporalDistortionSimulator();

		/** \brief Get access to the frequency smearing simulator of one ear
		*	\param [in] ear for which ear we want to get its frequency smearing simulator
		*	\retval frequencySmearingSimulator Pointer to the frequency smearing simulator
		*   \eh On error, an error code is reported to the error handler.
		*/
		shared_ptr<CFrequencySmearing> GetFrequencySmearingSimulator(Common::T_ear ear);

		CMultibandExpander* GetMultibandExpander(Common::T_ear ear);

		/** \brief Enable hearing loss simulation (global switch for all internal processes)
		*	\param[in] ear for which ear we want to enable hearing loss simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableHearingLossSimulation(Common::T_ear ear);

		/** \brief Disable hearing loss simulation (global switch for all internal processes)
		*	\param[in] ear for which ear we want to disable hearing loss simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableHearingLossSimulation(Common::T_ear ear);

		/** \brief Enable multiband expander (audiogram) for one (or both) ear
		*	\param[in] ear for which ear we want to enable multiband expander
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableMultibandExpander(Common::T_ear ear);

		/** \brief Disable multiband expander (audiogram) for one (or both) ear
		*	\param[in] ear for which ear we want to disable multiband expander
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableMultibandExpander(Common::T_ear ear);

		/** \brief Enable temporal Distortion simulation for one (or both) ear
		*	\param[in] ear for which ear we want to enable temporal Distortion simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableTemporalDistortion(Common::T_ear ear);

		/** \brief Disable temporal Distortion simulation for one (or both) ear
		*	\param[in] ear for which ear we want to disable temporal Distortion simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableTemporalDistortion(Common::T_ear ear);

		void SetMultibandExpander(Common::T_ear ear, shared_ptr<CMultibandExpander> multibandExpander);

		void SetFrequencySmearer(Common::T_ear ear, shared_ptr<CFrequencySmearing> frequencySmearer);

		/** \brief Enable frequency smearing simulation for one (or both) ear
		*	\param[in] ear for which ear we want to enable frequency smearing simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableFrequencySmearing(Common::T_ear ear);

		/** \brief Disable frequency smearing simulation for one (or both) ear
		*	\param[in] ear for which ear we want to disable frequency smearing simulation
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableFrequencySmearing(Common::T_ear ear);

		//Calculate band attenuation corresponding to one hearing level in dBHL
		float CalculateAttenuationFromDBHL(float dBHL);

		// Calculate expander threshold corresponding to one hearing level in dBHL
		float CalculateThresholdFromDBHL(float dBHL);
		
	private:															// PRIVATE METHODS

		// Calculate expander ratio corresponding to one hearing level in dBHL
		float CalculateRatioFromDBHL(float dBHL);

		float CalculateDBHLFromAttenuation(float attenuation);


		void SetMultibandExpanderParameters(CMultibandExpander* multibandExpander, int bandIndex, TAudiometry audiometry, bool filterGrouping);


	private:                                                           // PRIVATE ATTRIBUTES

		// Multiband expander
		Common::CEarPair<shared_ptr<CMultibandExpander>> multibandExpanders;	// Multiband expanders for both ears
		Common::CEarPair<TAudiometry> audiometries;					// Audiometries (hearing levels) for both ears, in dB SPL																	
		float dBs_SPL_for_0_dBs_fs;									// Equivalence between 0 dBFS and X dBSPL, coming from eventual calibration	

		// Temporal Distortion
		CTemporalDistortionSimulator temporalDistortionSimulator;	// Temporal Distortion simulator 

		// Frequency smearing
		Common::CEarPair<shared_ptr<CFrequencySmearing>> frequencySmearers;	// Frequency smearing processors for both ears
		Common::CEarPair<Common::CDelay> frequencySmearingBypassDelay;		// Buffers for delay compensation when only one ear is affected by frequency smearing

		// Switches for each effect, for each ear
		Common::CEarPair<bool> enableHearingLossSimulation;				// Global switch for whole hearing loss simulation process, for each ear
		Common::CEarPair<bool> enableMultibandExpander;					// Switches for multiband expander process, for each ear
		Common::CEarPair<bool> enableFrequencySmearing;					// Switches for frequency smearing process, for each ear		
	};
}// end namespace HAHLSimulation
#endif
