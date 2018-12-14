/**
* \class CMultibandExpander
*
* \brief Declaration of CMultibandExpander class interface. 
*
* \date	June 2017
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

#ifndef _CMULTIBAND_EXPANDER_H_
#define _CMULTIBAND_EXPANDER_H_

#include <Common/FiltersBank.h>
#include <Common/GammatoneFilterBank.h>
#include <Common/EnvelopeDetector.h>
#include <Common/CommonDefinitions.h>
#include <Common/BiquadFilter.h>
#include <Common/DynamicExpanderMono.h>
#include <vector>
#include <memory>

// Default values for initialization of expanders
#define DEFAULT_RATIO 1
#define DEFAULT_THRESHOLD 0
#define DEFAULT_ATTACK 20
#define DEFAULT_RELEASE 100

namespace HAHLSimulation {

	/** \details This class implements a multiband equalizer where each band has an	independent envelope follower and expander. 
	*	This is used for simulation of non-linear attenuation in hearing loss.
	*/
	class CMultibandExpander
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Setup the multiband expander
		*	\details Specifies the bands (number, initial frequency and number of internal filters per band, to increase bandwidth).
		*	\param [in] samplingRate sampling rate in samples per second
		*	\param [in] iniFreq_Hz initial frequency, in Hertzs
		*	\param [in] bandsNumber number of frequency bands
		*	\pre parameter filtersPerBand must be an odd number.
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int samplingRate, float iniFreq_Hz, int bandsNumber);

		/** \brief Process an input buffer
		*	\details The input buffer is processed by the multiband expander. The result is returned in the output buffer
		*	\param [in] inputBuffer input buffer
		*	\param [out] outputBuffer output buffer
		*	\pre The size of the buffers must be the same, which should be greater than 0
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer);

		/** \brief Get the frequency in Hertzs of the band whose index is passed
		*	\param [in] bandIndex index of the band whose frequency is requiered
		*	\retval frequency centre frequency in Hzs of the band
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		float GetBandFrequency(int bandIndex);

		/** \brief Get the current number of bands in the equalizer.
		*	\retval n number of bands in the equalizer.
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumBands() { return bandFrequencies_Hz.size(); }

		/** \brief Returns a reference to the expander object of one band.
		*	\param [in] bandIndex band index for which the expander will be returned
		*	\retval expander Pointer to the expander object 
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CDynamicExpanderMono* GetBandExpander(int bandIndex) { return bandExpanders[bandIndex]; }

		/** \brief Set attenuation to be applied after expander for one band.
		*	\param[in] bandIndex index of the band for which attenuation will be set
		*	\param[in] attenuation attenuation value in (positive) decibels
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		void SetAttenuationForBand(int bandIndex, float attenuation);

		/** \brief Get attenuation applied after the expander of one band.
		*	\param[in] bandIndex index of the band
		*	\retval attenuation attenuation in (positive) decibels
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		float GetAttenuationForBand(int bandIndex);

	private:

		// Get the first and last index in the filter bank for the internal filters corresponding to a given band index
		void GetBandFiltersFirstAndLastIndex(int bandIndex, int &firstFilterIndex, int &lastFilterIndex);

		// Get configured number of filters per band, to increase bandwidth
		int GetNumberOfFiltersPerBand();

		// Calculate factor to multiply to the samples, from a (positive) attenuation value in decibels
		float CalculateAttenuationFactor(float attenuation);

		// Calculate the corresponding gain for each filter
		float GetFilterGain(float filterIndex);

		// Calculate frequency and index of immediately lower band to the specified filter
		float GetLowerBandFrequency(float filterFrequency, int &lowerBandIndex);

		// Calculate frequency and index of immediately higher band to the specified filter
		float GetHigherBandFrequency(float filterFrequency, int &lowerBandIndex);

		vector<Common::CDynamicExpanderMono*> bandExpanders;	// Dynamic expanders for each band		
		vector<float> bandFrequencies_Hz;				// Center frequencies for each equalizer band, in Hertzs	
		vector<float> bandGains_dB;					// Gains for each equalizer band, in decibels
		vector<float> lowerBandFactors;				// Factor for the attenuation linear interpolation for each filter regarding the immediately lower band's attenuation
		vector<float> higherBandFactors;			// Factor for the attenuation linear interpolation for each filter regarding the immediately higher band's attenuation
		vector<int> lowerBandIndices;				// Index of the immediately lower bands' attenuation for each filter
		vector<int> higherBandIndices;				// Index of the immediately higher bands' attenuation for each filter
		Common::CGammatoneFilterBank filterBank;		// Filter bank to process the data
		vector<float> bandAttenuations;					// Attenuation applied after expander for each band
	};
}// end namespace HAHLSimulation
#endif

