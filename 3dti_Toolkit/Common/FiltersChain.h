/**
* \class CFiltersChain
*
* \brief Declaration of FiltersChain class interface.
* \date	July 2016
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

#ifndef _CFILTERS_CHAIN_H_
#define _CFILTERS_CHAIN_H_

#include <Common/BiquadFilter.h>
#include <vector>
#include <memory>

using namespace std;  //TODO: Try to avoid this

namespace Common {

	/** \brief Type definition for a set of coefficients of a filters chain
	*/
	typedef std::vector<TBiquadCoefficients> TFiltersChainCoefficients;

	/** \details Class to handle a set of cascade digital filters that are arranged so the samples are processed along a pipeline
	*/
	class CFiltersChain
	{
	public:
		/////////////
		// METHODS
		/////////////

		/** \brief Default constructor
		*   \eh On error, an error code is reported to the error handler.
		*/
		CFiltersChain();


		/** \brief Create and add a new CBiquadFilter object to the chain
		*	\retval filter shared pointer to filter created and added to the bank
		*   \throws May throw exceptions and errors to debugger
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		shared_ptr <CBiquadFilter> AddFilter();

		/** \brief Get one filter from the chain
		*	\param [in] index ID of the filter within the chain
		*	\retval filter shared pointer to filter from the chain
		*   \eh On error, an error code is reported to the error handler.
		*/
		shared_ptr <CBiquadFilter> GetFilter(int index);

		/** \brief Remove all previously created filters.
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void RemoveFilters();

		/** \brief Get the current number of filters in the chain
		*	\retval n Current number of filters in the chain
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumFilters();

		/** \brief Process an buffer through the whole set of filters
		*	\details The buffer is processed through each filter in the bank in chain.
		*	\param [in,out] buffer input and output buffer		
		*   \eh Nothing is reported to the error handler.
		*/
		void Process(CMonoBuffer <float> & buffer);

		/** \brief Process an buffer through the whole set of filters 
		 * 	\param [in] inBuffer input buffer
		 * 	\param [out] outBuffer output buffer
		*/
		void Process(CMonoBuffer <float> & inBuffer, CMonoBuffer <float> & outBuffer);

		/** \brief Setup a filters chain from a vector of (ordered) coefficients for any number of biquads	
		*	\details If the number of coefficients in the vector fits the current number of filters in the chain, the existing filter coefficients are set, 
		*	instead of creating a new filters chain from scratch
		*	\param [in] coefficients vector of ordered coefficients for all biquads in the chain				
		*   \eh Nothing is reported to the error handler.
		*/
		void SetFromCoefficientsVector(TFiltersChainCoefficients& coefficients);

	private:
		////////////////////////
		// PRIVATE ATTRIBUTES
		////////////////////////
		vector<shared_ptr<CBiquadFilter>> filters;                      // Hold the filters in the chain. 
																		// Indexes indicate the order within the chain.
	};
}//end namespace Common
#endif