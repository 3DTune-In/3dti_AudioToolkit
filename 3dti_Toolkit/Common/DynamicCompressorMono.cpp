/**
* \class DynamicCompresorMono
*
* \brief Declaration of DynamicCompresorMono class.
* \details Class to apply the effects of audio compression
* \date	February 2016
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
#include <Common/DynamicCompressorMono.h>
#include <Common/ErrorHandler.h>
#include <cmath>
//#include "Defaults.h"
#include <math.h>

namespace Common {

	//////////////////////////////////////////////////////////////////
	CDynamicCompressorMono::CDynamicCompressorMono()
	{
		dynamicProcessApplied = false;
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorMono::Setup(int samplingRate, float ratio, float threshold, float attack, float release)
	{
		envDetector.Setup(samplingRate);
		SetRatio(ratio);
		SetThreshold(threshold);
		SetAttack(attack);
		SetRelease(release);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorMono::SetAttack(float attack)
	{
		envDetector.SetAttackTime(attack);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorMono::SetRelease(float release)
	{
		envDetector.SetReleaseTime(release);
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicCompressorMono::GetAttack()
	{
		return envDetector.GetAttackTime();
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicCompressorMono::GetRelease()
	{
		return envDetector.GetReleaseTime();
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicCompressorMono::GetSlope()
	{
		return 1.0f - 1.0 / ratio;
	}
	//////////////////////////////////////////////////////////////////
	//bool CDynamicCompressorMono::GetCompression()
	//{
	//	return compressionApplied;
	//}

	//////////////////////////////////////////////////////////////////
	//void CDynamicCompressorMono::Process(CStereoBuffer<float> &buffer, bool leftChannel)
	//{
	//	CMonoBuffer<float> leftBuffer;
	//	CMonoBuffer<float> rightBuffer;

	//	buffer.Deinterlace(leftBuffer, rightBuffer);

	//	if (leftChannel)
	//		Process(leftBuffer);
	//	else
	//		Process(rightBuffer);

	//	buffer.Interlace(leftBuffer, rightBuffer);
	//}

	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorMono::Process(CMonoBuffer<float> &buffer)
	{
		dynamicProcessApplied = false;

		// ration < 1 implies gain = 1 -> nothing to do
		if (ratio < 1.0001)
			return;

		float s, envelope, gain_db, gain, cs;
		for (int c = 0; c < buffer.size(); c++)
		{
			s = buffer[c];

			envelope = envDetector.ProcessSample(s);

			cs = GetSlope();

			gain_db = cs * (threshold - 20.0f * std::log10(envelope));

			if (gain_db > 0)
				gain = 1;
			else
			{
				gain = std::pow(10.0f, gain_db / 20.0f);
				dynamicProcessApplied = true;
			}

			buffer[c] = s * gain;
			//WATCH(WV_COMPRESSOR_OUTPUT, buffer[c], float);
		}
	}
} //end namespace Common