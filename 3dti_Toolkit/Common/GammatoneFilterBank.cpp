/*
* \class CGammatoneFilterBank
*
* \brief Definition of GammatoneFilterBank class.
*
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

#include <cmath>
#include <Common/GammatoneFilterBank.h>
#include <Common/ErrorHandler.h>

namespace Common {

	#define GAMMATONE_FILTER_ORDER 4
	
	//////////////////////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR

	CGammatoneFilterBank::CGammatoneFilterBank()
	{
	}

	CGammatoneFilterBank::CGammatoneFilterBank(double _samplingFreq)
	{
		 SetSamplingFreq(_samplingFreq);
	}
	
	//////////////////////////////////////////////
	CGammatoneFilterBank::~CGammatoneFilterBank()
	{
	}

	//////////////////////////////////////////////
	unsigned CGammatoneFilterBank::InitWithFreqRangeOverlap(double _lowFreq, double _highFreq, double _overlap, T_earModel _earModel)
	{
		double stepfactor = 1.0 - _overlap;
		if(_overlap >= 1)
		{
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "overlap must be less than 1");
			return -1;
		}
		if(_lowFreq >= _highFreq)
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "low freq should be less than high freq");
			return -1;
		}

		RemoveFilters();
	
		double ear_q, min_bandwidth, ear_q_min_bw;
		GetEarModel(&ear_q, &min_bandwidth, &ear_q_min_bw, _earModel);
	
		double num_filters;
		
		num_filters = -(ear_q * log(_lowFreq	+ ear_q_min_bw)) / stepfactor;
		num_filters += (ear_q * log(_highFreq + ear_q_min_bw)) / stepfactor;
	
		//round to 2 decimal places to get rid of small numerical errors
		num_filters = round(num_filters * 100) * 0.01;
		num_filters = ceil(num_filters);
	
		for(int i = 0; i <= num_filters; i++)
		{
			double denominator = exp(i * stepfactor / ear_q);
			double center_freq = -ear_q_min_bw;
			center_freq += (_highFreq + ear_q_min_bw) / denominator;
			double bandwidth = GetERBOfHumanAuditoryFilter(center_freq, _earModel);
			shared_ptr<Common::CGammatoneFilter> filter = AddFilter(GAMMATONE_FILTER_ORDER, center_freq, bandwidth);
			filter->SetSamplingFreq(this->GetSamplingFreq());
		}

		return num_filters;
	}

	//////////////////////////////////////////////
	double CGammatoneFilterBank::InitWithFreqRangeNumFilters(double _lowFreq, double _highFreq, unsigned _numFilters, T_earModel _earModel)
	{
		double ear_q, min_bandwidth, ear_q_min_bw;
		GetEarModel(&ear_q, &min_bandwidth, &ear_q_min_bw, _earModel);
	
		_numFilters -= 1; //there will be an extra one at the nyquist freq
		double stepfactor = log(_highFreq + ear_q_min_bw) - log(_lowFreq + ear_q_min_bw);
		stepfactor *= ear_q / _numFilters;
		double overlap = 1-stepfactor;
		InitWithFreqRangeOverlap(_lowFreq, _highFreq, overlap, _earModel);
		return overlap;
	}

	//////////////////////////////////////////////
	double CGammatoneFilterBank::GetERBOfHumanAuditoryFilter(double _centerFreq, T_earModel _earModel)
	{
		double ear_q, min_bandwidth, ignored;
		GetEarModel(&ear_q, &min_bandwidth, &ignored, _earModel);
		return (_centerFreq / ear_q) + min_bandwidth;
	}

	//////////////////////////////////////////////
	double CGammatoneFilterBank::SetFreqBandwidthOfFilter(int _index, double _centerFreq, T_earModel _earModel)
	{
		double erb = GetERBOfHumanAuditoryFilter(_centerFreq, _earModel);
		shared_ptr<Common::CGammatoneFilter> filter = GetFilter(_index);
		if(filter != nullptr)
		{
			filter->SetCenterFrequency(_centerFreq);
			filter->SetERBBandwidth(erb);
		}
		return erb;
	}

	//////////////////////////////////////////////
	void CGammatoneFilterBank::GetEarModel(double* ear_q, double* min_bandwidth, double* ear_q_min_bw, T_earModel _earModel)
	{
		switch(_earModel)
		{
			case EAR_MODEL_GREENWOOD:
				*ear_q = 7.23824;
				*min_bandwidth = 22.8509;
				break;
			case EAR_MODEL_LYON:
				*ear_q = 8;
				*min_bandwidth = 125;
				break;
			case EAR_MODEL_GLASBERG: /* cascade */
			default:
				*ear_q = 9.26449;
				*min_bandwidth = 24.7;
				break;
		}
		*ear_q_min_bw = *ear_q * *min_bandwidth;
	}
	
	//////////////////////////////////////////////
	shared_ptr<Common::CGammatoneFilter> CGammatoneFilterBank::AddFilter(unsigned _order, double _freq, double _erb)
	{
		try
		{
			shared_ptr<Common::CGammatoneFilter> newFilter(new Common::CGammatoneFilter(_order, _freq, _erb));
			newFilter->SetSamplingFreq(this->GetSamplingFreq());
			filters.insert(filters.begin(), newFilter);

			SET_RESULT(RESULT_OK, "Filter added to filter bank succesfully");
			return newFilter;
		}
		catch (std::bad_alloc& ba)
		{
			SET_RESULT(RESULT_ERROR_BADALLOC, ba.what());
			//ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
			return nullptr;
		}
	}

	//////////////////////////////////////////////
	shared_ptr<Common::CGammatoneFilter> CGammatoneFilterBank::GetFilter(int _index)
	{
		if (_index < 0 || filters.size() <= _index)
		{
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Attempt to get a filter from filter bank outside bank size");
			return nullptr;
		}
		else
		{
			SET_RESULT(RESULT_OK, "Succesfully got filter from filter bank");
			return filters[_index];
		}
	}

	//////////////////////////////////////////////
	void CGammatoneFilterBank::RemoveFilters()
	{
		//pthread lock
		//todo: what if the Process() thread is using a filter when it is removed?
		filters.clear();
		//pthread unlock
		SET_RESULT(RESULT_OK, "All filters succesfully removed from filter bank");
	}

	//////////////////////////////////////////////
	int CGammatoneFilterBank::GetNumFilters()
	{
		return filters.size();
	}

	//////////////////////////////////////////////
	void CGammatoneFilterBank::SetSamplingFreq(float _samplingFreq)
	{
		if (_samplingFreq < 0.1)
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling frequency for gammatone filter is invalid");
			return;
		}

		samplingFreq = _samplingFreq;
	
		for (std::size_t c = 0; c < filters.size(); c++)
		{
			shared_ptr<Common::CGammatoneFilter> f = filters[c];

			if (f != nullptr)
				f->SetSamplingFreq(_samplingFreq);
		}
	}

	//////////////////////////////////////////////
	float CGammatoneFilterBank::GetSamplingFreq()
	{
		return samplingFreq;
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilterBank::Process(CMonoBuffer<float> & inBuffer, CMonoBuffer<float> & outBuffer)
	{
		int size = inBuffer.size();


		ASSERT(size > 0, RESULT_ERROR_BADSIZE, "Attempt to process a filter bank with an empty input buffer", "");
		ASSERT(size == outBuffer.size(), RESULT_ERROR_BADSIZE, "Attempt to process a filter bank with different sizes for input and output buffers", "");
		SET_RESULT(RESULT_OK, "");

		bool addResult = false;

		//pthread lock
		for (std::size_t c = 0; c < filters.size(); c++)
		{
			shared_ptr<Common::CGammatoneFilter> f = filters[c];

			if (f != nullptr)
			{
				f->Process(inBuffer, outBuffer, addResult);
				addResult = true;
			}
		}
		//pthread unlock
	}
}// end namespace Common
