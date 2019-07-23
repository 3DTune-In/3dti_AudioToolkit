/*
* \class CGammatoneFilter
*
* \brief Definition of GammatoneFilter class.
*
* Class to implement a Gammatone Digital Filter.
* Equation numnber throughout this code refer to the following paper:
* Implementing a GammaTone Filter Bank
* Annex C of the SVOS Final Report (Part A: The Auditory Filter Bank)
* John Holdsworth, Ian Nimmo-Smith, Roy Patterson, Peter Rice
* 26th February 1988
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre, D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions: (additional authors/contributors can be added here)
* \b The gammatone filter was implemented by Michael Krzyzaniak: m.krzyzaniak@surrey.ac.uk
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

#define _USE_MATH_DEFINES // TODO: Test in windows! Might also be problematic for other platforms??
#include <cmath>
#include <Common/GammatoneFilter.h>
#include <iomanip>

#ifndef M_PI 
#define M_PI 3.1415926535 
#endif

#ifndef M_TWO_PI
#define M_TWO_PI (2 * M_PI)
#endif

#define MAX_ORDER 30
#define DEFAULT_SAMPLING_RATE 44100
#define DEFAULT_SIN_FUNCTION sin
#define DEFAULT_COS_FUNCTION cos
#define DEFAULT_SQRT_FUNCTION sqrt
#define DEFAULT_POW_FUNCTION pow
#define DEFAULT_EXP_FUNCTION exp

namespace Common {

	//////////////////////////////////////////////
	CGammatoneFilter::CGammatoneFilter(unsigned _order, float _centerFrequency, float _erbBandwidth)
	{
		if ((_order < 1) || (_order > MAX_ORDER))
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Gammatone filter needs to be constructed with a order greater than 0 (try 4 for modelling the basilar membrane)");
			return;
		}
	
		order = _order;
		prev_z_real = new float[order]();
		prev_z_imag = new float[order]();
		prev_w_real = new float[order]();
		prev_w_imag = new float[order]();
	
		if(!(prev_z_real && prev_z_imag && prev_w_real && prev_w_imag))
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "stack overflow while allocating gammatone filter");
			return;
		}

		an = CalculateAn(order);
		cn = CalculateCn(order);
		sin_phase = 0;
		cos_phase = 1;
		generalGain = 1.0f;

		// error handler: these will set success or failure
		SetSamplingFreq(DEFAULT_SAMPLING_RATE);
		SetCenterFrequency(_centerFrequency);
		SetERBBandwidth(_erbBandwidth);
	}

	//////////////////////////////////////////////
	CGammatoneFilter::~CGammatoneFilter()
	{
		if(prev_z_real) delete[] prev_z_real;
		if(prev_z_imag) delete[] prev_z_imag;
		if(prev_w_real) delete[] prev_w_real;
		if(prev_w_imag) delete[] prev_w_imag;
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilter::Process(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> &outBuffer, bool addResult)
	{
		int size = inBuffer.size();

		if (size <= 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a Gammatone filter with an empty input buffer");
			return;
		}

		//todo: implement data-doubling to avoid ailising for high freqs > 11kHz
		float w_real, w_imag, z_real, z_imag, sample;
		double temp;
		double constant = this->equation_11_constant;
		int k_index;

#if defined CGAMMATONE_USE_ANTIAILIAING
		size <<= 1;
#endif

		for (int k = 0; k < size; k++)
		{
			k_index = k;
#if defined CGAMMATONE_USE_ANTIAILIAING
			/*upsample by repeating each input value*/
			k_index >>= 1;
#endif
			//eq. 9 frequency shifting by -f0 Hz
			z_real =  this->cos_phase * inBuffer[k_index];
			z_imag = -this->sin_phase * inBuffer[k_index];

			//eq. 10 recursive 1st order filter
			for (int n = 0; n < this->order; n++)
			{
				//last parenthesis
				w_real = this->prev_z_real[n] - this->prev_w_real[n];
				w_imag = this->prev_z_imag[n] - this->prev_w_imag[n];

				//second term
				w_real *= constant;
				w_imag *= constant;

				//first term
				w_real += this->prev_w_real[n];
				w_imag += this->prev_w_imag[n];

				this->prev_z_real[n] = z_real;
				this->prev_z_imag[n] = z_imag;
				this->prev_w_real[n] = z_real = w_real;
				this->prev_w_imag[n] = z_imag = w_imag;
			}

#if defined CGAMMATONE_USE_ANTIAILIAING
			/*downsample by decimation*/
			if(k & 0x01)
#endif
			{
			    sample = this->cos_phase*z_real - this->sin_phase*z_imag;
			    sample *= this->generalGain;
			    outBuffer[k_index] = (addResult) ? (outBuffer[k_index] + sample) : sample;
			}
    
			temp = this->cos_phase;
			this->cos_phase = this->cos_phase_increment * this->cos_phase + this->sin_phase_increment * this->sin_phase;
			this->sin_phase = this->cos_phase_increment * this->sin_phase - this->sin_phase_increment * temp;
		}
	}

	//////////////////////////////////////////////
	void CGammatoneFilter::Process(CMonoBuffer<float> &buffer)
	{
		Process(buffer, buffer, false);
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilter::SetSamplingFreq(float _samplingFreq)
	{
		if (_samplingFreq < 0.1)
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling frequency for gammatone filter is invalid");
			return;
		}

		SET_RESULT(RESULT_OK, "Sampling frequency for gammatone filter succesfully set");
#if defined CGAMMATONE_USE_ANTIAILIAING
		samplingFreq = _samplingFreq * 2;
#else
		samplingFreq = _samplingFreq;
#endif
		UpdateEq11Constant();
		UpdatePhaseIncrement();
	}

	//////////////////////////////////////////////
	float CGammatoneFilter::GetSamplingFreq()
	{
#if defined CGAMMATONE_USE_ANTIAILIAING
		return samplingFreq * 0.5;
#else
		return samplingFreq;
#endif
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilter::SetGeneralGain(float _gain)
	{
		generalGain = _gain;
	}

	//////////////////////////////////////////////
	float CGammatoneFilter::GetGeneralGain()
	{
		return generalGain;
	}
 
	//////////////////////////////////////////////
	unsigned CGammatoneFilter::GetOrder()
	{
		return order;
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilter::Set3dBBandwidth(float _bw)
	{
		b = _bw / cn;
		UpdateEq11Constant();
	}
	
	//////////////////////////////////////////////
	float CGammatoneFilter::Get3dBBandwidth()
	{
		return cn * b;
	}

	//////////////////////////////////////////////
	void CGammatoneFilter::SetERBBandwidth(float _erb)
	{
		b = _erb / an;
		UpdateEq11Constant();
	}
	
	//////////////////////////////////////////////
	float CGammatoneFilter::GetERBBandwidth()
	{
		return an * b;
	}

	//////////////////////////////////////////////
	void CGammatoneFilter::SetCenterFrequency(float _freq)
	{
		f0 = _freq;
		UpdatePhaseIncrement();
	}
	
	//////////////////////////////////////////////
	float CGammatoneFilter::GetCenterFrequency()
	{
		return f0;
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilter::UpdateEq11Constant()
	{
		equation_11_constant = 1.0 - DEFAULT_EXP_FUNCTION(-M_TWO_PI * b / samplingFreq);
	}

	//////////////////////////////////////////////
	void CGammatoneFilter::UpdatePhaseIncrement()
	{
		double phase_increment = f0 * M_TWO_PI / samplingFreq;
		cos_phase_increment = DEFAULT_COS_FUNCTION(phase_increment);
		sin_phase_increment = DEFAULT_SIN_FUNCTION(phase_increment);
	}

	//////////////////////////////////////////////
	double CGammatoneFilter::CalculateAn(unsigned _order)
	{
		if ((_order < 1) || (_order > MAX_ORDER))
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Calculate filter An -- an order out of bounds");
			return 1;
		}
		SET_RESULT(RESULT_OK, "Calculate filter An -- OK");
		
		//precomputed using python3's builtin bignums
		double an_table[] = 
		{
			0, //there is no zero order filter, but pad this for ease of access
			3.141592653589793,
			1.5707963267948966,
			1.1780972450961724,
			0.9817477042468103,
			0.8590292412159591, //notice there is a mistake in the paper here
			0.7731263170943632,
			0.7086991240031661,
			0.65807775800294,
			0.6169478981277563,
			0.5826730148984365,
			0.5535393641535147,
			0.5283784839647185,
			0.5063627137995219,
			0.48688722480723273,
			0.4694983953498315,
			0.45384844883817044,
			0.43966568481197754,
			0.4267343411410371,
			0.414880609442675,
			0.4039626986678677,
			0.393863631201171,
			0.38448592569638124,
			0.3757476092032817,
			0.3675791829162538,
			0.35992128327216516,
			0.35272285760672184,
			0.3459397257296695,
			0.33953343451245344,
			0.3334703374675882,
			0.3277208488905608,
		};
		
		return an_table[_order];
		
		////equation 6
		//long double two_n_minus_two     = (2.0 * _order) - 2.0;
		//long double two_n_minus_two_WOW = DEFAULT_FACTORIAL_FUNCTION(two_n_minus_two);
		//long double two_to_the_neg_x    = DEFAULT_POW_FUNCTION(2.0, -two_n_minus_two);
		//long double numerator           = M_PI * two_n_minus_two_WOW * two_to_the_neg_x;
		//long double denominator         = DEFAULT_FACTORIAL_FUNCTION(_order-1);
		//denominator *= denominator;
		//return numerator / denominator;
	}
	
	//////////////////////////////////////////////
	double CGammatoneFilter::CalculateCn(unsigned _order)
	{
		if (_order < 1)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "ERB of Gammatone filter needs an order greater than 0");
			return 1.0;
		}
		SET_RESULT(RESULT_OK, "Calculate c_n OK");
	
		//equation 7
		return 2.0 * DEFAULT_SQRT_FUNCTION(DEFAULT_POW_FUNCTION(2, 1.0/(double)_order) - 1.0);
	}
}
