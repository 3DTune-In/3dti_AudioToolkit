/**
* \class CFiltersBank
*
* \brief Declaration of FiltersBank class interface.
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

#ifndef _CFILTERS_BANK_H_
#define _CFILTERS_BANK_H_

#include <Common/BiquadFilter.h>
#include <vector>
#include <memory>

//using namespace std;  //TODO: Try to avoid this

namespace Common {

	/** \details Class to handle processing through a set of parallel digital filters whose contributions are added.
	*/
	class CFiltersBank
	{
	public:                                                             // PUBLIC METHODS

		/** \brief Default constructor
		*   \eh Nothing is reported to the error handler.
		*/
		CFiltersBank();

		/** \brief Create and add a new CBiquadFilter object to the bank.
		*	\retval filter shared pointer to filter created and added to the bank
		*   \throws May throw exceptions and errors to debugger
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		shared_ptr <Common::CBiquadFilter> AddFilter();

		/** \brief Get one filter from the bank
		*	\param [in] index ID of the filter within the bank
		*	\retval filter shared point to filter from the bank
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		shared_ptr <Common::CBiquadFilter> GetFilter(int index);

		/** \brief Remove all previously created filters.
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void RemoveFilters();

		/** \brief Get the current number of filters in the bank
		*	\retval n Current number of filters in the bank
		*   \eh Nothing is reported to the error handler.
		*/
		int GetNumFilters();

		/** \brief Process an input buffer through the whole set of filters
		*	\details The input buffer is processed by every filter in the bank. The outputs of the filters are added and returned in the output buffer
		*	\param [in] inBuffer input buffer
		*	\param [out] outBuffer output buffer
		*	\pre The size of the buffers must be the same, which should be greater than 0
		*   \eh On error, an error code is reported to the error handler.
		*/
		void Process(CMonoBuffer<float> &  inBuffer, CMonoBuffer<float> & outBuffer);


	private:
		// PRIVATE ATTRIBUTES
		vector<shared_ptr<Common::CBiquadFilter>> filters;                      // Hold the filters in the Bank. 
																		// Indexes indicate the order within the Bank.
	};
}// end namespace Common
#endif
