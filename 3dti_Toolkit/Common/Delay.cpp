/**
* \class CDelay
*
* \brief Declaration of CDelay class.
* \details Class detect the envelopment of an audio signal
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
#include "Delay.h"
#include <Common/EnvelopeDetector.h>
#include <Common/DynamicCompressorMono.h>
#include <Common/ErrorHandler.h>
#include <cmath>
#include <math.h>

namespace Common {

	//////////////////////////////////////////////////////////////////
	CDelay::CDelay()
	{

	}
	//////////////////////////////////////////////////////////////////
	void CDelay::Setup(int delayInSamples)
	{
// TODO: Check if fill can be setup with 0 delayInSamples 
		savedSamples.Fill( delayInSamples, 0 );
	}
	//////////////////////////////////////////////////////////////////
	void CDelay::Process(CMonoBuffer<float> &inBuffer, CMonoBuffer<float>& outBuffer)
	{
		if (savedSamples.size() == 0)
		{
			outBuffer = inBuffer;
			return;
		}

		// Error handler:
		if(inBuffer.size() < savedSamples.size() )
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "The buffer has not enough samples");
			return;
		}

		//// Error handler:
		//if (outBuffer.size() != inBuffer.size())
		//{
		//	SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Input and output buffers should have the same size");
		//	return;
		//}

		int delayInSamples = savedSamples.size();

		for (int c = 0; c < outBuffer.size(); c++)
		{
			if (c < delayInSamples)
			{
				outBuffer[c] = savedSamples[c];
				savedSamples[c] = inBuffer[ inBuffer.size() - delayInSamples + c ] ;
			}
			else
				outBuffer[c] = inBuffer[c - savedSamples.size() ];
		}
	}
	//////////////////////////////////////////////////////////////////
	CMonoBuffer<float>* CDelay::GetBuffer()
	{
		return &savedSamples;
	}

	//////////////////////////////////////////////////////////////////
	void CDelay::Reset()
	{
		savedSamples.Fill(savedSamples.size(), 0.0f);
	}

} //end namespace Common
