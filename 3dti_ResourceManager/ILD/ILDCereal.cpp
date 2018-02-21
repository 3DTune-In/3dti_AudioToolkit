/**
*
* \brief Functions to handle ILDs
*
* \date	March 2017
*
* \authors 3DI - DIANA Research Group(University of Malaga), in alphabetical order : M.Cuevas - Rodriguez, C.Garre, D.Gonzalez - Toledo, E.J.de la Rubia - Cuestas, L.Molina - Tanco ||
*Coordinated by, A.Reyes - Lecuona(University of Malaga) and L.Picinali(Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions : (additional authors / contributors can be added here)
*
* \b Project : 3DTI(3D - games for TUNing and lEarnINg about hearing aids) ||
*\b Website : http://3d-tune-in.eu/
*
* \b Copyright : University of Malaga and Imperial College London - 2018
*
* \b Licence : This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement : This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#include <BinauralSpatializer/ILD.h>
#include "ILDCereal.h"
#include <fstream>

#if defined (PLATFORM_ANDROID) 
#include <iostream>
#include <string>
#ifndef DEBUG_ANDROID
#define DEBUG_ANDROID(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "3DTI_ILDCereal", __VA_ARGS__))
#endif
#endif

namespace ILD {

	/*T_ILD_HashTable CreateFrom3dtiStream(std::istream& input3dtiStream)
	{
		try
		{
			cereal::PortableBinaryInputArchive archive(input3dtiStream);
			T_ILD_HashTable ild_data;
			archive(ild_data);
			SET_RESULT(RESULT_OK, "ILD created from 3DTI stream");
			return ild_data;
		}
		catch (const cereal::Exception& e)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, e.what());			
		}
		catch (const std::exception& e)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, e.what());			
		}
		catch (const std::string& e)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, "String exception when creating ILD from 3DTI stream");			
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when creating ILD from 3DTI stream");			
		}
        return T_ILD_HashTable();
	}

	T_ILD_HashTable CreateFrom3dti(const std::string & input3dti)
	{
		std::ifstream input3dtiStream(input3dti, std::ios::binary);
		if (input3dtiStream.is_open())
		{
			T_ILD_HashTable ildTable = CreateFrom3dtiStream(input3dtiStream);
			input3dtiStream.close();
			return ildTable;
		}
		else
		{
#if defined (PLATFORM_ANDROID)
			DEBUG_ANDROID("Could not open input file: %s", input3dti.c_str());
#endif
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Could not open 3DTI-ILD file");
			return T_ILD_HashTable();
		}
	}*/


	bool CreateFrom3dtiStream(std::istream& input3dtiStream, shared_ptr<Binaural::CListener> listener, T_ILDTable tableDestiny)
	{
		try
		{
			cereal::PortableBinaryInputArchive archive( input3dtiStream );
			//T_ILD_HashTable ild_data;
			ILDDetail_struct ild_data;
			archive( ild_data );
			
			if (tableDestiny == T_ILDTable::ILDNearFieldEffectTable) {
				//listener->GetILD()->AddILDNearFieldEffectTable(std::move(ild_data));
				listener->GetILD()->AddILDNearFieldEffectTable(std::move(ild_data.table));
			}
			else if (tableDestiny == T_ILDTable::ILDSpatializationTable) 
			{
				//listener->GetILD()->AddILDSpatialziationTable(std::move(ild_data));
				listener->GetILD()->AddILDSpatializationTable(std::move(ild_data.table));
			}			

			SET_RESULT(RESULT_OK, "ILD created from 3DTI stream");
			return true;
		}
		catch (const cereal::Exception& e)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, e.what());
			return false;
		}
		catch (const std::exception& e)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, e.what());
			return false;
		}
		catch (const std::string& e)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, "String exception when creating ILD from 3DTI stream");
			return false;
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when creating ILD from 3DTI stream");
			return false;
		}		
	}

	bool CreateFrom3dti_ILDNearFieldEffectTable(const std::string & input3dti, shared_ptr<Binaural::CListener> listener)
	{
		std::ifstream input3dtiStream(input3dti, std::ios::binary);
		if (input3dtiStream.is_open())
		{
			bool result = CreateFrom3dtiStream(input3dtiStream, listener, T_ILDTable::ILDNearFieldEffectTable);
			input3dtiStream.close();
			return result;
		}
		else
		{
#if defined (PLATFORM_ANDROID)
			DEBUG_ANDROID("Could not open input file: %s", input3dti.c_str());
#endif
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Could not open 3DTI-ILD file");
			return false;
		}
	}

	bool CreateFrom3dti_ILDSpatializationTable(const std::string & input3dti, shared_ptr<Binaural::CListener> listener)
	{
		std::ifstream input3dtiStream(input3dti, std::ios::binary);
		if (input3dtiStream.is_open())
		{
			bool result = CreateFrom3dtiStream(input3dtiStream, listener, T_ILDTable::ILDSpatializationTable);
			input3dtiStream.close();
			return result;
		}
		else
		{
#if defined (PLATFORM_ANDROID)
			DEBUG_ANDROID("Could not open input file: %s", input3dti.c_str());
#endif
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Could not open 3DTI-ILD file");
			return false;
		}
	}

//////////////////////////////////////////////////////

	int GetSampleRateFrom3dti(const std::string & input3dti)
	{
		int result = -1;

		std::ifstream input3dtiStream(input3dti, std::ios::binary);
		if (input3dtiStream.is_open())
		{
			try
			{
				cereal::PortableBinaryInputArchive archive(input3dtiStream);
				ILDDetail_struct ild_data;
				archive(ild_data);
				result = ild_data.samplingRate;
			}
			catch (const cereal::Exception& e)
			{
				SET_RESULT(RESULT_ERROR_EXCEPTION, e.what());				
			}
			catch (const std::exception& e)
			{
				SET_RESULT(RESULT_ERROR_EXCEPTION, e.what());				
			}
			catch (const std::string& e)
			{
				SET_RESULT(RESULT_ERROR_EXCEPTION, "String exception when creating ILD from 3DTI stream");				
			}
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when creating ILD from 3DTI stream");				
			}
			input3dtiStream.close();			
		}
		else
		{
#if defined (PLATFORM_ANDROID)
			DEBUG_ANDROID("Could not open input file: %s", input3dti.c_str());
#endif	
			SET_RESULT(RESULT_ERROR_FILE, "Could not open 3DTI-ILD file");			
		}

		return result;
	}

	//////////////////////////////////////////////////////

}

