/**
* \class DynamicExpanderMono
*
* \brief Declaration of DynamicExapanderMono class.
* \details Class to apply the effects of audio expansion
* \date	June 2017
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
#include <Common/DynamicExpanderMono.h>
#include <Common/ErrorHandler.h>
#include <cmath>
#include <math.h>

namespace Common {

	//////////////////////////////////////////////////////////////////
	CDynamicExpanderMono::CDynamicExpanderMono()
	{
		dynamicProcessApplied = false;
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicExpanderMono::Setup(int samplingRate, float ratio, float threshold, float attack, float release)
	{
		envelopeDetector.Setup(samplingRate);
		SetRatio(ratio);
		SetThreshold(threshold);
		SetAttack(attack);
		SetRelease(release);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicExpanderMono::SetAttack(float attack)
	{
		envelopeDetector.SetAttackTime(attack);
	}
	//////////////////////////////////////////////////////////////////
	void CDynamicExpanderMono::SetRelease(float release)
	{
		envelopeDetector.SetReleaseTime(release);
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicExpanderMono::GetAttack()
	{
		return envelopeDetector.GetAttackTime();
	}
	//////////////////////////////////////////////////////////////////
	float CDynamicExpanderMono::GetRelease()
	{
		return envelopeDetector.GetReleaseTime();
	}

	//////////////////////////////////////////////////////////////////
	float CDynamicExpanderMono::GetSlope()
	{
		return ratio;
	}

	//////////////////////////////////////////////////////////////////
	//void CDynamicExpanderMono::Process(CStereoBuffer<float> &buffer, bool leftChannel)
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
	void CDynamicExpanderMono::Process(CMonoBuffer<float> &buffer)
	{
		dynamicProcessApplied = false;

		// ratio < 1 implies gain = 1 -> nothing to do
		if (ratio < 1.0001)
			return;

		float sample, envelope, envelope_db, gain_db, gain, slope;
		for (int c = 0; c < buffer.size(); c++)
		{
			sample = buffer[c];

			envelope = envelopeDetector.ProcessSample(sample);
			envelope_db = 20.0f * std::log10(envelope);
			//WATCH(WV_BUFFER_TEST, envelope, float);

			if (envelope_db < threshold)
			{
				slope = GetSlope() - 1.0f;
				gain_db = slope * (envelope_db - threshold);
				gain = std::pow(10.0f, gain_db / 20.0f);
				dynamicProcessApplied = true;
				buffer[c] = sample * gain;
			}
			//WATCH(WV_EXPANDER_OUTPUT, buffer[c], float);
		}
	}
}//end namespace Common