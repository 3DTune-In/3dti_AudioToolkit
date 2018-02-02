/**
*
* \brief Functions to handle BRIR
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

#include <BinauralSpatializer/BRIR.h>
#include "BRIRCereal.h"
#include <fstream>

namespace BRIR 
{

//////////////////////////////////////////////////////

	bool CreateFrom3dtiStream(std::istream& input3dtiStream, shared_ptr<Binaural::CEnvironment> environment)
	{
		try
		{
			cereal::PortableBinaryInputArchive archive(input3dtiStream);
			BRIRDetail_struct brir;
			archive(brir);
			environment->GetBRIR()->BeginSetup(brir.irLength);
			//environment->GetBRIR()->AddBRIRTablePartitioned(std::move(brir.table));
			environment->GetBRIR()->AddBRIRTable(std::move(brir.table));
			environment->GetBRIR()->EndSetup();
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
			SET_RESULT(RESULT_ERROR_EXCEPTION, "String exception when creating BRIR from 3DTI stream");
			return false;
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when creating BRIR from 3DTI stream");
			return false;
		}
	}

//////////////////////////////////////////////////////

	bool CreateFrom3dti(const std::string & input3dti, shared_ptr<Binaural::CEnvironment> environment)
	{
		std::ifstream input3dtiStream(input3dti, std::ios::binary);
		if (input3dtiStream.is_open())
		{
			bool result = CreateFrom3dtiStream(input3dtiStream, environment);
			input3dtiStream.close();
			return result;
		}
		else 
		{
			SET_RESULT(RESULT_ERROR_FILE, "Could not open 3DTI-BRIR file");
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
				BRIRDetail_struct brir;
				archive(brir);
				result = brir.samplingRate;
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
			catch (...)
			{
				SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when reading BRIR sample rate from 3DTI stream");
				return false;
			}

			input3dtiStream.close();
			return result;
		}
		else
		{
#if defined (PLATFORM_ANDROID)
			DEBUG_ANDROID("Could not open input file: %s", input3dti.c_str());
#endif	
			SET_RESULT(RESULT_ERROR_FILE, "Could not open 3DTI-BRIR file");
			return false;
		}
	}

//////////////////////////////////////////////////////
}