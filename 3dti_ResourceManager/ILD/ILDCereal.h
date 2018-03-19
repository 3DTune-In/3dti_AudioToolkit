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

#ifndef ILDCereal_h
#define ILDCereal_h

#ifndef PLATFORM_DEFINED
	//#if defined(_WIN32) 
	//	#define PLATFORM_WIN32
	//#endif
	//#if defined(_WIN64)
	//	#define PLATFORM_WIN64
	//#endif	
	//// TO DO: Mac OSX and iOS: http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
	#if defined(__ANDROID_API__)
		#include <to_string.hpp>
		#define PLATFORM_ANDROID
	#endif
	#define PLATFORM_DEFINED
#endif

#include <BinauralSpatializer/ILD.h>
#include <BinauralSpatializer/Listener.h>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/unordered_map.hpp>

/** \brief Serialization function for ILD key
*	\param [out] archive for serialization
*	\param [in] k key 
*   \eh Nothing is reported to the error handler.
*/
template <class Archive>
void serialize(Archive & ar, CILD_Key & k)
{
	ar(k.distance, k.azimuth);
}

/** \brief Serialization function for the coefficients of the biquad fiter
*	\param [out] archive for serialization
*	\param [in] coeficients of the two biquad filters used in the serialization
*   \eh Nothing is reported to the error handler.
*/
template<class Archive>
void serialize(Archive & ar, T_ILD_TwoBiquadFilterCoefs & c)
{
	ar(c.coefs);
}

/** \details Auxiliary struct required to archive ILD with sampling rate */
struct ILDDetail_struct
{
	uint32_t samplingRate;
	T_ILD_HashTable table;
};

/** \details Serialization function for ILD archive */
template <class Archive>
void serialize(Archive & ar, ILDDetail_struct & h)
{	
	ar(h.samplingRate, h.table);
}

namespace ILD 
{
	/** \details Serialization function for ILD archive */
	enum T_ILDTable { ILDNearFieldEffectTable = 0 , ILDSpatializationTable	};
	
	/*T_ILD_HashTable CreateFrom3dti(const std::string & input3dti); ///< Create ILD from 3dti file. 
	T_ILD_HashTable CreateFrom3dtiStream(std::istream& input3dtiStream); ///< Create ILD from a stream opened from a 3dti file*/

	/** \brief Returns the sample rate in the input3dti file
	*	\param [in] 3dti file path
	*   \eh On error, an error code is reported to the error handler.
	*/
	int GetSampleRateFrom3dti(const std::string & input3dti);	///< Get sample rate from ILD 3dti file

	/** \brief Create ILD from 3dti file.
	*   \details Creates a table to process the ILD effect from the 3dti file.
	*	\param [in] 3dti file path with the data to create the ILD table
	*	\param [out] Listener that is affected by the ILD
	*   \eh On error, an error code is reported to the error handler.
	*/
	bool CreateFrom3dti_ILDNearFieldEffectTable(const std::string & input3dti, shared_ptr<Binaural::CListener> listener); 
	/** \brief Creates the spacialization table for ILD from the 3dti input file. 
	*	\param [in] 3dti file path with the data to create the spacialization table
	*	\param [out] Listener that is affected by spacialization table
	*   \eh On error, an error code is reported to the error handler.
	*/
	bool CreateFrom3dti_ILDSpatializationTable (const std::string & input3dti, shared_ptr<Binaural::CListener> listener ); 
	
	/** \brief Create ILD from a stream opened from a 3dti file
	*	\param [in] 3dti file path with the data to create the spacialization table
	*	\param [out] Listener that is affected by spacialization table
	*	\param [out] table containing the ouput data
	*   \eh On error, an error code is reported to the error handler.
	*/
	bool CreateFrom3dtiStream(std::istream& input3dtiStream, shared_ptr<Binaural::CListener> listener, T_ILDTable tableDestiny);
}

#endif


