/**
* \class CFrequencySmearing
*
* \brief  Declaration of CFrequencySmearing interface.
* \date	October 2017
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

#ifndef _CFSMEARING_H_
#define _CFSMEARING_H_

//#define _USE_MATH_DEFINES


#ifndef FSMEARING_THRESHOLD
#define FSMEARING_THRESHOLD 0.0000001f
#endif

#define DEFAULT_SMEARING_SECTION_SIZE 1
#define DEFAULT_SMEARING_HZ 0.0f
#define MIN_SMEARING_BROADENING_FACTOR 1.01f

namespace HAHLSimulation {

	/** \details This class implements frequency smearing, which simulates the broadening of auditory filters in sensorineural hearing loss
	*/
	class CFrequencySmearing
	{

	public:

		/** \brief Initialize the class and allocate memory.
		*   \details When this method is called, the system initializes variables and allocates memory space for the buffer.
		*	\param [in] _bufferSize size of the input signal buffer (L size)
		*	\param [in] _samplingRate sampling rate, in Hz
		*   \eh On success, RESULT_OK is reported to the error handler.
		*       On error, an error code is reported to the error handler.
		*/
		virtual void Setup(int _bufferSize, float _samplingRate) = 0;

		/** \brief Process one buffer through frequency smearing effect.
		*	\param [in] inputBuffer input buffer, in frequency domain
		*	\param [out] outputBuffer output buffer, in frequency domain
		*   \eh On error, an error code is reported to the error handler.
		*/
		virtual void Process(const CMonoBuffer<float>& inputBuffer, CMonoBuffer<float>& outputBuffer) = 0;

	};
}
#endif
