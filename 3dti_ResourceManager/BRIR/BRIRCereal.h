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

#ifndef BRIRCereal_h
#define BRIRCereal_h

#include <Common/Buffer.h>
#include <BinauralSpatializer/BRIR.h>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <stdint.h>

// Serialization function for virtual speakers
template <class Archive>
void serialize(Archive & ar, TVirtualSpeaker & vs)
{
	ar(vs.vsPosition, vs.vsChannel);
}

// Auxiliary struct required to archive CBRIR
struct BRIRDetail_struct
{
	uint32_t samplingRate;
	uint32_t irLength;
	//TBRIRTablePartitioned table;
	TBRIRTable table;
};

// Serialization function for BRIR archive
template <class Archive>
void serialize(Archive & ar, BRIRDetail_struct & b)
{
	ar(b.samplingRate, b.irLength, b.table);
}

namespace BRIR {


	/** \brief Returns the sample rate in the 3dti file whose path is input3dti
	*	\param [in] path of the 3dti file whose sampling rate will be returned
	*   \eh On error, an error code is reported to the error handler.
	*	\retval the sample rate of the file whose path is input3dti
	*/
	int GetSampleRateFrom3dti(const std::string & input3dti);	

	/** \brief Loads the data in input3dti as BRIR in environment.
	*	\param [in] path of the 3dti file that contains the data to be processed
	*	\param [in] environment in which the data will be loaded.
	*   \eh On error, an error code is reported to the error handler.
	*   \retval Returns true on success. False otherwise
	*/
	bool CreateFrom3dti(const std::string & input3dti, shared_ptr<Binaural::CEnvironment> environment);			

	bool CreateFrom3dtiStream(std::istream& input3dtiStream, shared_ptr<Binaural::CEnvironment> environment);
}

#endif