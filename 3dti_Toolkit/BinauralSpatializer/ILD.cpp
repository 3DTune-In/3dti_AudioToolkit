/*
* \class CILD
*
* \brief Declaration of CILD class.
*
* Class to handle deal with sound sources located close to the listener (Interaural Level Difference)
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

#include <BinauralSpatializer/ILD.h>
#include <Common/ErrorHandler.h>

namespace Binaural {	

	//////////////////////////////////////////////
	CILD::CILD()
	{		
		ILDNearFieldEffectTable_AzimuthStep		= 5;	//In degress
		ILDNearFieldEffectTable_DistanceStep	= 10;	//In milimeters
		ILDSpatializationTable_AzimuthStep		= 5;	//In degress
		ILDSpatializationTable_DistanceStep		= 10;	//In milimeters				
	}
			
	void CILD::AddILDNearFieldEffectTable(T_ILD_HashTable && newTable)
	{		
		t_ILDNearFieldEffect = newTable;
	}

	void CILD::AddILDSpatializationTable(T_ILD_HashTable && newTable)
	{
		t_ILDSpatialization = newTable;
	}

	std::vector<float> CILD::GetILDNearFieldEffectCoefficients(Common::T_ear ear, float distance_m, float azimuth)
	{	
		if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
		{
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get Near Field ILD coefficients for a wrong ear (BOTH or NONE)");
			std::vector<float> temp;
			return temp;
		}

		ASSERT(distance_m > 0, RESULT_ERROR_OUTOFRANGE, "Distance must be greater than zero when processing ILD", "");
		ASSERT(azimuth >= -90.0 && azimuth <= 90, RESULT_ERROR_OUTOFRANGE, "Azimuth must be between -90 deg and 90 deg when processing ILD", "");
		ASSERT(ILDNearFieldEffectTable_AzimuthStep > 0 && ILDNearFieldEffectTable_DistanceStep > 0, RESULT_ERROR_INVALID_PARAM, "Step values of ILD hash table are not valid", "");		

		float distance_mm = distance_m * 1000.0f;

		float distSign = distance_mm > 0 ? 1 : -1;
		float azimSign = azimuth > 0 ? 1 : -1;

		int q_distance_mm = ILDNearFieldEffectTable_DistanceStep * (int)((distance_mm + distSign * ((float)ILDNearFieldEffectTable_DistanceStep) / 2) / ILDNearFieldEffectTable_DistanceStep);
		int q_azimuth = ILDNearFieldEffectTable_AzimuthStep  * (int)((azimuth + azimSign * ((float)ILDNearFieldEffectTable_AzimuthStep) / 2) / ILDNearFieldEffectTable_AzimuthStep);
		if (ear == Common::T_ear::RIGHT)
			q_azimuth = -q_azimuth;
		
		auto itEar = t_ILDNearFieldEffect.find(CILD_Key(q_distance_mm, q_azimuth));

		if (itEar != t_ILDNearFieldEffect.end())
		{			
			std::vector<float> temp(itEar->second.coefs, itEar->second.coefs + 10);
			return temp;									
		}
		else 
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "{Distance-Azimuth} key value was not found in the Near Field ILD look up table");
			std::vector<float> temp;
			return temp;
		}
	}

	std::vector<float> CILD::GetILDSpatializationCoefficients(Common::T_ear ear, float distance_m, float azimuth)
	{
		if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
		{
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get High Performance Spatialization ILD coefficients for a wrong ear (BOTH or NONE)");
			std::vector<float> temp;
			return temp;
		}

		ASSERT(distance_m > 0, RESULT_ERROR_OUTOFRANGE, "Distance must be greater than zero when processing ILD", "");
		ASSERT(azimuth >= -90.0 && azimuth <= 90, RESULT_ERROR_OUTOFRANGE, "Azimuth must be between -90 deg and 90 deg when processing ILD", "");
		ASSERT(ILDSpatializationTable_AzimuthStep > 0 && ILDSpatializationTable_DistanceStep > 0, RESULT_ERROR_INVALID_PARAM, "Step values of ILD hash table are not valid", "");

		float distance_mm = distance_m * 1000.0f;

		float distSign = distance_mm > 0 ? 1 : -1;
		float azimSign = azimuth > 0 ? 1 : -1;

		int q_distance_mm = ILDSpatializationTable_DistanceStep * (int)((distance_mm + distSign * ((float)ILDSpatializationTable_DistanceStep) / 2) / ILDSpatializationTable_DistanceStep);
		int q_azimuth = ILDSpatializationTable_AzimuthStep  * (int)((azimuth + azimSign * ((float)ILDSpatializationTable_AzimuthStep) / 2) / ILDSpatializationTable_AzimuthStep);
		if (ear == Common::T_ear::RIGHT)
			q_azimuth = -q_azimuth;

		auto itEar = t_ILDSpatialization.find(CILD_Key(q_distance_mm, q_azimuth));

		if (itEar != t_ILDSpatialization.end())
		{
			std::vector<float> temp(itEar->second.coefs, itEar->second.coefs + 10);
			return temp;
		}
		else {
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "{Distance-Azimuth} key value was not found in the High Performance Spatialization ILD look up table");
			std::vector<float> temp;
			return temp;
		}
	}
}
