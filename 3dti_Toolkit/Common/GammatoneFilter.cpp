/*
* \class CGammatoneFilter
*
* \brief Definition of GammatoneFilter class.
*
* Class to implement a Biquad Digital Filter.
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
#include "GammatoneFilter.h"
#include <iomanip>

#ifndef M_PI 
#define M_PI 3.1415926535 
#endif

#ifndef M_TWO_PI
#define M_TWO_PI (2 * M_PI)
#endif

#define DEFAULT_SAMPLING_RATE 44100
#define DEFAULT_ORDER 4
#define DEFAULT_SIN_FUNCTION sin
#define DEFAULT_COS_FUNCTION cos
#define DEFAULT_SQRT_FUNCTION sqrt
#define DEFAULT_POW_FUNCTION pow
#define DEFAULT_EXP_FUNCTION exp
#define DEFAULT_FACTORIAL_FUNCTION Factorial

namespace Common {

	//////////////////////////////////////////////
	CGammatoneFilter::CGammatoneFilter(unsigned _order, float _centerFrequency)
	{
		//the upper bound is limited by the factorial function in the calculation of an.
		//an should be precomputed for, say 20 filter orders, and that should be the upper bound here
		if ((_order < 1) || (_order > 7))
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Gammatone filter needs to be constructed with a order greater than 0 (try 4 for modelling the basilar membrane)");
			return;
		}
	
		order = _order;
		an = CalculateAn(order);
		cn = CalculateCn(order);
	
		prev_z_real = new float[order]();
		prev_z_imag = new float[order]();
		prev_w_real = new float[order]();
		prev_w_imag = new float[order]();
		//todo: check that we got the memory without stack overflow?

		generalGain = 1.0f;

		phase = 0;
		// error handler: Trust in SetSamplingFreq for result
		SetSamplingFreq(DEFAULT_SAMPLING_RATE);
		SetFrequencyUsingERBOfHumanAuditoryFilter(_centerFrequency);
	}

	//////////////////////////////////////////////
	//void CGammatoneFilter::Process(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> & outBuffer, bool addResult)
	//{
	//	SET_RESULT(RESULT_ERROR_BADSIZE, "I haven't implemented this yet. Is it needed?");
	//}

	//////////////////////////////////////////////
	void CGammatoneFilter::Process(CMonoBuffer<float> &buffer)
	{
		int size = buffer.size();

		if (size <= 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with an empty input buffer");
			return;
		}

		//SET_RESULT(RESULT_OK, "Biquad filter process succesfull");

		//todo: make these instance variables and decleare them at filter creation time?
		//ok, but what if the buffer size changes?
		//CMonoBuffer<float> z_real(size);
		//CMonoBuffer<float> z_imag(size);
		float* z_real = new float[size]();
		float* z_imag = new float[size]();
		//todo: check that we got the memory without stack overflow?
	
		//todo: implement data-doubling to avoid ailising for high freqs > 11 kHz
		double phase_increment = this->f0 * M_TWO_PI / this->samplingFreq;
	
		//constant 1-e^(-2PI*b*t), for middle parenthesis term of eq 10
		//could be calculated when bandwidth or samplingFreq are calculated
		double constant = 1.0 - DEFAULT_EXP_FUNCTION(-M_TWO_PI * this->b / this->samplingFreq);
	
		//extra storage for intermediate computations
		float w_real, w_imag, sin_phase, cos_phase;
	
		for (int k = 0; k < size; k++)
		{
			cos_phase = DEFAULT_COS_FUNCTION(this->phase);
			sin_phase = DEFAULT_SIN_FUNCTION(this->phase);
			//eq. 9 frequency shifting by -f0 Hz
			z_real[k] = cos_phase * buffer[k];
			z_imag[k] = -sin_phase * buffer[k];

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
				
				this->prev_z_real[n] = z_real[k];
				this->prev_z_imag[n] = z_imag[k];
				this->prev_w_real[n] = z_real[k] = w_real;
				this->prev_w_imag[n] = z_imag[k] = w_imag;
				z_real[k] = w_real;
				z_imag[k] = w_imag;
			}
	 
			//equation 11 frequency shifting by +f0 Hz
			buffer[k] = cos_phase*z_real[k] - sin_phase*z_imag[k];
			buffer[k] *= this->generalGain;
		
			this->phase += phase_increment;
			if(this->phase > M_TWO_PI)
				this->phase -= M_TWO_PI;
		}
		//todo: this delayed the signal by half of the sampling period...
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
		samplingFreq = _samplingFreq;
	}

	//////////////////////////////////////////////
	float CGammatoneFilter::GetSamplingFreq()
	{
		return samplingFreq;
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
	}
	
	//////////////////////////////////////////////
	float CGammatoneFilter::GetCenterFrequency()
	{
		return f0;
	}
	
	//////////////////////////////////////////////
	void CGammatoneFilter::SetFrequencyUsingERBOfHumanAuditoryFilter(float _freq)
	{
		float erb = GetERBOfHumanAuditoryFilter(_freq);
		SetCenterFrequency(_freq);
		SetERBBandwidth(erb);
	}
	
	//////////////////////////////////////////////
	float CGammatoneFilter::GetERBOfHumanAuditoryFilter(float _freq)
	{
		float result = 0;
	
		/* equation 7 */
		/* this is supposedly valid from 100 to 10000 Hz. I don't know above 10000Hz */
		/* NB, Wikipedia s.v. "ERB" give the wrong coefficients (missing order of magnitude terms) */
		if(_freq > 100)
			result = (6.23e-6 * _freq * _freq) + (93.39e-3 * _freq) + 28.52;
	
		/* this is supposedly valid for frequencies ~ 0.1 to 10 Hz. I don't know from 10 to 100 Hz */
		//todo: check the coefficients against Moore and Glassberg 1983
		//B.C.J. Moore and B.R. Glasberg, "Suggested formulae for calculating auditory-filter bandwidths and excitation patterns" Journal of the Acoustical Society of America 74: 750-753, 1983
		else
			result = 24.7 * (4.37*_freq+1);
	
		return result;
	}
	
	//////////////////////////////////////////////
	//constructor ensures that x (filter order) is 1 to 12
	unsigned CGammatoneFilter::Factorial(unsigned x)
	{
		unsigned result = 1;
	
		while(x > 1)
			result *= x--;
	
		return result;
	}
	
	//////////////////////////////////////////////
	double CGammatoneFilter::CalculateAn(unsigned _order)
	{
		if (_order < 1)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "ERB of Gammatone filter needs an order greater than 0");
			return 1;
		}
		SET_RESULT(RESULT_OK, "OrderToEquivalentRectangularBandwidth OK");
	
		//equation 6
		//todo: precompute an using bignums, and here just look it up in a table and return it.
		long double two_n_minus_two	 = (2.0 * _order) - 2.0;
		long double two_n_minus_two_WOW = DEFAULT_FACTORIAL_FUNCTION(two_n_minus_two);
		long double two_to_the_neg_x		= DEFAULT_POW_FUNCTION(2.0, -two_n_minus_two);
		long double numerator			= M_PI * two_n_minus_two_WOW * two_to_the_neg_x;
		long double denominator		 = DEFAULT_FACTORIAL_FUNCTION(_order-1);
		denominator *= denominator;
	
		if(denominator == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Numerical error while setting bandwidth of gammatone filter (is the filter order too high?)");
			return 1.0;
		}
	
		return numerator / denominator;
	}
	
	//////////////////////////////////////////////
	double CGammatoneFilter::CalculateCn(unsigned _order)
	{
		if (_order < 1)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "ERB of Gammatone filter needs an order greater than 0");
			return 1.0;
		}
		SET_RESULT(RESULT_OK, "OrderToEquivalentRectangularBandwidth OK");
	
		//equation 7
		return 2.0 * DEFAULT_SQRT_FUNCTION(DEFAULT_POW_FUNCTION(2, 1.0/(double)_order) - 1.0);
	}
}
