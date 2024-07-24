/*
* \class CFiltersChain
*
* \brief Definition of FiltersChain class.
*
* Class to handle a set of cascade digital filters
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

#include <Common/FiltersChain.h>
#include <Common/ErrorHandler.h>

namespace Common {
	//////////////////////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR

	CFiltersChain::CFiltersChain()
	{
	}

	//////////////////////////////////////////////
	shared_ptr<CBiquadFilter> CFiltersChain::AddFilter()
	{
		try
		{
			shared_ptr<CBiquadFilter> newFilter(new CBiquadFilter());
			filters.push_back(newFilter);

			SET_RESULT(RESULT_OK, "Filter added to filter chain succesfully");
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

	shared_ptr<CBiquadFilter> CFiltersChain::GetFilter(int index)
	{
		if (index < 0 || filters.size() <= index)
		{
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to get a filter from filter chain outside chain size");
			return NULL;
		}
		else
		{
			//SET_RESULT(RESULT_OK, "Succesfully got filter from filter chain");
			return filters[index];
		}
	}

	//////////////////////////////////////////////
	void CFiltersChain::RemoveFilters()
	{
		filters.clear();

		SET_RESULT(RESULT_OK, "All filters succesfully removed from filter chain");
	}

	//////////////////////////////////////////////
	void CFiltersChain::Process(CMonoBuffer <float> & buffer)
	{
		//SET_RESULT(RESULT_OK, "");
		for (std::size_t c = 0; c < filters.size(); c++)
		{
			shared_ptr<CBiquadFilter> f = filters[c];
			if (f != NULL)
				f->Process(buffer);
		}
	}

	//////////////////////////////////////////////
	void CFiltersChain::Process(CMonoBuffer <float>& buffer, CMonoBuffer<float>& outBuffer)
	{
		//SET_RESULT(RESULT_OK, "");
		for (std::size_t c = 0; c < filters.size(); c++)
		{
			shared_ptr<CBiquadFilter> f = filters[c];
			if (f != NULL)
				f->Process(buffer, outBuffer);
				buffer = outBuffer;
		}
	}

	//////////////////////////////////////////////
	int CFiltersChain::GetNumFilters()
	{
		return filters.size();
	}

	//////////////////////////////////////////////
	void CFiltersChain::SetFromCoefficientsVector(TFiltersChainCoefficients& coefficients)
	{		
		if (coefficients.size() == filters.size())
		{
			// Set existing filters
			for (int i = 0; i < coefficients.size(); i++)
			{
				filters[i]->SetCoefficients(coefficients[i]);
			}
		}
		else
		{
			// Create chain from scratch
			RemoveFilters();
			for (int i = 0; i < coefficients.size(); i++)
			{
				shared_ptr<Common::CBiquadFilter> newBiquad = AddFilter();
				newBiquad->SetCoefficients(coefficients[i]);
			}
		}
	}
}//end namespace Common