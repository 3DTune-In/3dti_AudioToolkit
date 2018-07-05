/**
* \class CMagnitudes
*
* \brief Declaration of a CMagnitudes class interface
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

#ifndef _CMAGNITUDES_H_
#define _CMAGNITUDES_H_

// Some constants that we don't want to change
#define DEFAULT_SOUND_SPEED	343.0f						///< Default sound speed, in meters per second (m/s)
#define DISTANCE_MODEL_THRESHOLD_NEAR 1.95				///< Reference distance and near-distance threshold, in meters
#define DISTANCE_MODEL_THRESHOLD_FAR 15					///< Far-distance threshold, in meters
#define EPSILON_DISTANCE  0.0001f						
#define EPSILON_ATTACK_SAMPLES  0.001f					///< Attack sample lower limit attenuation in simple attenuation distance (used in ApplyGainExponentially method)
#define ATTACK_TIME_DISTANCE_ATTENUATION 100			///< Attack time for gradual attenuation in simple attenuation distance (used in ApplyGainExponentially method)


namespace Common {
	/** \details This class declare the vars and methods for handling physical magnitudes.
	*/
	class CMagnitudes
	{
		// METHODS:
	public:

		/** \brief Default constructor
		*	\details By default, sets distance attenuation to -6dB for anechoic process and -3dB for reverb process, and sound speed to 343 m/s.
		*   \eh Nothing is reported to the error handler.
		*/
		CMagnitudes();

		/** \brief Set distance attenuation factor for anechoic process
		*	\param [in] _anechoicAttenuationDB distance attenuation for anechoic process, in decibels
		*	\pre Attenuation value must be negative
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetAnechoicDistanceAttenuation(float _anechoicAttenuationDB);

		/** \brief Get distance attenuation factor for anechoic process
		*	\retval attenuation attenuation factor for anechoic process, in decibels
		*   \eh Nothing is reported to the error handler.
		*/
		float GetAnechoicDistanceAttenuation() const;

		/** \brief Set distance attenuation factor for reverb process
		*	\param [in] _reverbAttenuationDB distance attenuation for reverb process, in decibels
		*	\pre Attenuation value must be negative
		*   \eh On success, RESULT_OK is reported to the error handler.
		*/
		void SetReverbDistanceAttenuation(float _reverbAttenuationDB);

		/** \brief Get distance attenuation factor for reverb process
		*	\retval attenuation attenuation factor for reverb process, in decibels
		*   \eh Nothing is reported to the error handler.
		*/
		float GetReverbDistanceAttenuation() const;

		/** \brief Set distance attenuation factor for reverb process
		*	\param [in] _soundSpeed sound speed, in meters per second (m/s)
		*	\pre Sound speed must be greater than zero
		*   \eh On error, an error code is reported to the error handler.
		*/
		void SetSoundSpeed(float _soundSpeed);

		/** \brief Get sound speed
		*	\retval speed sound speed, in meters per second (m/s)
		*   \eh Nothing is reported to the error handler.
		*/
		float GetSoundSpeed() const;

		/** \brief Compare two float values
		*	\retval if both float are equals
		*   \eh Nothing is reported to the error handler.
		*/
		static bool AreSame(float a, float b, float epsilon);

		// ATTRIBUTES:
	private:
		float anechoicAttenuationDB;    // Constant for modeling the attenuation due to distance in anechoic process, in decibel units
		float reverbAttenuationDB;		// Constant for modeling the attenuation due to distance in reverb process, in decibel units
		float soundSpeed;				// Constant for modeling sound speed
	};
}//end namespace Common
#endif