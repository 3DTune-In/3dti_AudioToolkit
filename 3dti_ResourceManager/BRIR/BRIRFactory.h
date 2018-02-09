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

#ifndef BRIRFactory_h
#define BRIRFactory_h

#include <BinauralSpatializer/BRIR.h>
#include <string>

namespace BRIR
{
	/** \brief Returns the sample rate in Hz in the sofa file
	*	\param [in] path of the sofa file
	*   \eh On error, an error code is reported to the error handler.
	*/
	int GetSampleRateFromSofa(const std::string & sofafile);

	/** \brief Loads an BRIR from a sofa file
	*	\param [in] path of the sofa file
	*	\param [out] listener affected by the hrtf
	*   \eh On error, an error code is reported to the error handler.
	*/
	bool CreateFromSofa(const std::string & sofafile, shared_ptr<Binaural::CEnvironment> environment);

	/** \brief Loads a BRIR from SOFA file so a 3DTI file can be created later
	*	\param [in] path of the sofa file
	*	\param [out] listener affected by the hrtf
	*   \eh On error, an error code is reported to the error handler. */
	bool Create3DTIFromSofa(const std::string & sofafile, shared_ptr<Binaural::CEnvironment> environment);
	
	bool LoadBRIRTableFromSOFA(const std::string & sofafile, shared_ptr<Binaural::CEnvironment> environment);
};

#endif /* BRIRFactory_h */
