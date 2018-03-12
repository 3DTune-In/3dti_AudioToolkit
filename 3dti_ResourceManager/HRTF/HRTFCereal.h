/**
*
* \brief Functions to handle HRTFs
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

#ifndef HRTFCereal_h
#define HRTFCereal_h

#ifndef PLATFORM_DEFINED
	#if defined (__ANDROID_API__)
		#include <to_string.hpp>
		#if !defined(UNITY_ANDROID)
			#define PLATFORM_ANDROID		
		#endif
	#endif
	//#if defined(_WIN32) 
	//	#define PLATFORM_WIN32
	//#endif
	//#if defined(_WIN64)
	//	#define PLATFORM_WIN64
	//#endif	
	//// TO DO: Mac OSX and iOS: http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
	#define PLATFORM_DEFINED
#endif

#include <Common/Buffer.h>
#include <BinauralSpatializer/HRTF.h>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <stdint.h>

// Serialization function for orientations
template <class Archive>
void serialize(Archive & ar, orientation & ori)
{
	ar(ori.azimuth, ori.elevation);
}

// Serialization function for pairs of HRIRs 
template <class Archive>
void serialize(Archive & ar, THRIRStruct & hrir)
{
	ar(hrir.leftDelay, hrir.rightDelay, hrir.leftHRIR, hrir.rightHRIR); // FIXME: should use stdint types!!
}

// Auxiliary struct required to archive CHRTF
struct HRTFDetail_struct
{
	//uint32_t azimuthStep;
	//uint32_t elevationStep;
	uint32_t samplingRate;
	uint32_t hrirLength;
	float distanceOfMeasurement;
	T_HRTFTable table;
};

// Serialization function for HRIR archive
template <class Archive>
void serialize(Archive & ar, HRTFDetail_struct & h)
{	
	ar(h.samplingRate, h.hrirLength, h.distanceOfMeasurement, h.table);
}

namespace HRTF {		
	
	/** \brief Returns the sample rate in the 3dti file whose path is input3dti
	*	\param [in] path of the 3dti file whose sampling rate will be returned
	*   \eh On error, an error code is reported to the error handler.
	*	\retval the sample rate of the file whose path is input3dti */
	int GetSampleRateFrom3dti(const std::string & input3dti);	

	/** \brief Load HRTF head from 3dti file. 
	*	\param [in] path of the 3dti file
	*	\param [out] listener affected by the hrtf
	*   \eh On error, an error code is reported to the error handler. */
	bool CreateFrom3dti(const std::string & input3dti, shared_ptr<Binaural::CListener> listener);
	
	// Load HRTF head from a stream opened from a 3dti file
	bool CreateFrom3dtiStream(std::istream& input3dtiStream, shared_ptr<Binaural::CListener> listener);	

#if defined(PLATFORM_ANDROID)
	//Binaural::CHRTF CreateFrom3dtiWithAndroidActivity(const std::string input3dti, ANativeActivity* activity, int bufferSize, int sampleRate);	///< Create head from 3dti file in Android platform	
	void CreateFrom3dtiWithAndroidActivity(const std::string input3dti, ANativeActivity* activity, shared_ptr<Binaural::CListener> listener);	///< Load HRTF head from 3dti file in Android platform	
#endif	
}

#endif