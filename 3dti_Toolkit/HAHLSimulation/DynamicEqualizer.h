/**
* \class CDynamicEqualizer
*
* \brief Declaration of CDynamicEqualizer class interface. 
*
* \date	January 2017
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

#ifndef _CDYNAMIC_EQUALIZER_H_
#define _CDYNAMIC_EQUALIZER_H_

#include <Common/FiltersBank.h>
#include <Common/EnvelopeDetector.h>
#include <Common/CommonDefinitions.h>
#include <Common/BiquadFilter.h>
#include <vector>
#include <memory>

namespace HAHLSimulation {

	/** \details Class to implement a dynamic equalizer with multiple equalization curves or levels.
	*        Depending on the level of the signal, it chooses an equalization curve or another.
	*        It can apply the nearest equalization curve or interpolate between two equalization curves.
	*/
	class CDynamicEqualizer
	{
	public:

		/** \details Class to handle one level of the Dynamic Equalizer.
		*/
		class CEqLevel
		{
		public:
			vector<float> bands;           ///< Contains the value for each band (dBs)			
			float threshold;               ///< Threshold value in dBfs used to decide which EqLevel must be applied 
		};

	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*	\details Setup default values for all internal attributes
		*   \eh Nothing is reported to the error handler.
		*/
		CDynamicEqualizer();


		/** \brief Setup the dynamic equalizer
		*	\details Specifies the bands (number, frequency step, initial frequency and the Q factor of the filters.
		*	\param [in] samplingRate sampling rate, in samples per second
		*	\param [in] numberOfLevels The equalizer is actualy composed by numberOfLevels equalizers and the result is
		*               obtained from them depending on the level of the signal
		*	\param [in] iniFreq_Hz initial frequency, in Hertzs
		*	\param [in] bandsNumber number of frequency bands
		*	\param [in] octaveBandStep especifies the frequency step to determine the bandwidth of each band.
		*	\param [in] Q_BPF Q-factor of the band-pass filters
		*	\n for instance, if octaveBandStep is equal to 1, one octave will be used:
		*	setup(  125, 7, 1 ); will produce center frequencies of: 125, 250, 500, 1000, 2000, 4000, 8000
		*	\n If octaveBandStep is equal to 3, a third of octave will be use instead:
		*	setup(  125, 7, 3 ); will produce center frequencies of: 125, 160, 200, 250, 315, 400, 500
		*	\pre iniFreq_Hz must be any of the 31 center frequency values for the 31 ISO bands:
		*	20, 25, 31, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000
		*	2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate, int numberOfLevels, float iniFreq_Hz, int bandsNumber, int octaveBandStep, float Q_BPF);

		/** \brief Process an input buffer
		*	\details The input buffer is processed by the dynamic equalizer. The result is returned in the output buffer
		*	\param [in] inputBuffer input buffer
		*	\param [out] outputBuffer output buffer
		*	\pre The size of the buffers must be the same, which should be greater than 0
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer);

		/** \brief Set the gains for all bands of the equalizer
		*	\param [in] gains_dB vector with the gains, in decibels
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void SetGains_dB(vector<float> gains_dB);

		/** \brief Set the gain for all bands of the equalizer to 0 dB
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void ResetGains_dB();

		/** \brief Set the gain for a specific band of the equalizer
		*	\param [in] bandIndex ID of the band
		*	\param [in] gain_dB gain, in decibels
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void SetFiltersBankBandGain_dB(int bandIndex, float gain_dB);

		/** \brief returns the frequency in Hertzs of the band whose index is passed
		*	\param [in] bandIndex index of the band whose frequency is requiered
		*	\retval frequency frequency of that band, in Hz
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetBandFrequency(int bandIndex);

		/** \brief Set the internal state required for updating equalizer levels
		*	\details When the info in levels is modified, this function must be called to tag this changes to
		*          be applied when the next frame is processed
		*   \eh Nothing is reported to the error handler.
		*/
		void SetUpdateLevelsIsPending();

		/** \brief Set the gain for a specific band of a specific level of the equalizer
		*	\param [in] levelIndex ID of the level
		*	\param [in] bandIndex ID of the band
		*	\param [in] gain_dB gain, in decibels
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetLevelBandGain_dB(int levelIndex, int bandIndex, float gain_dB);

		/** \brief Returns the gain (dBs) for the specified band and level
		*	\param [in] levelIndex ID of the level
		*	\param [in] bandIndex ID of the band
		*	\retval gainDB gain for that band in that level, in dB
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetLevelBandGain_dB(int levelIndex, int bandIndex);

		/** \brief Set the threshold signal level (dBfs) to apply the curves of the dynamic equalizer
		*	\param [in] levelIndex index of the level
		*	\param [in] threshold_dBfs signal level threshold value in dBfs
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetLevelThreshold(int levelIndex, float threshold_dBfs);

		/** \brief Returns the threshold signal level (dBfs) used to apply the curves of the dynamic equalizer
		*	\param [in] levelIndex index of the level
		*	\retval threshold threshold for that level, in dBfs
		*   \eh On error, an error code is reported to the error handler.
		*/
		float GetLevelThreshold(int levelIndex);

		/** \brief Returns the current number of levels in the dynamic equalizer.
		*	\retval n number of levels in the dynamic equalizer
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumLevels() { return levels.size(); }

		/** \brief Returns the current number of bands in the dynamic equalizer.
		*	\retval n number of bands for each level of the dynamic equalizer
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumBands() { return bandFrequencies_Hz.size(); }

		/** \brief Set the compression percentage for all the levels towards the level 0.
		*	\param [in] percentage percentage of compression. 100% means all the level's gains are set to
		*               those of level 0.
		*   \eh Nothing is reported to the error handler.
		*/
		void SetCompressionPercentage(float percentage);

		/** \brief Returns the compression percentage applied
		*	\retval percentage compression percentage
		*   \eh Nothing is reported to the error handler.
		*/
		float GetCompressionPercentage();

		/** \brief Returns the gain in dBs for specified level and band, after applying compression.
		*	\param [in] levelIndex level index for which the gain will be returned
		*	\param [in] bandIndex band index for which the gain will be returned
		*	\retval gainDB gain in dBs for specified level and band, after applying compression.
		*   \eh Nothing is reported to the error handler.
		*/
		float GetCompressedGain_dB(int levelIndex, int bandIndex);
		
		/** \brief Returns the gain in dBs for specified level and band, after applying compression.
		*	\param [in] level level object for which the gain will be returned
		*	\param [in] bandIndex band index for which the gain will be returned
		*	\retval gainDB gain in dBs for specified level and band, after applying compression.
		*   \eh Nothing is reported to the error handler.
		*/
		float GetCompressedGain_dB(CEqLevel &level, int bandIndex);

		/** \brief Returns the gain offset in dBs that is currently being applied.
		*	\retval offset offset gain in dBs.
		*   \eh Nothing is reported to the error handler.
		*/
		float GetOveralOffset_dB();

		/** \brief Set gain offset in dBs
		*	\param [in] offset_dB gain offset, in dB 
		*   \eh Nothing is reported to the error handler.
		*/
		void SetOveralOffset_dB(float offset_dB);

		/** \brief Set the maximum value for the gains in the equalizer.
		*	\param [in] maxGain_dB maximun value for gains, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		void SetMaxGain_dB(float maxGain_dB);

		/** \brief Set the minimum value for the gains in the equalizer.
		*	\param [in] minGain_dB minimum value for gains, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		void SetMinGain_dB(float minGain_dB);

		/** \brief Returns the maximum value for the gains in the equalizer. 
		*	\retval maxGainDB maximum gain, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		float GetMaxGain_dB();
		
		/** \brief Returns the minimum value for the gains in the equalizer. 
		*	\retval minGainDB minimum gain, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		float GetMinGain_dB();

		/** \brief Get levels Interpolation flag 
		*	\retval levelsInterpolation levels interpolation flag
		*   \eh Nothing is reported to the error handler.
		*/
		bool GetLevelsInterpolation() { return levelsInterpolation; }
		
		/** \brief Enable levels Interpolation 
		*   \eh Nothing is reported to the error handler.
		*/
		void EnableLevelsInterpolation() { levelsInterpolation = true; }

		/** \brief Disable levels Interpolation
		*   \eh Nothing is reported to the error handler.
		*/
		void DisableLevelsInterpolation() { levelsInterpolation = false; }

		/** \brief Returns the attack time in ms. 
		*	\retval attack Attack time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAttack_ms() { return attack_ms; }

		/** \brief Returns the release time in ms. 
		*	\retval release Release time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		float GetRelease_ms() { return release_ms; }

		/** \brief Set the attack time in ms. 
		*	\param [in] _attack_ms attack time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		void SetAttack_ms(float _attack_ms) { attack_ms = _attack_ms; }

		/** \brief Set the release time in ms.
		*	\param [in] _release_ms release time, in ms
		*   \eh Nothing is reported to the error handler.
		*/
		void SetRelease_ms(float _release_ms) { release_ms = _release_ms; }

		/** \brief Returns the signal level provided by the envelope detector. 
		*	\retval leveldB signal level, in dB
		*   \eh Nothing is reported to the error handler.
		*/
		float GetLevel_db() { return level_db; }

		/** \brief Returns a reference to the envelopeDetector object used by the class.
		*	\retval envelopeDetector reference to CEnvelopeDetector object
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CEnvelopeDetector &GetEnvelopeDetector() { return envelopeDetector; }

	private:

		float attack_ms;                        // Attack time in ms for the envelope detector 	
		float release_ms;                       // Release time in ms for the envelope detector 	
		float level_db;                         // Signal level provided by the envelope detector
		Common::CEnvelopeDetector envelopeDetector;     // Envelope detector	
		bool levelsInterpolation;				// When false, applies only the closest equalization curve. Otherwise, interpolates the closest two.
		vector<float> bandFrequencies_Hz;      // Center frequencies for each equalizer band, in Hertzs
		vector<CEqLevel> levels;                // Levels of the dynamic equalizer
		Common::CFiltersBank filterBank;        // Filter Bank to proccess the data
		bool updateBandGainsIsPending;          // true to update the gains of the bands before processing the next frame
		float compressionPercentage;            // Compression percentage towards the level 0 that will be applied
		float overalOffset_dB;                  // Overal offset in dBs applied to every band and level 
		float maxGain_dB;                       // Maximun gain value in dBs 
		float minGain_dB;                       // Minimum gain value in dBs 


		void ApplyLevel(CEqLevel &level, bool applyCompression);	// Aplies the level to the filtersBank
	};
}// end namespace HAHLSimulation
#endif
