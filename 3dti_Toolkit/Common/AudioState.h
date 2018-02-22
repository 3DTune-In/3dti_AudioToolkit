/**
* \struct TAudioStateStruct
*
* \brief Declaration of TAudioStateStruct interface.
* \date	September 2017
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

/*! \file */

#ifndef _AUDIO_STATE_H_
#define _AUDIO_STATE_H_

#ifndef DEFAULT_SAMPLE_RATE
#define DEFAULT_SAMPLE_RATE 44100
#endif
#ifndef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 512
#endif

namespace Common 
{
	/** \details Simple AudioState struct to centralise sampleRate and bufferSize
	*/
	struct TAudioStateStruct
	{
		int sampleRate;	///< sample rate in Hertzs
		int bufferSize;	///< buffer size (number of samples for each channel)

		TAudioStateStruct() :TAudioStateStruct{ DEFAULT_SAMPLE_RATE, DEFAULT_BUFFER_SIZE} {}
		TAudioStateStruct(int a, int b) : sampleRate(a), bufferSize(b) {}
	};

}
#endif