/**
* \class CommonDefinitions
*
* \brief Declaration of Common definitios
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
#ifndef _3DTI_COMMONDEFINITIONS_H_
#define _3DTI_COMMONDEFINITIONS_H_

namespace Common
{
	//----------------------------------------------------------------------
	/** \brief Type definition for specifying one ear
	*/
	enum T_ear {
		LEFT = 0,	///< Left ear
		RIGHT = 1,	///< Right ear
		BOTH = 2,	///< Both ears
		NONE = 3	///< No ear
	};

	//----------------------------------------------------------------------

	/* By default, the UPC algorithm is apllied. If the following defines are activated, the basic convolution will be applied. */
	//#define USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC		//Fconvolver witouth UPC algorithms in the anechoic path
	//#define USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB			//Fconvolver witouth UPC algorithms in the reverb path

	//----------------------------------------------------------------------

	/* \brief Declaration of CEarPair class to work with objects that must be duplicated in order to work with
	*        left and right channels.
	*/
	template <class T>
	class CEarPair
	{
	public:
		T left;		///< left channel
		T right;	///< right channel
	};
}
#endif