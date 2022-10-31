/**
* \class CStaticEqualizer
*
* \brief Declaration of CStaticEqualizer class interface. 
*
* \date	January 2022
*
* \authors F. Arebola-Pérez and A. Reyes-Lecuona, members of the 3DI-DIANA Research Group (University of Malaga)
* \b Contact: A. Reyes-Lecuona as head of 3DI-DIANA Research Group (University of Malaga): areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SAVLab (Spatial Audio Virtual Laboratory) ||
* \b Website:
*
* \b Copyright: University of Malaga - 2021
*
* \b Licence: GPLv3
*
* \b Acknowledgement: This project has received funding from Spanish Ministerio de Ciencia e Innovación under the SAVLab project (PID2019-107854GB-I00)
*/
#ifndef _CGRAPHIC_EQUALIZER_H_
#define _CGRAPHIC_EQUALIZER_H_

#include <Common/FiltersBank.h>
// ************************************* #include <Common/EnvelopeDetector.h>
#include <Common/CommonDefinitions.h>
#include <Common/BiquadFilter.h>
#include <vector>
#include <memory>

namespace Common {

	/** \details Class to implement a static equalizer.
	*/
	class CGraphicEqualizer
	{
		public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*	\details Setup default values for all internal attributes
		*   \eh Nothing is reported to the error handler.
		*/
		CGraphicEqualizer();

		/** \brief Setup the static equalizer
		*	\details Specifies the bands (number, frequency step, initial frequency and the Q factor of the filters.
		*	\param [in] samplingRate sampling rate, in samples per second
		*	\param [in] iniFreq_Hz initial frequency, in Hertzs
		*	\param [in] bandsNumber number of frequency bands
		*	\param [in] octaveBandStep especifies the frequency step to determine the bandwidth of each band.
		*	\param [in] Q_BPF Q-factor of the band-pass filters
		*	\n for instance, if octaveBandStep is equal to 1, one octave will be used:
		*	setup(  fs, 125, 7, 1, Q ); will produce center frequencies of: 125, 250, 500, 1000, 2000, 4000, 8000
		*	\n If octaveBandStep is equal to 3, a third of octave will be use instead:
		*	setup(  fs, 125, 7, 3, Q ); will produce center frequencies of: 125, 160, 200, 250, 315, 400, 500
		*	\pre iniFreq_Hz must be any of the 31 center frequency values for the 31 ISO bands:
		*	20, 25, 31, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000
		*	2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000
		*   \eh Nothing is reported to the error handler.
		*/
		void Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, int octaveBandStep, float Q_BPF);

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
		
		/** \brief Returns the current number of bands in the dynamic equalizer.
		*	\retval n number of bands for each level of the dynamic equalizer
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumBands() { return bandFrequencies_Hz.size(); }
	
	private:

		vector<float> bandFrequencies_Hz;      // Center frequencies for each equalizer band, in Hertzs
		Common::CFiltersBank filterBank;        // Filter Bank to proccess the data
	};
}// end namespace Common
#endif
