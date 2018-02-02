/**
* \class CEnvelopeDetector
*
* \brief Declaration of CCompressor class.
* \details Class detect the envelopment of an audio signal
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

#include <Common/EnvelopeDetector.h>
#include <Common/DynamicCompressorMono.h>
#include <Common/ErrorHandler.h>
#include <cmath>
//#include "Defaults.h"
#include <math.h>

#define EPSILON_ 0.00001

namespace Common {

	//////////////////////////////////////////////////////////////////
	CEnvelopeDetector::CEnvelopeDetector()
	{
		samplingRate = 44100;
		envelope = 0;

		SetAttackTime(20);  // m_fAttackTime -> Will be initialized with this call
		SetReleaseTime(100);  // m_fReleaseTime -> Will be initialized with this call
	}
	//////////////////////////////////////////////////////////////////
	void CEnvelopeDetector::Setup(int _samplingRate)
	{
		samplingRate = _samplingRate;
	}
	//////////////////////////////////////////////////////////////////
	void CEnvelopeDetector::SetAttackTime(float ms)
	{
		float den = ms * samplingRate;

		if (den > EPSILON_)
			m_fAttackTime = std::exp(1000.0 * std::log(0.01) / den);
		else
			m_fAttackTime = 0;

		m_fAttackTime_ms = ms;
	}
	//////////////////////////////////////////////////////////////////
	void CEnvelopeDetector::SetReleaseTime(float ms)
	{
		float den = ms * samplingRate;

		if (den > EPSILON_)
			m_fReleaseTime = std::exp(1000.0 * std::log(0.01) / den);
		else
			m_fReleaseTime = 0;

		m_fReleaseTime_ms = ms;
	}
	//////////////////////////////////////////////////////////////////
	float CEnvelopeDetector::ProcessSample(float input_sample)
	{
		input_sample = std::fabs(input_sample);

		if (input_sample > envelope) envelope = m_fAttackTime  * (envelope - input_sample) + input_sample;
		else                         envelope = m_fReleaseTime * (envelope - input_sample) + input_sample;

		return envelope;
	}
}//end namespace Common