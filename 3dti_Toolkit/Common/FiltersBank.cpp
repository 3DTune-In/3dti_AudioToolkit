/*
* \class CFiltersBank
*
* \brief Definition of FiltersBank class.
*
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

#include <Common/FiltersBank.h>
#include <Common/ErrorHandler.h>

namespace Common {

	//////////////////////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR

	CFiltersBank::CFiltersBank()
	{
	}

	//////////////////////////////////////////////
		// Creates and add a new CBiquadFilter object to the Bank.
	shared_ptr<Common::CBiquadFilter> CFiltersBank::AddFilter()
	{
		try
		{
			shared_ptr<Common::CBiquadFilter> newFilter(new Common::CBiquadFilter());
			filters.push_back(newFilter);

			SET_RESULT(RESULT_OK, "Filter added to filter bank succesfully");
			return newFilter;
		}
		catch (std::bad_alloc& ba)
		{
			//SET_RESULT(RESULT_ERROR_BADALLOC, ba.what());
			ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
			return nullptr;
		}
	}

	//////////////////////////////////////////////
		// Returns the <index>th filter in the Bank
	shared_ptr<Common::CBiquadFilter> CFiltersBank::GetFilter(int index)
	{
		if (index < 0 || filters.size() <= index)
		{
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to get a filter from filter bank outside bank size");
			return NULL;
		}
		else
		{
			SET_RESULT(RESULT_OK, "Succesfully got filter from filter bank");
			return filters[index];
		}
	}

	//////////////////////////////////////////////
		// Remove all the filters previously created. 
	void CFiltersBank::RemoveFilters()
	{
		filters.clear();

		SET_RESULT(RESULT_OK, "All filters succesfully removed from filter bank");
	}

	//////////////////////////////////////////////
		// Returns the current number of filters in the bank
	int CFiltersBank::GetNumFilters()
	{
		return filters.size();
	}

	//////////////////////////////////////////////
		// Applies the whole set of filters in the Bank to the data
		// inBuffer is processed by every filter in the bank. The outputs of the filters are added and returned in outBuffer
		// The size of the buffers must have the same value which should be greater than 0 
	void CFiltersBank::Process(CMonoBuffer<float> &  inBuffer, CMonoBuffer<float> & outBuffer)
	{
		int size = inBuffer.size();

		//if (size <= 0)
		//{
		//	SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The input buffer is empty");			
		//	return;
		//}
		//else if (size != outBuffer.size())
		//{
		//	SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Input and output buffers size must agree");			
		//	return;
		//}
		ASSERT(size > 0, RESULT_ERROR_BADSIZE, "Attempt to process a filter bank with an empty input buffer", "");
		ASSERT(size == outBuffer.size(), RESULT_ERROR_BADSIZE, "Attempt to process a filter bank with different sizes for input and output buffers", "");
		//SET_RESULT(RESULT_OK, "");

		bool addResult = false;

		for (std::size_t c = 0; c < filters.size(); c++)
		{
			shared_ptr<Common::CBiquadFilter> f = filters[c];
			if (f != NULL)
			{
				f->Process(inBuffer, outBuffer, addResult);
				addResult = true;
			}
		}
	}
}// end namespace Common