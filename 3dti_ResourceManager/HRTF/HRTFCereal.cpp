/**
*
* \brief Functions to handle HRTFs 3DTI files
*
* \date	March 2016
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

#include <BinauralSpatializer/HRTF.h>
#include "HRTFCereal.h"
#include <fstream>

#if defined (PLATFORM_ANDROID)
#include <iostream>
#include <string>
#ifndef DEBUG_ANDROID
#define DEBUG_ANDROID(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "3DTI_HRTFCereal", __VA_ARGS__))
#endif
#endif

namespace HRTF 
{

//////////////////////////////////////////////////////
		
	bool CreateFrom3dtiStream(std::istream& input3dtiStream, shared_ptr<Binaural::CListener> listener)
	{
		try
		{
			cereal::PortableBinaryInputArchive archive(input3dtiStream);
			HRTFDetail_struct hrtf;
			archive(hrtf);								
			listener->GetHRTF()->BeginSetup(hrtf.hrirLength, hrtf.distanceOfMeasurement);
			listener->GetHRTF()->AddHRTFTable(std::move(hrtf.table));
			listener->GetHRTF()->EndSetup();
			SET_RESULT(RESULT_OK, "HRTF created from 3DTI stream");
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
			SET_RESULT(RESULT_ERROR_EXCEPTION, "String exception when creating HRTF from 3DTI stream");
			return false;
		}
		catch (...)	
		{
			SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when creating HRTF from 3DTI stream");
			return false;
		}
	}

//////////////////////////////////////////////////////

#if defined (PLATFORM_ANDROID)
		void CreateFrom3dtiWithAndroidActivity(const std::string input3dti, ANativeActivity* activity, shared_ptr<Binaural::CListener> listener)
	{
		// TO DO: Use error handler to set result

		// Get access to environment and asset manager
		JNIEnv* env = activity->env;
		AAssetManager* assetManager = activity->assetManager;

		// Open asset
		AAsset* asset = AAssetManager_open(assetManager, input3dti.c_str(), AASSET_MODE_STREAMING);
		if (asset == NULL)
		{
			DEBUG_ANDROID("Asset %s not found!", input3dti.c_str());
			return CHRTF();
		}

		// Write asset to file, so that we can reuse existing code
		std::string dataPath(activity->internalDataPath);
		std::string fileWithPath = dataPath + input3dti;	// Without adding this path, we would receive a read-only error when trying to open the out file
		char buf[BUFSIZ];
		int nb_read = 0;
		FILE* out = fopen(fileWithPath.c_str(), "wb");
		if (out == NULL)
		{
			DEBUG_ANDROID("Error opening file to write asset: %s", strerror(errno));
			return CHRTF();
		}
		while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
		{
			fwrite(buf, sizeof(char), nb_read, out);
		}
		fclose(out);
		AAsset_close(asset);

		// Now, read HRTF from that file		
		CreateFrom3dti(fileWithPath, listener);
#endif

//////////////////////////////////////////////////////
	
	bool CreateFrom3dti(const std::string & input3dti, shared_ptr<Binaural::CListener> listener)
	{
		std::ifstream input3dtiStream(input3dti, std::ios::binary);
		if (input3dtiStream.is_open())
		{
			bool result = CreateFrom3dtiStream(input3dtiStream, listener);
			input3dtiStream.close();
			return result;
		}
		else
		{
#if defined (PLATFORM_ANDROID)
			DEBUG_ANDROID("Could not open input file: %s", input3dti.c_str());
#endif	
			SET_RESULT(RESULT_ERROR_FILE, "Could not open 3DTI-HRTF file");
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
				HRTFDetail_struct hrtf;
				archive(hrtf);
				result = hrtf.samplingRate;
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
				SET_RESULT(RESULT_ERROR_EXCEPTION, "Unknown exception when creating HRTF from 3DTI stream");
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
			SET_RESULT(RESULT_ERROR_FILE, "Could not open 3DTI-HRTF file");
			return false;
		}
	}

//////////////////////////////////////////////////////
}