/*
	* \class CMagnitudes
	*
	* \brief Definition of a class for handling physical magnitudes.
	*
	* This class define the vars and methods for handling physical magnitudes.
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
#include <Common/Magnitudes.h>
#include <Common/ErrorHandler.h>

#include <cmath>

#define DEFAULT_REVERB_ATTENUATION_DB -3.01f				///< Default reverb attenuation with distance, in decibels
#define DEFAULT_ANECHOIC_ATTENUATION_DB	-6.0206f			///< log10f(0.5f) * 20.0f Default anechoic attenuation with distance, in decibels 

namespace Common {
	////////////////////////////////////////////////
	// CONSTRUCTOR/DESTRUCTOR

	CMagnitudes::CMagnitudes()
	{
		SetAnechoicDistanceAttenuation(DEFAULT_ANECHOIC_ATTENUATION_DB);
		SetReverbDistanceAttenuation(DEFAULT_REVERB_ATTENUATION_DB);
		SetSoundSpeed(DEFAULT_SOUND_SPEED);
	}

	////////////////////////////////////////////////
	// GET/SET METHODS	

		// Set sound speed in m/s
	void CMagnitudes::SetSoundSpeed(float _soundSpeed)
	{
		if (_soundSpeed < 0.0f)
		{
			SET_RESULT(RESULT_ERROR_PHYSICS, "Sound speed must be a positive value");
			return;
		}

		soundSpeed = _soundSpeed;

		//SET_RESULT(RESULT_OK, "Sound speed succesfully set");

	}

	////////////////////////////////////////////////

		// Get sound speed in m/s
	float CMagnitudes::GetSoundSpeed() const
	{
		return soundSpeed;
	}

	////////////////////////////////////////////////

		// Set distance attenuation constant for anechoic process 
	void CMagnitudes::SetAnechoicDistanceAttenuation(float _anechoicAttenuationDB)
	{
		//SetAttenuation(anechoicAttenuation, anechoicAttenuationDB, anechoicAttenuationGAIN, units);
		if (_anechoicAttenuationDB > 0.0f)
		{
			SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation constant in decibels must be a negative value");
			return;
		}
		anechoicAttenuationDB = _anechoicAttenuationDB;

		//SET_RESULT(RESULT_OK, "Anechoic distance attenuation succesfully set");
	}

	////////////////////////////////////////////////

		// Set distance attenuation constant for reverb process 
	void CMagnitudes::SetReverbDistanceAttenuation(float _reverbAttenuationDB)
	{
		//SetAttenuation(reverbAttenuation, reverbAttenuationDB, reverbAttenuationGAIN, units);
		if (_reverbAttenuationDB > 0.0f)
		{
			SET_RESULT(RESULT_ERROR_PHYSICS, "Attenuation constant in decibels must be a negative value");
			return;
		}
		reverbAttenuationDB = _reverbAttenuationDB;

		//SET_RESULT(RESULT_OK, "Reverb distance attenuation succesfully set");
	}

	////////////////////////////////////////////////

		// Get distance attenuation constant for anechoic process
		//float CMagnitudes::GetAnechoicDistanceAttenuation(TUnits units)
	float CMagnitudes::GetAnechoicDistanceAttenuation() const
	{
		return anechoicAttenuationDB;
	}
	////////////////////////////////////////////////
		// Get distance attenuation constant for anechoic process 	
		//float CMagnitudes::GetReverbDistanceAttenuation(TUnits units)	
	float CMagnitudes::GetReverbDistanceAttenuation() const
	{
		return reverbAttenuationDB;
	}

	bool CMagnitudes::AreSame(float a, float b, float epsilon)
	{
		float absA = fabs(a);
		float absB = fabs(b);
		float diff = fabs(a - b);

		return diff < epsilon;
	}

}//end namespace Common