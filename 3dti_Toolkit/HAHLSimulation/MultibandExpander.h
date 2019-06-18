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
#define LINEAR_GAIN_CORRECTION_GAMMATONE 4					// equivalent to 12dB gain
#define LINEAR_GAIN_CORRECTION_BUTTERWORTH 0.70710678118	// sqrt(2)/2, equivalent to 3dB attenuation

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
		*	\param [in] filtersPerBand specifies the number of filters per band
		*	\param [in] _filterBank specifies which type of filterbank to use: butterworth or gammatone. Output gain will be corrected to achieve homogeneous gain
		*	\pre parameter filtersPerBand must be an odd number.
		*   \eh On error, an error code is reported to the error handler.
		*/
		virtual void Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, bool filterGrouping) = 0;

		/** \brief Process an input buffer
		*	\details The input buffer is processed by the multiband expander. The result is returned in the output buffer
		*	\param [in] inputBuffer input buffer
		*	\param [out] outputBuffer output buffer
		*	\pre The size of the buffers must be the same, which should be greater than 0
		*   \eh Nothing is reported to the error handler.
		*/
		virtual void Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer) = 0;

		/** \brief Get the frequency in Hertzs of the band whose index is passed
		*	\param [in] bandIndex index of the band whose frequency is requiered
		*	\retval frequency centre frequency in Hzs of the band
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		virtual float GetOctaveBandFrequency(int bandIndex) = 0;

		virtual float GetBandFrequency(int bandIndex, bool filterGrouping) = 0;

		virtual float GetFilterFrequency(int bandIndex) = 0;
		/** \brief Get the current number of bands in the equalizer.
		*	\retval n number of bands in the equalizer.
		*   \eh Nothing is reported to the error handler.
		*/
		virtual int GetNumBands(bool filterGrouping) = 0;

		/** \brief Returns a reference to the expander object of one band.
		*	\param [in] bandIndex band index for which the expander will be returned
		*	\retval expander Pointer to the expander object 
		*   \eh Nothing is reported to the error handler.
		*/
		virtual Common::CDynamicExpanderMono* GetBandExpander(int bandIndex, bool filterGrouping) = 0;

		virtual void SetAttenuationForOctaveBand(int bandIndex, float attenuation) = 0;

		virtual float GetAttenuationForOctaveBand(int bandIndex) = 0;

		virtual bool IsReady() = 0;

		virtual void SetFilterGrouping(bool filterGrouping) = 0;
		virtual bool GetFilterGrouping() = 0;

		// Calculate the corresponding gain for each filter
		virtual float GetFilterGain(int filterIndex) = 0;
		virtual float GetFilterGainDB(int filterIndex) = 0;

		virtual float GetNumFilters() = 0;

	};
}// end namespace HAHLSimulation
#endif

