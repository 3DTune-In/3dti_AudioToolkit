/**
* \class CFarDistanceEffects
*
* \brief Applies effects to audio signal for far distances
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
#include <Common/FarDistanceEffects.h>
#include <Common/ErrorHandler.h>
#include <BinauralSpatializer/Core.h>

// Defines the frequency at which audible frequencies are not attenuated in a low pass filter.
#define NO_FILTERING_CUT_OFF_FREQUENCY 20000

// Q for LPF
#define LPF_Q 1.414213562

#define NUM_OF_BIQUAD_FILTERS_FOR_FAR_DISTANCE_FILTERING 2

// The default function that provides the cutoff frequency that models the distortion of far sound sources
// follows this expression:   Fc = A · 10^-B(distance - C)
// The A and B constants are obtained from the following specifications:
//   Distance = 15m --> Fc = 20Khz
//   Distance = 50m --> Fc =  5Khz
// After solving an equations system:   Cutoff Frecuency = 20000 * pow(10, -0.0172 * (distance - 15));
#define CUT_OFF_FREQUENCY_FUNCTION__COEF_A NO_FILTERING_CUT_OFF_FREQUENCY
#define CUT_OFF_FREQUENCY_FUNCTION__COEF_B 0.0172
#define CUT_OFF_FREQUENCY_FUNCTION__COEF_C DISTANCE_MODEL_THRESHOLD_FAR

//#define USE_PROFILER_FarDistanceEffects
#ifdef USE_PROFILER_FarDistanceEffects
#include <Common/Profiler.h>
CProfilerDataSet dsDAFar;
#endif 

namespace Common {

	CFarDistanceEffects::CFarDistanceEffects()
	{

	}

	//////////////////////////////////////////////

	void CFarDistanceEffects::Setup(int samplingRate)
	{
#ifdef USE_PROFILER_FarDistanceEffects	
		PROFILER3DTI.SetAutomaticWrite(dsDAFar, "PROF_Distance_Far.txt");
		PROFILER3DTI.StartRelativeSampling(dsDAFar);
#endif

		for (int c = 0; c < NUM_OF_BIQUAD_FILTERS_FOR_FAR_DISTANCE_FILTERING; c++)
		{
			shared_ptr <CBiquadFilter> onefilter  = distanceFiltersChain.AddFilter();			
			onefilter ->SetSamplingFreq(samplingRate);			
			onefilter ->Setup(samplingRate, NO_FILTERING_CUT_OFF_FREQUENCY, LPF_Q, LOWPASS);			
		}
	}


	//////////////////////////////////////////////

	void CFarDistanceEffects::Process(CMonoBuffer<float> &bufferMono, float distance)
	{
		if (distance > DISTANCE_MODEL_THRESHOLD_FAR) // NOTE: This was already checked when called from CSingleSourceDSP, and will be checked again in CalculateCutoffFrequency
		{
#ifdef USE_PROFILER_FarDistanceEffects
			PROFILER3DTI.RelativeSampleStart(dsDAFar);
#endif
			float fc = CalculateCutoffFrequency(distance);

			for (int c = 0; c < NUM_OF_BIQUAD_FILTERS_FOR_FAR_DISTANCE_FILTERING; c++)
				distanceFiltersChain.GetFilter(c)->SetCoefficients(fc, LPF_Q, T_filterType::LOWPASS);

			if (bufferMono.size() != 0)
				distanceFiltersChain.Process(bufferMono);

#ifdef USE_PROFILER_FarDistanceEffects
			PROFILER3DTI.RelativeSampleEnd(dsDAFar);
#endif
		}
	}//Process
	//////////////////////////////////////////////

	// Returns the cutoff frequency of the low pass filters that is applied for long distances.
	float CFarDistanceEffects::CalculateCutoffFrequency(float distance)
	{
		//SET_RESULT(RESULT_OK, "");

		if (distance > DISTANCE_MODEL_THRESHOLD_FAR)	
		{
			/*
			// See comments in definition of constants CUT_OFF_FREQUENCY_FUNCTION__COEF_X
			float A = CUT_OFF_FREQUENCY_FUNCTION__COEF_A;
			float B = CUT_OFF_FREQUENCY_FUNCTION__COEF_B;
			float C = CUT_OFF_FREQUENCY_FUNCTION__COEF_C;

			return A * std::pow(10, -B * (distance - C));*/

			float c_pow = 2.0f, c_div = 7100.0f, ax = 100.0f, dmin = 15.0f, dmax = 100.0f;
			float b = exp(pow(dmax - dmin, c_pow) / c_div);
			if (distance > dmax) distance = dmax;
			return ((20000 / b) * exp((pow(dmax - distance, c_pow)) / c_div));


		}
		else
			return NO_FILTERING_CUT_OFF_FREQUENCY;
	}

}//end namespace common

