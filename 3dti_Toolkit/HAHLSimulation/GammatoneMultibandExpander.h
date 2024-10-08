/**
* \class CGammatoneMultibandExpander
*
* \brief Declaration of CGammatoneMultibandExpander class interface. 
*
* \date	June 2019
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

#ifndef _CGAMMATONEMULTIBAND_EXPANDER_H_
#define _CGAMMATONEMULTIBAND_EXPANDER_H_

#include <HAHLSimulation/MultibandExpander.h>
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
	class CGammatoneMultibandExpander : public CMultibandExpander
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Setup the multiband expander
		*	\details Specifies the bands (number, initial frequency and number of internal filters per band, to increase bandwidth).
		*	\param [in] samplingRate sampling rate in samples per second
		*	\param [in] iniFreq_Hz initial frequency, in Hertzs
		*	\param [in] bandsNumber number of frequency bands
		* 	\param [in] filterGrouping specifies if the filters are grouped or not
		*	\pre parameter filtersPerBand must be an odd number.
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int samplingRate, float iniFreq_Hz, int bandsNumber, bool filterGrouping);

		/** \brief Process an input buffer
		*	\details The input buffer is processed by the multiband expander. The result is returned in the output buffer
		*	\param [in] inputBuffer input buffer
		*	\param [out] outputBuffer output buffer
		*	\pre The size of the buffers must be the same, which should be greater than 0
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer);

		float GetFilterFrequency(int bandIndex);

		/** \brief Get the current number of bands in the equalizer.
		*	\retval n number of bands in the equalizer.
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumBands(bool filterGrouping);

		/** \brief Returns a reference to the expander object of one band.
		*	\param [in] bandIndex band index for which the expander will be returned
		*	\retval expander Pointer to the expander object 
		*   \eh Nothing is reported to the error handler.
		*/
		Common::CDynamicExpanderMono* GetBandExpander(int bandIndex, bool filterGrouping);
		
		void SetAttenuationForOctaveBand(int bandIndex, float attenuation);

		float GetAttenuationForOctaveBand(int bandIndex);

		float GetOctaveBandFrequency(int bandIndex);

		bool IsReady();

		void SetGroups(vector<float> bandLimits);

		void SetFilterGrouping(bool filterGrouping);
		bool GetFilterGrouping();					

		float GetBandFrequency(int bandIndex, bool filterGrouping);

		float GetNumFilters();

	private:

		// Calculates the gain applied to a specific filter
		float GetFilterGain(int filterIndex);		

		// Calculates the gain applied to a specific filter, in dB
		float GetFilterGainDB(int filterIndex);		

		// Calculate factor to multiply to the samples, from a (positive) attenuation value in decibels
		float CalculateAttenuationFactor(float attenuation);

		// Calculate frequency and index of immediately lower band to the specified filter
		float GetLowerOctaveBandFrequency(float filterFrequency, int &lowerBandIndex);

		// Calculate frequency and index of immediately higher band to the specified filter
		float GetHigherOctaveBandFrequency(float filterFrequency, int &lowerBandIndex);

		// Calculates the first and last index of the filterbank that belong to a specific group of filters
		void GetBandsFirstAndLastIndex(int bandIndex, int &firstInternalBand, int &lastInternalBand); 
		
		// Calculates the corresponding gain for a specific filter group
		float GetBandGain(int bandIndex);						
		
		// Calculates the corresponding gain for a specific filter group, in dB
		float GetBandGainDB(int bandIndex);						
		
		// Cleans all buffers of the filter bank
		void CleanAllBuffers();									
		
		// Adds a mono expander with default settings to a vector
		void AddMonoExpander(vector<Common::CDynamicExpanderMono*>& monoExpanderVector); 

		vector<Common::CDynamicExpanderMono*> perGroupBandExpanders;			 // Dynamic expanders for each band	group	
		vector<Common::CDynamicExpanderMono*> perFilterGammatoneBandExpanders;	 // Dynamic expanders for each Gammatone filter		

		vector<float> groupBandCentralFrequencies_Hz;
		vector<float> octaveBandFrequencies_Hz;					// Center frequencies for each equalizer band, in Hertzs
		vector<float> octaveBandGains_dB;						// Gains for each equalizer band, in decibels
		
		vector<float> gammatoneExpanderBandFrequencies_Hz;		// Center frequencies for each Gammatone filter, in Hertzs
		vector<float> gammatoneLowerBandFactors;				// Factor for the attenuation linear interpolation for each Gammatone filter regarding the immediately lower band's attenuation
		vector<float> gammatoneHigherBandFactors;				// Factor for the attenuation linear interpolation for each Gammatone filter regarding the immediately higher band's attenuation
		vector<int>   gammatoneLowerBandIndices;				// Index of the immediately lower bands' attenuation for each Gammatone filter
		vector<int>   gammatoneHigherBandIndices;				// Index of the immediately higher bands' attenuation for each Gammatone filter
		
		vector<vector<int>>   bandIndices;						// Vector containing the indices of the first and last filter index for each group
		vector<float> bandLimits_Hz;							// Boundary frequencies between band groups, in Hertzs
		vector<float> gammatoneLowerBandGroupFactors;			// Factor for the attenuation linear interpolation for each band group regarding the immediately lower band's attenuation
		vector<float> gammatoneHigherBandGroupFactors;			// Factor for the attenuation linear interpolation for each band group regarding the immediately higher band's attenuation
		vector<int>   gammatoneLowerBandGroupIndices;			// Index of the immediately lower bands' attenuation for each Gammatone filter group
		vector<int>   gammatoneHigherBandGroupIndices;			// Index of the immediately higher bands' attenuation for each Gammatone filter group

		bool octaveBandFilterGrouping;

		Common::CGammatoneFilterBank gammatoneFilterBank;		// Filter bank to process the data
		vector<float> octaveBandAttenuations;					// Attenuation applied after expander for each band
		bool setupDone;
		unsigned int samplingRate;

	};
}// end namespace HAHLSimulation
#endif

