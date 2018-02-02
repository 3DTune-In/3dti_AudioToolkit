/**
* \class CLowHighSplitFilter
*
* \brief Declaration of CHighOrderButterworthFilter class interface. 
*
* \date	October 2017
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



#ifndef _CHIGHORDER_BUTTERWORTH_FILTER_H_
#define _CHIGHORDER_BUTTERWORTH_FILTER_H_

#include <Common/Buffer.h>
#include <Common/FiltersChain.h>
#include <unordered_map>

// Default values for high order filter parameters
#define DEFAULT_SAMPLE_RATE 44100
#define DEFAULT_CUTOFF_FREQUENCY 1600

/** \brief Definition of high order filter parameters for table lookup
*/
struct THighOrderFilterParameters
{
	int sampleRate;			///< Sample rate, in Hz
	int cutoffFrequency;	///< Cutoff frequency, in Hz

	THighOrderFilterParameters(int _sampleRate, int _cutoffFrequency) :sampleRate{ _sampleRate }, cutoffFrequency{ _cutoffFrequency } {}
	THighOrderFilterParameters() :THighOrderFilterParameters{ DEFAULT_SAMPLE_RATE, DEFAULT_CUTOFF_FREQUENCY } {}

	bool operator==(const THighOrderFilterParameters& oth) const
	{
		return ((this->sampleRate == oth.sampleRate) && (this->cutoffFrequency== oth.cutoffFrequency));
	}
};

// Hash key for high order filter parameters table lookup
template<>
struct std::hash<THighOrderFilterParameters>
{	
	// adapted from http://en.cppreference.com/w/cpp/utility/hash
	size_t operator()(const THighOrderFilterParameters & key) const
	{
		size_t h1 = std::hash<int32_t>()(key.sampleRate);
		size_t h2 = std::hash<int32_t>()(key.cutoffFrequency);
		return h1 ^ (h2 << 1);  // exclusive or of hash functions for each int.
	}
};

/** \brief Type definition for the high order filter coefficients table
*/
typedef std::unordered_map<THighOrderFilterParameters, Common::TFiltersChainCoefficients> THighOrderFilterCoefficientsTable;


namespace HAHLSimulation {

	// Hardcoded coefficients table for 6th order low pass filters
	static void FillLowPassFilterOrder6CoefficientsTable();
	static THighOrderFilterCoefficientsTable lowPassFilterOrder6Table;
	static bool isLowPassFilterOrder6TableFilled = false;

	// Hardcoded coefficients table for 4th order low pass filters
	static void FillLowPassFilterOrder4CoefficientsTable();
	static THighOrderFilterCoefficientsTable lowPassFilterOrder4Table;
	static bool isLowPassFilterOrder4TableFilled = false;

	// Hardcoded coefficients table for 4th order high pass filters
	static void FillHighPassFilterOrder4CoefficientsTable();
	static THighOrderFilterCoefficientsTable highPassFilterOrder4Table;
	static bool isHighPassFilterOrder4TableFilled = false;
	
	/** \details This class implements different types (LPF and HPF) of high order butterworth filters which coefficients are in hardcoded tables.
	*/
	class CHighOrderButterworthFilter
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Setup the filter
		*	\details Setup the internal biquads coefficients depending on the sampling rate, cutoff frequency, filter type and filter order. 
		*	The coefficients are taken from	a hardcoded coefficients table. The table to use is chosen depending on filter type and filter order.
		*	If this is the first time a filter of this type and order is setup, the hardcoded table will be filled also in this setup.
		*	Once the table is selected, the entry to get from the table depends on the sampling rate and the cutoff frequency.
		*	\param [in] samplingRate sampling rate in Hzs
		*	\param [in] cutoffFrequency cutoff frequency, in Hzs
		*	\param [in] filterType type of filter (LOWPASS or HIGHPASS)
		*	\param [in] filterOrder order of filter (currently, only 6 is supported)
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Setup(int samplingRate, int cutoffFrequency, Common::T_filterType filterType, int filterOrder);

		/** \brief Process an input buffer
		*	\details Process input buffer to return filtered output buffer
		*	\param [in] inputBuffer input buffer
		*	\param [out] outputBuffer filtered output buffer		
		*   \throws May throw exceptions and errors to debugger
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> & inputBuffer, CMonoBuffer<float> & outputBuffer);

		/** \brief Set filter coefficients, depending on sampling rate and cutoff frequency
		*	\details Coefficients are stored in the hardcoded table that was chosen during setup
		*	\param [in] samplingRate sampling rate, in Hzs
		*	\param [in] cutoffFrequency cutoff frequency, in Hzs
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetFilterCoefficients(int samplingRate, int cutoffFrequency);

	private:

		THighOrderFilterCoefficientsTable* coefficientsTable;
		Common::CFiltersChain filtersChain;
	};
}// end namespace HAHLSimulation
#endif


