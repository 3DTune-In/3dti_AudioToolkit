/*
* \class CDistanceAttenuator
*
* \brief Definition of CDistanceAttenuator class.
*
* Class to apply the effects of the distance of the sound source on the listener
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

#include <cmath>
#include <Common/DistanceAttenuator.h>
#include <Common/ErrorHandler.h>
#include <BinauralSpatializer/Core.h>

#define EPSILON_ATT 0.0001

//#define USE_PROFILER_DistanceAttenuator
#ifdef USE_PROFILER_DistanceAttenuator
#include <Common/Profiler.h>
CProfilerDataSet dsDAAttenuation;
#endif

namespace Common {

	CDistanceAttenuator::CDistanceAttenuator()
	{
		referenceDistance = DISTANCE_MODEL_THRESHOLD_NEAR;
		//Simple distance attenuation vrbles initialization

		previousAttenuation_Channel = 0.0f;

#ifdef USE_PROFILER_DistanceAttenuator	
		PROFILER3DTI.SetAutomaticWrite(dsDAAttenuation, "PROF_Distance_Attenuation.txt");
		PROFILER3DTI.StartRelativeSampling(dsDAAttenuation);
#endif
	}

	//////////////////////////////////////////////

	void CDistanceAttenuator::Process(CMonoBuffer<float> & buffer, float distance, float attenuationConstant, int bufferSize, int sampleRate, float extraAttennuation_dB)
	{
#ifdef USE_PROFILER_DistanceAttenuator
		PROFILER3DTI.RelativeSampleStart(dsDAAttenuation);
#endif

		// Attenuation is computed independently
		float attenuation = GetDistanceAttenuation(attenuationConstant, distance, extraAttennuation_dB);

		//Apply attenuation gradually using Exponential Moving Average method
		float unnecessary_fixme;
		if( buffer.size() != 0 ) buffer.ApplyGainExponentially(previousAttenuation_Channel, unnecessary_fixme, attenuation, bufferSize, sampleRate );

#ifdef USE_PROFILER_DistanceAttenuator
		PROFILER3DTI.RelativeSampleEnd(dsDAAttenuation);
#endif

	}//ApplyDistanceEffectsAux


	//////////////////////////////////////////////

#define FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB -6.0206f		///< std::log10(0.5f) * 20.0f
	float CDistanceAttenuator::GetDistanceAttenuation(float attForDuplicateDistance, float distance, float extraAttennuation_dB) const
	{
		// Error handler:
		if (distance <= 0.0f)
		{
			SET_RESULT(RESULT_ERROR_DIVBYZERO, "Attempt to compute distance attenuation for a negative or zero distance");
			return 1.0f;
		}
		//else
		//	SET_RESULT(RESULT_OK, "Distance attenuation returned succesfully for single source");	

		// Compute gain
		if (distance > EPSILON_DISTANCE && std::fabs(attForDuplicateDistance) > EPSILON_ATT)
		{
			// Compute attenuation factor
			float attenuationFactor = attForDuplicateDistance / FUNDAMENTAL_DISTANCE_ATTENUATION_REFERENCE_DB;

			return std::pow(10.0f, extraAttennuation_dB + attenuationFactor * std::log10(referenceDistance / distance));
		}
		else
			return 1.0;
	}

	void CDistanceAttenuator::SetReferenceDistance(float _referenceDistance)
	{
		referenceDistance = _referenceDistance;
	}

	float CDistanceAttenuator::GetReferenceDistance()
	{
		return referenceDistance;
	}

}//end namespace Common