/**
* \class CGammatoneFilterBank
*
* \brief Declaration of FiltersBank class interface.
* \date	July 2016
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre, D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
* \b This module was written by Michael Krzyzaniak m.krzyzaniak@surrey.ac.uk
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

#ifndef _CGAMMATONE_FILTER_BANK_H_
#define _CGAMMATONE_FILTER_BANK_H_

#include <Common/GammatoneFilter.h>
#include <vector>
#include <memory>

using namespace std;	//TODO: Try to avoid this

namespace Common {

	/** \details This model implements a Patterson-Holdsworth Auditory Filterbank
	*	As described in:
	*	Malcolm Slaney
	*	An Efficient Implementation of the Patterson-Holdsworth Auditory Filter Bank 
	*	Apple Computer Technical Report #35
	*	Perception Group—Advanced Technology Group
	*	(c) 1993, Apple Computer, Inc.
	*	https://engineering.purdue.edu/~malcolm/apple/tr35/PattersonsEar.pdf
	*	Note that the actual filter is implemented in Common/GammatoneFilter.cpp,
	*	and is marginally different than what is reported in the Slaney paper.
	*	Slaney is used only to determine the spacing and number of filters.
	*	Because there are no equation numbers, comments throughout
	*	this code refer to the relevant page numbers.
	*/
	class CGammatoneFilterBank
	{
	public:
			/** \brief Constants that define how the ceneter frequency of a filter should
		*	relate to its bandwidth. This determines not only the bandwidth, but
		*	also the number and spacing of filters within a filterbank.
		*	These are to be passed as arguments to methods like InitWithFreqRangeOverlap,
		*	GetERBOfHumanAuditoryFilter, etc. If you aren't sure, just pass EAR_MODEL_DEFAULT.
		*	Further datails about these can be found in the Malcolm Slaney paper cited below.
		*/
		enum T_earModel
		{
			EAR_MODEL_GLASBERG,
			EAR_MODEL_LYON,
			EAR_MODEL_GREENWOOD,
			EAR_MODEL_DEFAULT = EAR_MODEL_GLASBERG,
		};

		////////////////////
		// PUBLIC METHODS
		///////////////////

		/** \brief Default empty constructor
		*	\eh Nothing is reported to the error handler.
		*/
		CGammatoneFilterBank();

		/** \brief Default constructor
		*	\param [_samplingFreq] the desired audio sample rate, in Hz.
		*	\eh Nothing is reported to the error handler.
		*/
		CGammatoneFilterBank(double _samplingFreq);

		/** \brief Default destructor
		*	 \eh Nothing is reported to the error handler.
		*/
		~CGammatoneFilterBank();

		/** \brief Remove of all of the current filters and replace them with new ones, spaced according to some perceptual model.
		*	This starts at the Nyquist freq and works its way down. The first filter added to the bank will EQUAL TO
		*	_highFreq, and the lowest one added will be the first whose frequency is LESS THAN OR EQUAL TO _lowFreq.
		*	\param [_lowFreq] lower bound of the desired range
		*	\param [_highFreq] highest frequency in the deisred range
		*	\param [_overlap] how much adjacent filters should overlap. 0 menas no overlap and 1 would mean 100% overlap (must be less than 1). Negative overlap means not all frequencies are covered by the equivalent rectangular filters.
		*	\param [_earModel] one of EAR_MODEL_DEFAULT, EAR_MODEL_GLASBERG, EAR_MODEL_LYON, EAR_MODEL_GREENWOOD
		*	\retval n number of filters that was needed to sample the frequency range with the given overlap. -1 on error
		*/
		unsigned InitWithFreqRangeOverlap(double _lowFreq, double _highFreq, double _overlap, T_earModel _earModel);

		/** \brief Remove of all of the current filters and replace them with new ones, spaced according to some perceptual model
		*	This starts at the Nyquist freq and works its way down. The first filter added to the bank will be EQUAL TO
		*	_highFreq, and the last one will be EQUAL TO _lowFreq, exactly respecting the specified number of filters.
		*	\param [_lowFreq] lower bound of the desired range
		*	\param [_highFreq] highest frequency in the deisred range
		*	\param [_numFilters] how many filters to use to cover the frequency range
		*	\param [_earModel] one of EAR_MODEL_DEFAULT, EAR_MODEL_GLASBERG, EAR_MODEL_LYON, EAR_MODEL_GREENWOOD
		*	\retval overlap the filter overlap coefficient used to cover the frequency range with the given number of filters
		*/
		double InitWithFreqRangeNumFilters(double _lowFreq, double _highFreq, unsigned _numFilters, T_earModel _earModel);

		/** \brief Get the bandwidth of the human auditory filter
		*	\param [_lowFreq] index ID of the filter within the bank
		*	\param [_earModel] one of EAR_MODEL_DEFAULT, EAR_MODEL_GLASBERG, EAR_MODEL_LYON, EAR_MODEL_GREENWOOD
		*	\retval ERB the bandwidth, in Hz, of the auditory filter at the given frequency, using the given model
		*/
		double GetERBOfHumanAuditoryFilter(double _centerFreq, T_earModel _earModel);

		/** \brief Set the bandwidth of a gammatone filter to the specified frequency, using the bandwidth determined by the given ear model.
		*	\param [_index] the index of the filter to be adjusted.
		*	\param [_highFreq] index ID of the filter within the bank
		*	\param [overlap] index ID of the filter within the bank
		* \retval ERB the bandwidth, in Hz, of the auditory filter at the given frequency, using the given model, negative if a filter does not exist at the given index
		*/
		double SetFreqBandwidthOfFilter(int _index, double _centerFreq, T_earModel _earModel);

		/** \brief Create and add a new CGammatoneFilter object to the bank.
		*	\param [_order] the filter order. 4 is normally the correct answer.
		*	\param [_freq] the center frequency of the filter
		*	\param [_erb] the equivalent rectangular bandwidth, in Hz, of the filter.
		*	\retval filter shared pointer to filter created and added to the bank
		*	\throws May throw exceptions and errors to debugger
		*	\eh On success, RESULT_OK is reported to the error handler.
		*			 On error, an error code is reported to the error handler.
		*/
		shared_ptr <Common::CGammatoneFilter> AddFilter(unsigned _order, double _freq, double _erb);

		/** \brief Get one filter from the bank
		*	\param [in] index ID of the filter within the bank
		*	\retval filter shared point to filter from the bank
		*	\eh On success, RESULT_OK is reported to the error handler.
		*			 On error, an error code is reported to the error handler.
		*/
		shared_ptr <Common::CGammatoneFilter> GetFilter(int _index);

		/** \brief Remove all previously created filters.
		*	 \eh On success, RESULT_OK is reported to the error handler.
		*/
		void RemoveFilters();

		/** \brief Get the current number of filters in the bank
		*	\retval n Current number of filters in the bank
		*	\eh Nothing is reported to the error handler.
		*/
		int GetNumFilters();

		/** \brief Set the sampling frequency at which audio samples were acquired. This will adjust each filter in the bank, accordingly.
		*	\param [_samplingFreq] _samplingFreq sampling frequency, in Hz
		*	\eh On success, RESULT_OK is reported to the error handler.
		*			 On error, an error code is reported to the error handler.
		*/
		void SetSamplingFreq(float _samplingFreq);

		/** \brief Get the sample rate, in Hz, of the filter
		*	\retval freq filter sampling frequency
		*	\eh Nothing is reported to the error handler.
		*/
		float GetSamplingFreq();

		/** \brief Process an input buffer through the whole set of filters
		*	\details The input buffer is processed by every filter in the bank. The outputs of the filters are added and returned in the output buffer
		*	\param [in] inBuffer input buffer
		*	\param [out] outBuffer output buffer
		*	\pre The size of the buffers must be the same, which should be greater than 0
		*	\eh On error, an error code is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> &	inBuffer, CMonoBuffer<float> & outBuffer);


	private:
		////////////////////
		// PRIVATE METHODS
		///////////////////
		void GetEarModel(double* ear_q, double* min_bandwidth, double* ear_q_min_bw, T_earModel _earModel);

		////////////////
		// ATTRIBUTES
		////////////////
		vector<shared_ptr <Common::CGammatoneFilter> > filters;			// Hold the filters in the Bank.
		double samplingFreq;							// Default samnpling freq for adding filters
	};
}// end namespace Common
#endif
