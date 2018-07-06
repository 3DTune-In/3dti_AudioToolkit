/**
* \class CListener
*
* \brief Definition of CListener interfaces.
* \version 
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

#include <BinauralSpatializer/Listener.h>
#include <BinauralSpatializer/Core.h>

#define ILDATTENUATION -6.0f
#define NUM_STEPS_TO_INTEGRATE_CARDIOID_FOR_REVERB 100

namespace Binaural 
{

	CListener::CListener(CCore* _ownerCore, float _listenerHeadRadius)
    :ownerCore{_ownerCore},
     listenerHeadRadius{_listenerHeadRadius},
	 listenerILDAttenutationDB{ ILDATTENUATION },
	 enableDirectionality {false, false},	 
	 anechoicDirectionalityLinearAttenuation{0.0f, 0.0f},	 
	 reverbDirectionalityLinearAttenuation{ 0.0f, 0.0f }	 
    {				
		std::unique_ptr<CHRTF> a(new CHRTF(this));		// HRTF of listener
		listenerHRTF = std::move(a);	
		
		std::unique_ptr<CILD> b(new CILD());			// ILD Near Field effect of listener
		listenerILD = std::move(b);		
	}

// LISTENER METHODS
		
	// Get Core AudioState Struct
	Common::TAudioStateStruct CListener::GetCoreAudioState() const
	{
		return ownerCore->GetAudioState();
	}

	// Set listener position and orientation
	void CListener::SetListenerTransform(Common::CTransform _listenerTransform)
	{
		listenerTransform = _listenerTransform;
		
		ownerCore->CalculateSourceCoordinates();

		// WATCHER
		WATCH(WV_LISTENER_POSITION, listenerTransform.GetPosition(), Common::CVector3);		
	}

	// Get listener position and orientation
	const Common::CTransform CListener::GetListenerTransform() const
	{
        return listenerTransform;
	}

	// Get position and orientation of one listener ear
	Common::CTransform CListener::GetListenerEarTransform(Common::T_ear ear) const
	{
		if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
		{
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
			return Common::CTransform();
		}

		Common::CVector3 earLocalPosition = Common::CVector3::ZERO;
		if (ear == Common::T_ear::LEFT) {
			earLocalPosition.SetAxis(RIGHT_AXIS, -listenerHeadRadius);
		}
		else
			earLocalPosition.SetAxis(RIGHT_AXIS, listenerHeadRadius);

		
		return listenerTransform.GetLocalTranslation(earLocalPosition);
	}

	// Get local position of one listener ear
	Common::CVector3 CListener::GetListenerEarLocalPosition(Common::T_ear ear) const
	{
		if (ear == Common::T_ear::BOTH || ear == Common::T_ear::NONE)
		{
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "Attempt to get listener ear transform for BOTH or NONE ears");
			return Common::CVector3();
		}

		Common::CVector3 earLocalPosition = Common::CVector3::ZERO;
		if (ear == Common::T_ear::LEFT) {
			earLocalPosition.SetAxis(RIGHT_AXIS, -listenerHeadRadius);
		}
		else
			earLocalPosition.SetAxis(RIGHT_AXIS, listenerHeadRadius);


		return earLocalPosition;
	}

	// Get HRTF for listener		
	CHRTF* CListener::GetHRTF() const
	{
		return listenerHRTF.get();
	}	
	
	// Get ILD Near Field for listener
	CILD* CListener::GetILD() const
	{
		return listenerILD.get();
	}
			
	// Set head radius for listener (m)
	void CListener::SetHeadRadius(float _listenerHeadRadius)
	{
        listenerHeadRadius = _listenerHeadRadius;
	}

	//Get listener head radious
	float CListener::GetHeadRadius() const{
        return listenerHeadRadius;
    }
			
	//Enable customized ITD process
	void CListener::EnableCustomizedITD() 
	{ 
		listenerHRTF->EnableHRTFCustomizedITD(); 
	}
	
	//Disable customized ITD process
	void CListener::DisableCustomizedITD() 
	{ 
		listenerHRTF->DisableHRTFCustomizedITD(); 
	}
	
	//Get the flag for customized ITD process enabling
	bool CListener::IsCustomizedITDEnabled() 
	{ 
		return listenerHRTF->IsHRTFCustomizedITDEnabled(); 
	}
	
	//Set the notification that a new HRTF has been loaded into CHRTF class
	void CListener::SetHRTFLoaded() 
	{		
		ownerCore->ResetConvolutionBuffers();		
	}

	//Re-Calculate the HRTF resampling and FFT again
	void CListener::CalculateHRTF() 
	{
		if (listenerHRTF!=nullptr){ listenerHRTF->CalculateNewHRTFTable(); }		
	}
	
	
	//Get CMagnitudes instance
	Common::CMagnitudes CListener::GetCoreMagnitudes() const 
	{
		return ownerCore->magnitudes;
	};
	
	//Set ILD attenuation of listener for HighPerformance Spatialization
	void CListener::SetILDAttenutaion(float _listenerILDAttenutationDB) { listenerILDAttenutationDB = _listenerILDAttenutationDB; }
	//Get ILD attenuation of listener for HighPerformance Spatialization
	float CListener::GetILDAttenutaion() { return listenerILDAttenutationDB; }
	
	void CListener::EnableDirectionality(Common::T_ear ear) 
	{ 
		if (ear == Common::T_ear::BOTH)
		{
			EnableDirectionality(Common::T_ear::LEFT);
			EnableDirectionality(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableDirectionality.left = true; 
		if (ear == Common::T_ear::RIGHT)
			enableDirectionality.right = true;
	}

	void CListener::DisableDirectionality(Common::T_ear ear)
	{
		if (ear == Common::T_ear::BOTH)
		{
			DisableDirectionality(Common::T_ear::LEFT);
			DisableDirectionality(Common::T_ear::RIGHT);
			return;
		}
		if (ear == Common::T_ear::LEFT)
			enableDirectionality.left = false;
		if (ear == Common::T_ear::RIGHT)
			enableDirectionality.right = false;
	}
	
	bool CListener::IsDirectionalityEnabled(Common::T_ear ear) 
	{ 
		if (ear == Common::T_ear::LEFT)
			return  enableDirectionality.left; 
		if (ear == Common::T_ear::RIGHT)
			return  enableDirectionality.right;
		return false;
	}					
				
	void CListener::SetDirectionality_dB(Common::T_ear ear, float _directionalityAttenuation)
	{ 
		if (ear == Common::T_ear::BOTH)
		{
			SetDirectionality_dB(Common::T_ear::LEFT, _directionalityAttenuation);
			SetDirectionality_dB(Common::T_ear::RIGHT, _directionalityAttenuation);
			return;
		}
		if (ear == Common::T_ear::LEFT)
		{
			anechoicDirectionalityLinearAttenuation.left = std::pow(10, _directionalityAttenuation / 20);
			reverbDirectionalityLinearAttenuation.left = CalculateReverbDirectionalityLinearAttenuation(_directionalityAttenuation);
		}
		if (ear == Common::T_ear::RIGHT)
		{
			anechoicDirectionalityLinearAttenuation.right = std::pow(10, _directionalityAttenuation / 20);
			reverbDirectionalityLinearAttenuation.right = CalculateReverbDirectionalityLinearAttenuation(_directionalityAttenuation);
		}
	}
	
	float CListener::GetAnechoicDirectionalityAttenuation_dB(Common::T_ear ear) const 
	{ 
		if (ear == Common::T_ear::LEFT)
			return 20 * std::log(anechoicDirectionalityLinearAttenuation.left);
		if (ear == Common::T_ear::RIGHT)
			return 20 * std::log(anechoicDirectionalityLinearAttenuation.right);
		return -1.0f;
	}

	float CListener::GetAnechoicDirectionalityLinearAttenuation(Common::T_ear ear) const
	{
		if (ear == Common::T_ear::LEFT)
			return anechoicDirectionalityLinearAttenuation.left;
		if (ear == Common::T_ear::RIGHT)
			return anechoicDirectionalityLinearAttenuation.right;
		return -1.0f;
	}

	float CListener::GetReverbDirectionalityAttenuation_dB(Common::T_ear ear) const 
	{ 
		if (ear == Common::T_ear::LEFT)			
			return 20 * std::log(reverbDirectionalityLinearAttenuation.left); 
		if (ear == Common::T_ear::RIGHT)
			return 20 * std::log(reverbDirectionalityLinearAttenuation.right);
		return -1.0f;
	}								
	
	float CListener::GetReverbDirectionalityLinearAttenuation(Common::T_ear ear) const
	{
		if (ear == Common::T_ear::LEFT)
			return reverbDirectionalityLinearAttenuation.left;
		if (ear == Common::T_ear::RIGHT)
			return reverbDirectionalityLinearAttenuation.right;
		return -1.0f;
	}

	float CListener::CalculateReverbDirectionalityLinearAttenuation(float directionalityExtend_dB)
	{
		
		float angle_rad = 0;
		float angleStep = M_PI / (float)NUM_STEPS_TO_INTEGRATE_CARDIOID_FOR_REVERB;
		float v = 0;

		for (int c = 0; c <= NUM_STEPS_TO_INTEGRATE_CARDIOID_FOR_REVERB; c++)
		{			
			// Weighted sum of the directionality of the sphere rings
			v += std::pow(CalculateDirectionalityLinearAttenuation(directionalityExtend_dB, angle_rad),2) * std::sin(angle_rad);	
			angle_rad += angleStep;
		}
		// Normalizing (making omnidirectional non-attenuating)
		v = std::sqrt(v /(2 * (NUM_STEPS_TO_INTEGRATE_CARDIOID_FOR_REVERB + 1)));

		return v;		
	}	

	float CListener::CalculateDirectionalityLinearAttenuation(float directionalityExtend, float angleToForwardAxis_rad)
	{
		if		(directionalityExtend > 30)	  directionalityExtend = 30.0f;
		else if (directionalityExtend <  0)	  directionalityExtend =  0.0f;

		float directionalityFactor = 0.5f - 0.5f * std::pow(10, -directionalityExtend / 20);

		return 1 - directionalityFactor + directionalityFactor * std::cos(angleToForwardAxis_rad);
	}

	float CListener::CalculateDirectionalityAttenuation_dB(float directionalityExtend, float angleToForwardAxis_rad)
	{
		return 20.0f * log10(CalculateDirectionalityLinearAttenuation(directionalityExtend, angleToForwardAxis_rad));
	}

	
	int CListener::GetHRTFResamplingStep() const
	{
		return ownerCore->GetHRTFResamplingStep();
	}

	//Reset HRTF
	void CListener::ResetHRTF() {
		listenerHRTF->Reset();		
	}

	//Reset ILD
	void CListener::ResetILD() {
		listenerILD.reset();
		std::unique_ptr<CILD> b(new CILD());			// ILD Near Field effect of listener
		listenerILD = std::move(b);
	}

}
