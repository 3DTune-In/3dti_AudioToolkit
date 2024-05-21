/*
* \class CBiquadFilter
*
* \brief Definition of BiquadFilter class.
*
* Class to implement a Biquad Digital Filter
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

#define _USE_MATH_DEFINES // TODO: Test in windows! Might also be problematic for other platforms??
#include <cmath>
#include <Common/BiquadFilter.h>
#include <iomanip>

#ifndef M_PI 
#define M_PI 3.1415926535 
#endif

#define DEFAULT_SAMPLING_RATE 44100

namespace Common {
	//////////////////////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR

	CBiquadFilter::CBiquadFilter()
	{
		// error handler: Trust in SetSamplingFreq for result

		z1_l = 0;
		z2_l = 0;
		z1_r = 0;
		z2_r = 0;

		b0 = 1;
		b1 = 0;
		b2 = 0;
		a1 = 0;
		a2 = 0;

		new_b0 = 1;
		new_b1 = 0;
		new_b2 = 0;
		new_a1 = 0;
		new_a2 = 0;
		crossfadingNeeded = false;
		new_z1_l = 0;
		new_z2_l = 0;
		new_z1_r = 0;
		new_z2_r = 0;

		SetCoefficients(1, 0, 0, 0, 0);

		generalGain = 1.0f;

		SetSamplingFreq(DEFAULT_SAMPLING_RATE);
	}

	//////////////////////////////////////////////

	void CBiquadFilter::Setup(float samplingRate, float _b0, float _b1, float _b2, float _a1, float _a2)
	{
		samplingFreq = samplingRate;
		SetCoefficients(_b0, _b1, _b2, _a1, _a2);
	}

	//////////////////////////////////////////////

	void CBiquadFilter::Setup(float samplingRate, float frequency, float Q, T_filterType filterType, float gain)
	{
		samplingFreq = samplingRate;
		SetCoefficients(frequency, Q, filterType, gain);
	}

	//////////////////////////////////////////////

	void CBiquadFilter::SetCoefficients(float _b0, float _b1, float _b2, float _a1, float _a2)
	{
		crossfadingNeeded = true;

		new_b0 = _b0;
		new_b1 = _b1;
		new_b2 = _b2;
		new_a1 = _a1;
		new_a2 = _a2;

		new_z1_l = 0;
		new_z2_l = 0;
		new_z1_r = 0;
		new_z2_r = 0;
	}

	//////////////////////////////////////////////

	void CBiquadFilter::SetCoefficients(float *coefficients)
	{
		//SET_RESULT(RESULT_OK, "");
		SetCoefficients(coefficients[0], coefficients[1], coefficients[2], coefficients[3], coefficients[4]);
	}

	//void CBiquadFilter::SetCoefficients(std::vector<float>& coefficients)
	void CBiquadFilter::SetCoefficients(TBiquadCoefficients& coefficients)
	{
		//SET_RESULT(RESULT_OK, "");
		SetCoefficients(coefficients[0], coefficients[1], coefficients[2], coefficients[3], coefficients[4]);
	}

	//////////////////////////////////////////////

	void CBiquadFilter::SetCoefficients(float frequency, float Q, T_filterType filterType, float gain)
	{
		if (filterType == LOWPASS) {
			SetGeneralGain(gain);
			SetCoefsFor_LPF(frequency, Q);
		}
		else if (filterType == HIGHPASS) {
			SetGeneralGain(gain);
			SetCoefsFor_HPF(frequency, Q);
		}
		else if (filterType == BANDPASS) {
			SetGeneralGain(gain);
			SetCoefsFor_BandPassFilter(frequency, Q);
		}
		else if (filterType == LOWSHELF) {
			// Not implemented
            SET_RESULT(RESULT_ERROR_NOTIMPLEMENTED, "Lowshelf filter type not implemented");
		}
		else if (filterType == HIGHSHELF) {
			// Not implemented
            SET_RESULT(RESULT_ERROR_NOTIMPLEMENTED, "Highshelf filter type not implemented");
		}
		else if (filterType == PEAKNOTCH) {
			// Not implemented
            SET_RESULT(RESULT_ERROR_NOTIMPLEMENTED, "Peak Notch filter type not implemented");
		}
		else {
            SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Invalid filter type");
		}
	}

	//////////////////////////////////////////////
	void CBiquadFilter::SetSamplingFreq(float _samplingFreq)
	{
		if (_samplingFreq < 0.1)
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling frequency for biquad filter is invalid");
			return;
		}

		SET_RESULT(RESULT_OK, "Sampling frequency for biquad filter succesfully set");
		samplingFreq = _samplingFreq;
	}

	//////////////////////////////////////////////
	bool CBiquadFilter::SetCoefsFor_BandPassFilter(double centerFreqHz, double Q)
	{
		if (samplingFreq < 0.1 || Q < 0.0000001 || centerFreqHz > samplingFreq / 2.0) // To prevent aliasing problems
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Cutoff frequency of biquad (bandpass) filter is higher than Nyquist frequency");
			return false;
		}

		try // -> To handle division by 0
		{
			double K = std::tan(M_PI * centerFreqHz / samplingFreq);

			double norm = 1 / (1 + K / Q + K * K);
			double _b0 = K / Q * norm;
			double _b1 = 0;
			double _b2 = -_b0;
			double _a1 = 2 * (K * K - 1) * norm;
			double _a2 = (1 - K / Q + K * K) * norm;

			SetCoefficients(_b0, _b1, _b2, _a1, _a2);

			SET_RESULT(RESULT_OK, "Bandpass filter coefficients of biquad filter succesfully set");

			return true;
		}
		catch (exception e)
		{
			//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "");
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Division by zero setting coefficients for bandpass biquad filter");
			return false;
		}
	}

	//////////////////////////////////////////////
	bool CBiquadFilter::SetCoefsFor_LPF(double cutoffFreq, double Q)
	{
		if (samplingFreq < 0.1 || cutoffFreq > samplingFreq / 2.0) // To prevent aliasing problems
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Cutoff frequency of biquad (LPF) filter is higher than Nyquist frequency");
			return false;
		}

		try // -> To handle division by 0
		{
			double K = std::tan(M_PI * cutoffFreq / samplingFreq);

			double norm = 1 / (1 + K / Q + K * K);
			double _b0 = K * K * norm;
			double _b1 = 2 * _b0;
			double _b2 = _b0;
			double _a1 = 2 * (K * K - 1) * norm;
			double _a2 = (1 - K / Q + K * K) * norm;

			SetCoefficients(_b0, _b1, _b2, _a1, _a2);

			//SET_RESULT(RESULT_OK, "LPF filter coefficients of biquad filter succesfully set");

			return true;
		}
		catch (exception e)
		{
			//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "");
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Division by zero setting coefficients for LPF biquad filter");
			return false;
		}
	}

	//////////////////////////////////////////////
	bool CBiquadFilter::SetCoefsFor_HPF(double cutoffFreq, double Q)
	{
		if (samplingFreq < 0.1 || cutoffFreq > samplingFreq / 2.0) // To prevent aliasing problems
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Cutoff frequency of biquad (HPF) filter is higher than Nyquist frequency");
			return false;
		}

		try // -> To handle division by 0
		{
			double K = std::tan(M_PI * cutoffFreq / samplingFreq);

			double norm = 1 / (1 + K / Q + K * K);
			double _b0 = 1 * norm;
			double _b1 = -2 * _b0;
			double _b2 = _b0;
			double _a1 = 2 * (K * K - 1) * norm;
			double _a2 = (1 - K / Q + K * K) * norm;

			SetCoefficients(_b0, _b1, _b2, _a1, _a2);

			SET_RESULT(RESULT_OK, "HPF filter coefficients of biquad filter succesfully set");

			return true;
		}
		catch (exception e)
		{
			//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "");
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Division by zero setting coefficients for HPF biquad filter");
			return false;
		}
	}

	//////////////////////////////////////////////
	bool CBiquadFilter::SetCoefsFor_PeakNotch(double centerFreqHz, double Q)
	{
		// Use Vällimäki's method to calculate the coefficients of a peak-notch filter


	}
	
	//////////////////////////////////////////////
	bool CBiquadFilter::SetCoefsFor_LowShelf(double cutoffFreq, double Q)
	{
		// Not implemented
	}

	//////////////////////////////////////////////
	bool CBiquadFilter::SetCoefsFor_HighShelf(double cutoffFreq, double Q)
	{
		// Not implemented
	}


	//////////////////////////////////////////////
	void CBiquadFilter::Process(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> & outBuffer, bool addResult)
	{
		int size = inBuffer.size();

		if (size <= 0)
		{
			//SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The input buffer is empty");
			SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with an empty input buffer");
			return;
		}
		else if (size != outBuffer.size())
		{
			//SET_RESULT( RESULT_ERROR_INVALID_PARAM, "Input and output buffers size must agree" );
			SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with different sizes for input and output buffers");
			return;
		}

		//SET_RESULT(RESULT_OK, "");

		// This is expression of the biquad filter but the implementation follows a more efficient
		// approach in which only 2 delays cells are used.
		//   See schemes in: https://en.wikipedia.org/wiki/Digital_biquad_filter
		//   y(n) = b0.x(n) + b1.x(n-1) + b2.x(n-2) + a1.y(n-1) + a2.y(n-2) 	

		if (crossfadingNeeded && size > 0)  // size > 1 to avoid division by zero if size were 1 while calculating alpha
		{
			for (int c = 0; c < size; c++)
			{
				// To ensure alpha is in [0,1] we use -2 because the buffer is stereo
				double alpha = ((double)c) / ((double)(size - 1));

				double     sample = ProcessSample(inBuffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
				double new_sample = ProcessSample(inBuffer[c], new_a1, new_a2, new_b0, new_b1, new_b2, new_z1_l, new_z2_l);

				double res = sample * (1.0 - alpha) + new_sample * alpha;

				outBuffer[c] = addResult ? outBuffer[c] + res : res;
			}

			UpdateAttributesAfterCrossfading();
		}
		else
		{
			for (int c = 0; c < size; c++)
			{
				double res = ProcessSample(inBuffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
				outBuffer[c] = addResult ? outBuffer[c] + res : res;
			}
		}
		AvoidNanValues();
	}
	//////////////////////////////////////////////
	void CBiquadFilter::Process(CMonoBuffer<float> &buffer)
	{
		int size = buffer.size();

		if (size <= 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Attempt to process a biquad filter with an empty input buffer");
			return;
		}

		//SET_RESULT(RESULT_OK, "Biquad filter process succesfull");

		if (crossfadingNeeded)
		{
			for (int c = 0; c < size; c++)
			{
				double alpha = ((double)c) / ((double)(size - 1));

				double     sample = ProcessSample(buffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
				double new_sample = ProcessSample(buffer[c], new_a1, new_a2, new_b0, new_b1, new_b2, new_z1_l, new_z2_l);

				buffer[c] = sample * (1.0 - alpha) + new_sample * alpha;
			}

			UpdateAttributesAfterCrossfading();
		}
		else
		{
			for (int c = 0; c < size; c++)
				buffer[c] = ProcessSample(buffer[c], a1, a2, b0, b1, b2, z1_l, z2_l);
		}

		AvoidNanValues();
	}
	//////////////////////////////////////////////
	void CBiquadFilter::AvoidNanValues()
	{
		// FIXME: IIRs filters can eventually end up in a non stable state that can lead the filter output
		//      to +/-Inf. To prevent this situation we reset the delay cells of the filter when this happens.
		//    A known scenario in whinch this happens is this: In the binaural test app when two sound sources
		//   are played at the same time using anechoic an reverb and one of the sourcers is moved beyond the 
		//   far distances threshold, the LPF of the distances can end up with this unstable state.

		if (std::isnan(z1_l)) z1_l = 0;
		if (std::isnan(z2_l)) z2_l = 0;
		if (std::isnan(z1_r)) z1_r = 0;
		if (std::isnan(z2_r)) z2_r = 0;

		if (std::isnan(new_z1_l)) new_z1_l = 0;
		if (std::isnan(new_z2_l)) new_z2_l = 0;
		if (std::isnan(new_z1_r)) new_z1_r = 0;
		if (std::isnan(new_z2_r)) new_z2_r = 0;
	}

	//////////////////////////////////////////////
	double CBiquadFilter::ProcessSample(const double sample, const double _a1, const double _a2, const double _b0, const double _b1, const double _b2, double &z1, double &z2)
	{
		// This is expression of the biquad filter but the implementation follows a more efficient
		// approach in which only 2 delays cells are used.
		//   See schemes in: https://en.wikipedia.org/wiki/Digital_biquad_filter
		//   y(n) = b0.x(n) + b1.x(n-1) + b2.x(n-2) + a1.y(n-1) + a2.y(n-2) 	

		double m_l = sample - _a1 * z1 - _a2 * z2;
		double res = generalGain * (float)(_b0 * m_l + _b1 * z1 + _b2 * z2);
		z2 = z1;
		z1 = m_l;
		return res;
	}
	//////////////////////////////////////////////
	void CBiquadFilter::UpdateAttributesAfterCrossfading()
	{
		crossfadingNeeded = false;

		z1_l = new_z1_l;
		z2_l = new_z2_l;
		z1_r = new_z1_r;
		z2_r = new_z2_r;

		b0 = new_b0;
		b1 = new_b1;
		b2 = new_b2;
		a1 = new_a1;
		a2 = new_a2;
	}

	//////////////////////////////////////////////
	void CBiquadFilter::SetGeneralGain(float _gain)
	{
		generalGain = _gain;
	}

	//////////////////////////////////////////////
	float CBiquadFilter::GetGeneralGain()
	{
		return generalGain;
	}
}
