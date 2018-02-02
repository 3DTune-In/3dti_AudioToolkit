/**
* \class CDynamicCompressorStereo
*
* \brief Declaration of CDynamicCompressorStereo class.
* \details Class to apply the effects of audio compression
* \date	March 2017
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
#include <Common/DynamicCompressorStereo.h>
#include <Common/ErrorHandler.h>
#include <cmath>
//#include "Defaults.h"
#include <math.h>

namespace Common {

	//////////////////////////////////////////////////////////////////
	CDynamicCompressorStereo::CDynamicCompressorStereo()
	{
		dynamicProcessApplied = false;
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorStereo::Setup(int samplingRate, float ratio, float threshold, float attack, float release)
	{
		envelopeDetectorLeft.Setup(samplingRate);
		envelopeDetectorRight.Setup(samplingRate);
		SetRatio(ratio);
		SetThreshold(threshold);
		SetAttack(attack);
		SetRelease(release);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorStereo::SetAttack(float attack)
	{
		envelopeDetectorLeft.SetAttackTime(attack);
		envelopeDetectorRight.SetAttackTime(attack);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorStereo::SetRelease(float release)
	{
		envelopeDetectorLeft.SetReleaseTime(release);
		envelopeDetectorRight.SetReleaseTime(release);
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicCompressorStereo::GetAttack()
	{
		return envelopeDetectorLeft.GetAttackTime();
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicCompressorStereo::GetRelease()
	{
		return envelopeDetectorLeft.GetReleaseTime();
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicCompressorStereo::GetSlope()
	{
		return 1.0f - 1.0 / ratio;
	}
	//////////////////////////////////////////////////////////////////
	//bool CDynamicCompressorStereo::GetCompression()
	//{
	//	return compressionApplied;
	//}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorStereo::Process(CStereoBuffer<float> &buffer)
	{
		Common::CEarPair<CMonoBuffer<float>> bufferPair;
		buffer.Deinterlace(bufferPair.left, bufferPair.right);
		Process(bufferPair);
		buffer.Interlace(bufferPair.left, bufferPair.right);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicCompressorStereo::Process(Common::CEarPair<CMonoBuffer<float>> &buffer)
	{
		dynamicProcessApplied = false;

		// ration < 1 implies gain = 1 -> nothing to do
		if (ratio < 1.0001)
			return;

		float l, r, envelope, envelopeL, envelopeR, gain_db, gain, cs;
		for (int c = 0; c < buffer.left.GetNsamples(); c ++)
		{
			l = buffer.left[c];
			r = buffer.right[c];

			envelopeL = envelopeDetectorLeft.ProcessSample(l);
			envelopeR = envelopeDetectorLeft.ProcessSample(r);

			envelope = max(envelopeL, envelopeR);

			cs = GetSlope();

			gain_db = cs * (threshold - 20.0f * std::log10(envelope));

			if (gain_db > 0)
				gain = 1;
			else
			{
				gain = std::pow(10.0f, gain_db / 20.0f);
				dynamicProcessApplied = true;
			}

			buffer.left[c] = l * gain;
			buffer.right[c] = r * gain;
		}
	}
}//end namespace Common