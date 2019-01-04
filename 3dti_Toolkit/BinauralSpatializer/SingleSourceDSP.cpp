/**
* \class CSingleSourceDSP
*
* \brief Definition of CSingleSource class.
* \details This class manages the spatialization of a single source
* \date	November 2015
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
#include <BinauralSpatializer/SingleSourceDSP.h>
#include <Common/ErrorHandler.h>

//#define USE_PROFILER_SingleSourceDSP
#ifdef USE_PROFILER_SingleSourceDSP
#include <Common/Profiler.h>
CProfilerDataSet dsSSDSPTransform;
CProfilerDataSet dsSSDSPGetHRIRInterpolated;
CProfilerDataSet dsSSDSPGetHRIRNoInterpolated;
CProfilerDataSet dsSSDSPFreqConvolver;
CProfilerDataSet dsSSDSPTimeConvolver;
#endif



namespace Binaural {

	//Constructor called from CCore class
	CSingleSourceDSP::CSingleSourceDSP(CCore* _ownerCore)
		:ownerCore{ _ownerCore }, enableInterpolation{ true }, enableFarDistanceEffect{ true }, enableDistanceAttenuationAnechoic{ true }, enableNearFieldEffect{true}, 
		spatializationMode{ TSpatializationMode::HighQuality}
	{
		// TO THINK: our initial idea was not to use error handler in constructors. Should this this an exception to the rule?
		//if (owner == NULL)
		//	SET_RESULT(RESULT_ERROR_NULLPOINTER, "3DTI Toolkit Core not created");
		//else
		//	SET_RESULT(RESULT_OK, "");
		ASSERT(_ownerCore != NULL, RESULT_ERROR_NULLPOINTER, "3DTI Toolkit Core not created", "Single Source DSP succesfully created");

		if (ownerCore != NULL)
		{		
			farDistanceEffect.Setup(ownerCore->GetAudioState().sampleRate);
		}
        
	#ifdef USE_PROFILER_SingleSourceDSP	
		PROFILER3DTI.SetAutomaticWrite(dsSSDSPTransform, "PROF_SSDSP_Transform.txt");
		PROFILER3DTI.SetAutomaticWrite(dsSSDSPGetHRIRNoInterpolated, "PROF_SSDSP_GetHRIR_NOInt.txt");
		PROFILER3DTI.SetAutomaticWrite(dsSSDSPGetHRIRInterpolated, "PROF_SSDSP_GetHRIR_INTERPOLATED.txt");
		PROFILER3DTI.SetAutomaticWrite(dsSSDSPFreqConvolver, "PROF_SSDSP_FrequencyConvolver.txt");
		PROFILER3DTI.SetAutomaticWrite(dsSSDSPTimeConvolver, "PROF_SSDSP_TimeConvolver.txt");	
		PROFILER3DTI.StartRelativeSampling(dsSSDSPTransform);
		PROFILER3DTI.StartRelativeSampling(dsSSDSPGetHRIRNoInterpolated);
		PROFILER3DTI.StartRelativeSampling(dsSSDSPGetHRIRInterpolated);
		PROFILER3DTI.StartRelativeSampling(dsSSDSPFreqConvolver);
		PROFILER3DTI.StartRelativeSampling(dsSSDSPTimeConvolver);	
	#endif

		// Independent process control
		enableAnechoic = true;
		enableReverb = true;
		readyForAnechoic = false;
		readyForReverb = false;

		leftAzimuth = 0;
		leftElevation = 0;

		rightAzimuth = 0;
		rightElevation = 0;

		centerAzimuth = 0;
		centerElevation = 0;

		distanceToListener = 0;
		interauralAzimuth = 0;
		
		nearFieldEffectFilters.left.AddFilter();		//Initialize the filter to ILD simulation 
		nearFieldEffectFilters.left.AddFilter();		//Initialize the filter to ILD simulation
		nearFieldEffectFilters.right.AddFilter();		//Initialize the filter to ILD simulation
		nearFieldEffectFilters.right.AddFilter();		//Initialize the filter to ILD simulation
		ILDSpatializationFilters.left.AddFilter();		//Initialize the filter to ILD simulation
		ILDSpatializationFilters.left.AddFilter();		//Initialize the filter to ILD simulation
		ILDSpatializationFilters.right.AddFilter();		//Initialize the filter to ILD simulation
		ILDSpatializationFilters.right.AddFilter();		//Initialize the filter to ILD simulation
	}

	//////////////////////////////////
	// GET/SET METHODS
	//////////////////////////////////
	
	/// Update internal buffer
	void CSingleSourceDSP::SetBuffer(CMonoBuffer<float> & buffer)
	{
		internalBuffer = buffer;	
		readyForAnechoic = true;
		readyForReverb = true;
	}
	/// Get internal buffer
	const CMonoBuffer<float> CSingleSourceDSP::GetBuffer() const
	{
		// TO DO: check readyForAnechoic and/or readyForAnechoic flags?
		ASSERT(internalBuffer.size() > 0, RESULT_ERROR_NOTSET, "Getting empty buffer from single source DSP", "");
		return internalBuffer;
	}
	
	// Move source (position and orientation)
	void CSingleSourceDSP::SetSourceTransform(Common::CTransform newTransform)
	{	
		sourceTransform = newTransform;

		CalculateSourceCoordinates();

		//sourceTransform = CalculateTransformPositionWithRestrictions(newTransform);
	}
	// Get source transform (position and orientation)
	const Common::CTransform & CSingleSourceDSP::GetSourceTransform() const
	{
		return sourceTransform;
	}

	// Returns the attenuation gain of the anechoic process due to distance
	float CSingleSourceDSP::GetAnechoicDistanceAttenuation(float distance) const
	{
		float distAttConstant = ownerCore->GetMagnitudes().GetAnechoicDistanceAttenuation();
		//return enableDistanceAttenuationAnechoic ? distanceAttenuatorAnechoic.left.GetDistanceAttenuation(distAttConstant, distance) : 1.0f;
		return enableDistanceAttenuationAnechoic ? distanceAttenuatorAnechoic.GetDistanceAttenuation(distAttConstant, distance) : 1.0f;
	}
	// Returns the attenuation gain of the reverb process due to distance
	float CSingleSourceDSP::GetReverbDistanceAttenuation(float distance) const
	{
		float distAttConstant = ownerCore->GetMagnitudes().GetReverbDistanceAttenuation();
		//return  enableDistanceAttenuationAnechoic ? distanceAttenuatorReverb.left.GetDistanceAttenuation(distAttConstant, distance):1.0f;
		return  enableDistanceAttenuationAnechoic ? distanceAttenuatorReverb.GetDistanceAttenuation(distAttConstant, distance) : 1.0f;
	}

	void CSingleSourceDSP::SetSpatializationMode(TSpatializationMode _spatializationMode) { spatializationMode = _spatializationMode; }

	TSpatializationMode CSingleSourceDSP::GetSpatializationMode() { return spatializationMode; }
	
	/////////////////////////////
	// ENABLE DISABLE METHODS
	/////////////////////////////

	///Enable HRTF interpolation method	
	void CSingleSourceDSP::EnableInterpolation() { enableInterpolation = true; }
	///Disable HRTF interpolation method
	void CSingleSourceDSP::DisableInterpolation() { enableInterpolation = false; }
	///Get the flag for HRTF interpolation method
	bool CSingleSourceDSP::IsInterpolationEnabled() { return enableInterpolation; }

	///Enable anechoic process for this source	
	void CSingleSourceDSP::EnableAnechoicProcess() { enableAnechoic = true; }
	///Disable anechoic process for this source	
	void CSingleSourceDSP::DisableAnechoicProcess() { enableAnechoic = false; }
	///Get the flag for anechoic process enabling
	bool CSingleSourceDSP::IsAnechoicProcessEnabled() { return enableAnechoic; }

	///Enable reverb process for this source	
	void CSingleSourceDSP::EnableReverbProcess() { enableReverb = true; }
	///Disable reverb process for this source	
	void CSingleSourceDSP::DisableReverbProcess() { enableReverb = false; }
	///Get the flag for reverb process enabling
	bool CSingleSourceDSP::IsReverbProcessEnabled() { return enableReverb; }

	///Enable far distance effect for this source
	void CSingleSourceDSP::EnableFarDistanceEffect() { enableFarDistanceEffect = true; };
	///Disable far distance effect for this source
	void CSingleSourceDSP::DisableFarDistanceEffect() { enableFarDistanceEffect = false; };
	///Get the flag for far distance effect enabling
	bool CSingleSourceDSP::IsFarDistanceEffectEnabled() { return enableFarDistanceEffect; };

	///Enable distance attenuation effect for this source for anechoic path
	void CSingleSourceDSP::EnableDistanceAttenuationAnechoic() { enableDistanceAttenuationAnechoic = true; };
	///Disable distance attenuation effect for this source for anechoic path
	void CSingleSourceDSP::DisableDistanceAttenuationAnechoic() { enableDistanceAttenuationAnechoic = false; };
	///Get the flag for distance attenuation effect enabling for anechoic path
	bool CSingleSourceDSP::IsDistanceAttenuationEnabledAnechoic() { return enableDistanceAttenuationAnechoic; };

	///Enable distance attenuation effect for this source for anechoic path
	void CSingleSourceDSP::EnableDistanceAttenuationReverb() { enableDistanceAttenuationReverb = true; };
	///Disable distance attenuation effect for this source for anechoic path
	void CSingleSourceDSP::DisableDistanceAttenuationReverb() { enableDistanceAttenuationReverb = false; };
	///Get the flag for distance attenuation effect enabling for anechoic path
	bool CSingleSourceDSP::IsDistanceAttenuationEnabledReverb() { return enableDistanceAttenuationReverb; };

	///Enable near field effect for this source
	void CSingleSourceDSP::EnableNearFieldEffect() { enableNearFieldEffect = true; };
	///Disable near field effect for this source
	void CSingleSourceDSP::DisableNearFieldEffect() { enableNearFieldEffect = false; };
	///Get the flag for near field effect enabling
	bool CSingleSourceDSP::IsNearFieldEffectEnabled() { return enableNearFieldEffect; };
			
	/////////////////////////////
	// RESET METHODS
	/////////////////////////////
	void CSingleSourceDSP::ResetSourceBuffers() 
	{
		if (ownerCore->GetListener()->GetHRTF() != nullptr) {
			ResetSourceConvolutionBuffers(ownerCore->GetListener());
		}
		//TODO reset ILD and others buffers
	};

	/////////////////////////////
	// PROCESS METHODS
	/////////////////////////////

	// Process data from input buffer to generate anechoic spatialization (direct path). Overloaded: using internal buffer
	void CSingleSourceDSP::ProcessAnechoic(CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer)
	{
		if (readyForAnechoic)
			ProcessAnechoic(internalBuffer, outLeftBuffer, outRightBuffer);
		else
		{
			SET_RESULT(RESULT_WARNING, "Attempt to do anechoic process without updating source buffer; please call to SetBuffer before ProcessAnechoic.");
			outLeftBuffer.Fill(ownerCore->GetAudioState().bufferSize, 0.0f);
			outRightBuffer.Fill(ownerCore->GetAudioState().bufferSize, 0.0f);
		}
	}
	// Process data from input buffer to generate anechoic spatialization (direct path). Overloaded: using internal buffer
	void CSingleSourceDSP::ProcessAnechoic(CStereoBuffer<float> & outBuffer)
	{
		CMonoBuffer<float> outLeftBuffer;
		CMonoBuffer<float> outRightBuffer;
		ProcessAnechoic(outLeftBuffer, outRightBuffer);
		outBuffer.Interlace(outLeftBuffer, outRightBuffer);
	}

	// Process data from input buffer to generate anechoic spatialization (direct path)
	void CSingleSourceDSP::ProcessAnechoic(const CMonoBuffer<float> & _inBuffer/* FIXME: can be const ref */, CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer)
	{
		ASSERT(_inBuffer.size() == ownerCore->GetAudioState().bufferSize, RESULT_ERROR_BADSIZE, "InBuffer size has to be equal to the input size indicated by the Core::SetAudioState method", "");
		
		// Check process flag
		if (!enableAnechoic)
		{
			outLeftBuffer.Fill(ownerCore->GetAudioState().bufferSize, 0.0f);
			outRightBuffer.Fill(ownerCore->GetAudioState().bufferSize, 0.0f);
			return;
		}

		#ifdef USE_PROFILER_SingleSourceDSP
			PROFILER3DTI.RelativeSampleStart(dsSSDSPTransform);
		#endif

		// WATCHER 
		WATCH(TWatcherVariable::WV_ANECHOIC_AZIMUTH_LEFT, leftAzimuth, float);
		WATCH(TWatcherVariable::WV_ANECHOIC_AZIMUTH_RIGHT, rightAzimuth, float);		

		#ifdef USE_PROFILER_SingleSourceDSP
			PROFILER3DTI.RelativeSampleEnd(dsSSDSPTransform);
		#endif				
		
		if (_inBuffer.size() == ownerCore->GetAudioState().bufferSize)
		{
			CMonoBuffer<float> inBuffer = _inBuffer; //We have to copy input buffer to a new buffer because the distance effects methods work changing the input buffer				
			
			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization
			if (distanceToListener <= ownerCore->GetListener()->GetHeadRadius())
			{
				outLeftBuffer = inBuffer;
				outRightBuffer = inBuffer;
				return;
			}
													 
			//Apply Far distance effect
			if (IsFarDistanceEffectEnabled()) {	ProcessFarDistanceEffect(inBuffer, distanceToListener); }			
			
			// Apply distance attenuation
			if (IsDistanceAttenuationEnabledAnechoic()){ ProcessDistanceAttenuationAnechoic(inBuffer, ownerCore->GetAudioState().bufferSize, ownerCore->GetAudioState().sampleRate, distanceToListener );}
			
			//Apply Spatialization
			if( spatializationMode == TSpatializationMode::HighQuality ) {
				ProcessHRTF(inBuffer, outLeftBuffer, outRightBuffer, leftAzimuth, leftElevation, rightAzimuth, rightElevation, centerAzimuth, centerElevation);		// Apply HRTF spatialization effect
				ProcessNearFieldEffect(outLeftBuffer, outRightBuffer, distanceToListener, interauralAzimuth );									// Apply Near field effects (ILD)		
			}
			else if (spatializationMode == TSpatializationMode::HighPerformance)
			{
				outLeftBuffer = inBuffer;			//Copy input to left channel
				outRightBuffer = inBuffer;			//Copy input to right channels						
				ProccesILDSpatializationAndAddITD(outLeftBuffer, outRightBuffer, distanceToListener, interauralAzimuth, leftAzimuth, leftElevation, rightAzimuth, rightElevation);	//Apply the ILD spatialization
			}
			else if (spatializationMode == TSpatializationMode::NoSpatialization) 
			{
				outLeftBuffer = inBuffer;
				outRightBuffer = inBuffer;
			}
			else {//Nothing
			}
			
			// Apply the directionality to simulate the hearing aid device
			float angleToForwardAxisRadians = vectorToListener.GetAngleToForwardAxisRadians();  //angle that this vector keeps with the forward axis
			ProcessDirectionality(outLeftBuffer, outRightBuffer, angleToForwardAxisRadians);

			readyForAnechoic = false;	// Mark the buffer as already used for anechoic process

			// WATCHER
			WATCH(WV_ANECHOIC_OUTPUT_LEFT, outLeftBuffer, CMonoBuffer<float>);
			WATCH(WV_ANECHOIC_OUTPUT_RIGHT, outRightBuffer, CMonoBuffer<float>);
		}
	}
	// Process data from input buffer to generate anechoic spatialization (direct path)
	void CSingleSourceDSP::ProcessAnechoic(const CMonoBuffer<float> & inBuffer, CStereoBuffer<float> & outBuffer)
	{
		CMonoBuffer<float> outLeftBuffer;
		CMonoBuffer<float> outRightBuffer;
		ProcessAnechoic(inBuffer, outLeftBuffer, outRightBuffer);
		outBuffer.Interlace(outLeftBuffer, outRightBuffer);
	}

	// Calculates the values returned by GetEarAzimuth and GetEarElevation
	void CSingleSourceDSP::CalculateSourceCoordinates()
	{

		//Get azimuth and elevation between listener and source
		vectorToListener = ownerCore->GetListener()->GetListenerTransform().GetVectorTo(sourceTransform);

		distanceToListener = vectorToListener.GetDistance();

		//Check listener and source are in the same position
		if (distanceToListener <= EPSILON ) {
			return;
		}

		Common::CVector3 leftVectorTo = ownerCore->GetListener()->GetListenerEarTransform(Common::T_ear::LEFT).GetVectorTo(sourceTransform);
		Common::CVector3 rightVectorTo = ownerCore->GetListener()->GetListenerEarTransform(Common::T_ear::RIGHT).GetVectorTo(sourceTransform);
		Common::CVector3 leftVectorTo_sphereProjection =	GetSphereProjectionPosition(leftVectorTo, ownerCore->GetListener()->GetListenerEarLocalPosition(Common::T_ear::LEFT), ownerCore->GetListener()->GetHRTF()->GetHRTFDistanceOfMeasurement());
		Common::CVector3 rightVectorTo_sphereProjection =	GetSphereProjectionPosition(rightVectorTo, ownerCore->GetListener()->GetListenerEarLocalPosition(Common::T_ear::RIGHT), ownerCore->GetListener()->GetHRTF()->GetHRTFDistanceOfMeasurement());

		leftElevation = leftVectorTo_sphereProjection.GetElevationDegrees();	//Get left elevation
		if (!Common::CMagnitudes::AreSame(ELEVATION_SINGULAR_POINT_UP, leftElevation, EPSILON) && !Common::CMagnitudes::AreSame(ELEVATION_SINGULAR_POINT_DOWN, leftElevation, EPSILON)) 
		{
			leftAzimuth = leftVectorTo_sphereProjection.GetAzimuthDegrees();	//Get left azimuth
		}
		
		rightElevation = rightVectorTo_sphereProjection.GetElevationDegrees();	//Get right elevation	
		if (!Common::CMagnitudes::AreSame(ELEVATION_SINGULAR_POINT_UP, rightElevation, EPSILON) && !Common::CMagnitudes::AreSame(ELEVATION_SINGULAR_POINT_DOWN, rightElevation, EPSILON))
		{
			rightAzimuth = rightVectorTo_sphereProjection.GetAzimuthDegrees();		//Get right azimuth
		}


		centerElevation = vectorToListener.GetElevationDegrees();		//Get elevation from the head center
		if (!Common::CMagnitudes::AreSame(ELEVATION_SINGULAR_POINT_UP, centerElevation, EPSILON) && !Common::CMagnitudes::AreSame(ELEVATION_SINGULAR_POINT_DOWN, centerElevation, EPSILON)) 
		{
			centerAzimuth = vectorToListener.GetAzimuthDegrees();		//Get azimuth from the head center
		}

		interauralAzimuth = vectorToListener.GetInterauralAzimuthDegrees();	//Get Interaural Azimuth

	}

	// Returns the azimuth of the specified ear.
	float CSingleSourceDSP::GetEarAzimuth( Common::T_ear ear ) const
	{
		if (ear == Common::T_ear::LEFT)
			return leftAzimuth;
		else if ( ear == Common::T_ear::RIGHT )
			return rightAzimuth;
        else
        {
            SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Call to CSingleSourceDSP::GetEarAzimuth with invalid param" );
            return 0.0f;
        }
	}

	// Returns the elevation of the specified ear
	float CSingleSourceDSP::GetEarElevation(Common::T_ear ear) const
	{
		if (ear == Common::T_ear::LEFT)
			return leftElevation;
		else if (ear == Common::T_ear::RIGHT)
			return rightElevation;
        else
        {
			SET_RESULT( RESULT_ERROR_INVALID_PARAM, "Call to CSingleSourceDSP::GetEarElevation with invalid param" );
            return 0.0f;
        }
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///PRIVATE METHODS
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void CSingleSourceDSP::ProcessHRTF(CMonoBuffer<float> &inBuffer, CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer, float leftAzimuth, float leftElevation, float rightAzimuth, float rightElevation, float centerAzimuth, float centerElevation)
	{
		ASSERT(ownerCore->GetListener()->GetHRTF()->IsHRTFLoaded(), RESULT_ERROR_NOTSET, "CSingleSourceDSP::ProcessAnechoic: error: HRTF has not been loaded yet.", "");
		////////////////////////////
		//	FREQUENCY CONVOLUTION
		///////////////////////////
		//Get HRIRs
#ifdef USE_PROFILER_SingleSourceDSP
		if (enableInterpolation)
			PROFILER3DTI.RelativeSampleStart(dsSSDSPGetHRIRInterpolated);
		else
			PROFILER3DTI.RelativeSampleStart(dsSSDSPGetHRIRNoInterpolated);
#endif			
		//Make FFT-1 of the output (two channels)	
		CMonoBuffer<float> leftChannel_withoutDelay;
		CMonoBuffer<float> rightChannel_withoutDelay;

		if ((ownerCore->GetListener()->GetHRTF()->IsHRTFLoaded()) && (inBuffer.size() == ownerCore->GetAudioState().bufferSize))
		{

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC

			//Get the HRIR, with different orientation for both ears
			oneEarHRIR_struct leftHRIR_Frequency = listener.GetHRTF()->GetHRIR_frequency(Common::T_ear::LEFT, leftAzimuth, leftElevation, enableInterpolation);
			oneEarHRIR_struct rightHRIR_Frequency = listener.GetHRTF()->GetHRIR_frequency(Common::T_ear::RIGHT,  rightAzimuth, rightElevation, enableInterpolation);
			//Get delay
			leftDelay = leftHRIR_Frequency.delay;
			rightDelay = rightHRIR_Frequency.delay;
#ifdef USE_PROFILER_SingleSourceDSP
			if (enableInterpolation)
				PROFILER3DTI.RelativeSampleEnd(dsSSDSPGetHRIRInterpolated);
			else
				PROFILER3DTI.RelativeSampleEnd(dsSSDSPGetHRIRNoInterpolated);
#endif	
#ifdef USE_PROFILER_SingleSourceDSP
			PROFILER3DTI.RelativeSampleStart(dsSSDSPFreqConvolver);
#endif
			// Make the FFT of input signal			
			CMonoBuffer<float> inBuffer_Frequency;
			Common::CFprocessor::GetFFT(inBuffer, inBuffer_Frequency, listener.GetHRTF()->GetHRIRLength());

			//Multiplication of HRIR and input signal
			CMonoBuffer<float> leftChannel_Frequency;
			CMonoBuffer<float> rightChannel_Frequency;

			Common::CFprocessor::ComplexMultiplication(inBuffer_Frequency, leftHRIR_Frequency.HRIR, leftChannel_Frequency);
			Common::CFprocessor::ComplexMultiplication(inBuffer_Frequency, rightHRIR_Frequency.HRIR, rightChannel_Frequency);

			outputLeft.CalculateIFFT_OLA(leftChannel_Frequency, leftChannel_withoutDelay);
			outputRight.CalculateIFFT_OLA(rightChannel_Frequency, rightChannel_withoutDelay);

#else   //USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC

			//Get the HRIR, with different orientation for both ears
			TOneEarHRIRPartitionedStruct  leftHRIR_partitioned;
			TOneEarHRIRPartitionedStruct  rightHRIR_partitioned;

			leftHRIR_partitioned.HRIR_Partitioned = ownerCore->GetListener()->GetHRTF()->GetHRIR_partitioned(Common::T_ear::LEFT, leftAzimuth, leftElevation, enableInterpolation);
			rightHRIR_partitioned.HRIR_Partitioned = ownerCore->GetListener()->GetHRTF()->GetHRIR_partitioned(Common::T_ear::RIGHT, rightAzimuth, rightElevation, enableInterpolation);

			//Get delay
			leftHRIR_partitioned.delay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::LEFT, centerAzimuth, centerElevation, enableInterpolation);
			rightHRIR_partitioned.delay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::RIGHT, centerAzimuth, centerElevation, enableInterpolation);

#ifdef USE_PROFILER_SingleSourceDSP
			if (enableInterpolation)
				PROFILER3DTI.RelativeSampleEnd(dsSSDSPGetHRIRInterpolated);
			else
				PROFILER3DTI.RelativeSampleEnd(dsSSDSPGetHRIRNoInterpolated);
#endif	
#ifdef USE_PROFILER_SingleSourceDSP
			PROFILER3DTI.RelativeSampleStart(dsSSDSPFreqConvolver);
#endif

#ifdef USE_UPC_WITHOUT_MEMORY
			//UPC algorithm without memory
			outputLeftUPConvolution.ProcessUPConvolution(inBuffer, leftHRIR_partitioned, leftChannel_withoutDelay);
			outputRightUPConvolution.ProcessUPConvolution(inBuffer, rightHRIR_partitioned, rightChannel_withoutDelay);
#else
			//UPC algorothm with memory
			outputLeftUPConvolution.ProcessUPConvolutionWithMemory(inBuffer, leftHRIR_partitioned, leftChannel_withoutDelay);
			outputRightUPConvolution.ProcessUPConvolutionWithMemory(inBuffer, rightHRIR_partitioned, rightChannel_withoutDelay);
#endif

#endif // !USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC		

			ProcessAddDelay_ExpansionMethod(leftChannel_withoutDelay, outLeftBuffer, leftChannelDelayBuffer, leftHRIR_partitioned.delay);
			ProcessAddDelay_ExpansionMethod(rightChannel_withoutDelay, outRightBuffer, rightChannelDelayBuffer, rightHRIR_partitioned.delay);

		}
	}
	
	void CSingleSourceDSP::ProccesILDSpatializationAndAddITD(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float distance, float interauralAzimuth, float leftAzimuth, float leftElevation, float rightAzimuth, float rightElevation)
	{
				
		ProcessILDSpatialization(leftBuffer, rightBuffer, distance, interauralAzimuth);		//ILD spatialization
					
		//Apply the ILD attenuation
		float attenuationILD =  std::pow(10.0f, ownerCore->GetListener()->GetILDAttenutaion() * 0.05f);
		leftBuffer.ApplyGain(attenuationILD);
		rightBuffer.ApplyGain(attenuationILD);

		//Add ITD customized to the outbuffer if it is activated
		if (ownerCore->GetListener()->IsCustomizedITDEnabled())
		{			
			int leftDelay = ownerCore->GetListener()->GetHRTF()->GetCustomizedDelay(leftAzimuth, leftElevation, Common::T_ear::LEFT);		//Get left ITD
			int rightDelay = ownerCore->GetListener()->GetHRTF()->GetCustomizedDelay(rightAzimuth, rightElevation, Common::T_ear::RIGHT);	//Get right ITD

			//Add ITD																														
			CMonoBuffer<float> leftChannel_withoutDelay = leftBuffer;			//Make a copy because the ProcessAdd needs input and output buffer
			CMonoBuffer<float> rightChannel_withoutDelay = rightBuffer;			//Make a copy because the ProcessAdd needs input and output buffer

			ProcessAddDelay_ExpansionMethod(leftChannel_withoutDelay, leftBuffer, leftChannelDelayBuffer, leftDelay);				//Add delay to left buffer
			ProcessAddDelay_ExpansionMethod(rightChannel_withoutDelay, rightBuffer, rightChannelDelayBuffer, rightDelay);			//Add delay to right buffer
		}		
	}

	/// Apply distance attenuation
	void CSingleSourceDSP::ProcessDistanceAttenuationAnechoic(CMonoBuffer<float> &buffer, int bufferSize, int sampleRate, float distance) 
	{		
		float distAttConstant = ownerCore->GetMagnitudes().GetAnechoicDistanceAttenuation();		

		if (IsDistanceAttenuationEnabledAnechoic())
		{
			distanceAttenuatorAnechoic.Process(buffer, distance, distAttConstant, bufferSize, sampleRate);			
		}				
	}
	
	// Apply Far distance effect
	void CSingleSourceDSP::ProcessFarDistanceEffect(CMonoBuffer<float> &buffer, float distance) 
	{							
		if (IsFarDistanceEffectEnabled()){ farDistanceEffect.Process(buffer, distance);}		
	}
	
	// Apply Near field effects (ILD)	
	void CSingleSourceDSP::ProcessNearFieldEffect(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float distance, float interauralAzimuth)
	{
		if (IsNearFieldEffectEnabled()) {
			if (distance > DISTANCE_MODEL_THRESHOLD_NEAR) { return; }

			ASSERT(leftBuffer.size() > 0 || rightBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Input buffer is empty when processing ILD", "");


			//Get coefficients from the ILD table
			std::vector<float> coefficientsLeft = ownerCore->GetListener()->GetILD()->GetILDNearFieldEffectCoefficients(Common::T_ear::LEFT, distance, interauralAzimuth);
			std::vector<float> coefficientsRight = ownerCore->GetListener()->GetILD()->GetILDNearFieldEffectCoefficients(Common::T_ear::RIGHT, distance, interauralAzimuth);

			//Set LEFT coefficients into the filters and process the signal
			if (coefficientsLeft.size() == 10) {
				std::vector<float> temp(coefficientsLeft.begin(), coefficientsLeft.begin() + 5);
				nearFieldEffectFilters.left.GetFilter(0)->SetCoefficients(temp);

				std::vector<float> temp2(coefficientsLeft.begin() + 5, coefficientsLeft.end());
				nearFieldEffectFilters.left.GetFilter(1)->SetCoefficients(temp2);

				nearFieldEffectFilters.left.Process(leftBuffer);
			}

			//Set Right coefficients into the filters and process the signal
			if (coefficientsRight.size() == 10) {
				std::vector<float> temp(coefficientsRight.begin(), coefficientsRight.begin() + 5);
				nearFieldEffectFilters.right.GetFilter(0)->SetCoefficients(temp);

				std::vector<float> temp2(coefficientsRight.begin() + 5, coefficientsRight.end());
				nearFieldEffectFilters.right.GetFilter(1)->SetCoefficients(temp2);

				nearFieldEffectFilters.right.Process(rightBuffer);
			}
		}
	}

	// Apply the directionality to simulate the hearing aid device
	void CSingleSourceDSP::ProcessDirectionality(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float angleToForwardAxisRadians)
	{			
		if (ownerCore->GetListener()->IsDirectionalityEnabled(Common::T_ear::LEFT)) 
		{
			ProcessDirectionality(leftBuffer, ownerCore->GetListener()->GetAnechoicDirectionalityLinearAttenuation(Common::T_ear::LEFT), angleToForwardAxisRadians);
		}
		if (ownerCore->GetListener()->IsDirectionalityEnabled(Common::T_ear::RIGHT))
		{
			ProcessDirectionality(rightBuffer, ownerCore->GetListener()->GetAnechoicDirectionalityLinearAttenuation(Common::T_ear::RIGHT), angleToForwardAxisRadians);
		}	
	}

	void CSingleSourceDSP::ProcessDirectionality(CMonoBuffer<float> &buffer, float directionalityAttenutaion, float angleToForwardAxis_rad)
	{				
		buffer.ApplyGain(ownerCore->GetListener()->CalculateDirectionalityLinearAttenuation(directionalityAttenutaion, angleToForwardAxis_rad));
	}		

	// Apply doppler effect simulation
	void CSingleSourceDSP::ProcessAddDelay_ExpansionMethod(CMonoBuffer<float>& input, CMonoBuffer<float>& output, CMonoBuffer<float>& delayBuffer, int newDelay)
	{
		//Prepare the outbuffer		
		if (output.size() != input.size()) { output.resize(input.size()); }

		//Prepare algorithm variables
		float position = 0;
		float numerator = input.size() - 1;
		float denominator = input.size() - 1 + newDelay - delayBuffer.size();
		float compressionFactor = numerator / denominator;

		//Add samples to the output from buffer
		for (int i = 0; i<delayBuffer.size(); i++)
		{
			output[i] = delayBuffer[i];
		}		

		//Fill the others buffers
		//if the delay is the same one as the previous frame use a simplification of the algorithm
		if (newDelay == delayBuffer.size())
		{
			//Copy input to output
			int j = 0;
			for (int i = delayBuffer.size(); i<input.size(); i++)
			{
				output[i] = input[j++];
			}
			//Fill delay buffer
			for (int i = 0; i < newDelay; i++)
			{
				delayBuffer[i] = input[j++];
			}
		}
		//else, apply the expansion/compression algorihtm
		else
		{
			int j;
			float rest;
			int forLoop_end;
			//The last loop iteration must be addressed in a special way if newDelay = 0 (part 1)
			if (newDelay == 0) { forLoop_end = input.size() - 1; }
			else { forLoop_end = input.size(); }

			//Fill the output buffer with the new values 
			for (int i = delayBuffer.size(); i < forLoop_end; i++)
			{
				j = static_cast<int>(position);
				rest = position - j;
				output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
				position += compressionFactor;
			}

			//The last loop iteration must be addressed in a special way if newDelay = 0 (part 2)
			if (newDelay == 0)
			{
				output[input.size() - 1] = input[input.size() - 1];
				delayBuffer.clear();
			}

			//if newDelay!=0 fill out the delay buffer
			else
			{
				//Fill delay buffer 			
				CMonoBuffer<float> temp;
				temp.reserve(newDelay);
				for (int i = 0; i < newDelay - 1; i++)
				{
					int j = int(position);
					float rest = position - j;
					temp.push_back(input[j] * (1 - rest) + input[j + 1] * rest);
					position += compressionFactor;
				}
				//Last element of the delay buffer that must be addressed in a special way
				temp.push_back(input[input.size() - 1]);
				//delayBuffer.swap(temp);				//To use in C++03
				delayBuffer = std::move(temp);			//To use in C++11
			}
		}
	}//End ProcessAddDelay_ExpansionMethod
	
	 //Reset the source convolution buffers
	void CSingleSourceDSP::ResetSourceConvolutionBuffers(shared_ptr<CListener> listener)
	{		
		#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_ANECHOIC
			outputLeft.Setup(ownerCore->GetAudioState().bufferSize, listener->GetHRTF()->GetHRIRLength());
			outputRight.Setup(ownerCore->GetAudioState().bufferSize, listener->GetHRTF()->GetHRIRLength());
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		#else
			int numOfSubfilters = listener->GetHRTF()->GetHRIRNumberOfSubfilters();
			int subfilterLength = listener->GetHRTF()->GetHRIRSubfilterLength();
			outputLeftUPConvolution.Setup(ownerCore->GetAudioState().bufferSize, subfilterLength, numOfSubfilters, true);
			outputRightUPConvolution.Setup(ownerCore->GetAudioState().bufferSize, subfilterLength, numOfSubfilters, true);
			//Init buffer to store delay to be used in the ProcessAddDelay_ExpansionMethod method
			leftChannelDelayBuffer.clear();
			rightChannelDelayBuffer.clear();
		#endif
	}
	
	
	///Return the flag which tells if the buffer is updated and ready for a new anechoic process
	bool CSingleSourceDSP::IsAnechoicProcessReady() { return readyForAnechoic; }
	///Return the flag which tells if the buffer is updated and ready for a new reverb process
	bool CSingleSourceDSP::IsReverbProcessReady() { return readyForReverb; }
	///Sets the ready flag for reverb process to false
	void CSingleSourceDSP::SetReverbProcessNotReady() { readyForReverb = false; }

	void CSingleSourceDSP::ProcessILDSpatialization(CMonoBuffer<float> &leftBuffer, CMonoBuffer<float> &rightBuffer, float distance_m, float azimuth)
	{
		if (distance_m > DISTANCE_MODEL_THRESHOLD_NEAR) { distance_m= DISTANCE_MODEL_THRESHOLD_NEAR; }

		ASSERT(leftBuffer.size() > 0 || rightBuffer.size() > 0, RESULT_ERROR_BADSIZE, "Input buffer is empty when processing ILD", "");


		//Get coefficients from the ILD table
		std::vector<float> coefficientsLeft  = ownerCore->GetListener()->GetILD()->GetILDSpatializationCoefficients(Common::T_ear::LEFT, distance_m, azimuth);
		std::vector<float> coefficientsRight = ownerCore->GetListener()->GetILD()->GetILDSpatializationCoefficients(Common::T_ear::RIGHT, distance_m, azimuth);

		//Set LEFT coefficients into the filters and process the signal
		if (coefficientsLeft.size() == 10) {
			std::vector<float> temp(coefficientsLeft.begin(), coefficientsLeft.begin() + 5);
			ILDSpatializationFilters.left.GetFilter(0)->SetCoefficients(temp);

			std::vector<float> temp2(coefficientsLeft.begin() + 5, coefficientsLeft.end());
			ILDSpatializationFilters.left.GetFilter(1)->SetCoefficients(temp2);

			ILDSpatializationFilters.left.Process(leftBuffer);
		}

		//Set Right coefficients into the filters and process the signal
		if (coefficientsRight.size() == 10) {
			std::vector<float> temp(coefficientsRight.begin(), coefficientsRight.begin() + 5);
			ILDSpatializationFilters.right.GetFilter(0)->SetCoefficients(temp);

			std::vector<float> temp2(coefficientsRight.begin() + 5, coefficientsRight.end());
			ILDSpatializationFilters.right.GetFilter(1)->SetCoefficients(temp2);

			ILDSpatializationFilters.right.Process(rightBuffer);
		}
	}

	// In orther to obtain the position where the HRIR is needed, this method calculate the projection of each ear in the sphere where the HRTF has been measured
	const Common::CVector3 CSingleSourceDSP::GetSphereProjectionPosition(Common::CVector3 vectorToEar, Common::CVector3 earLocalPosition, float distance) const
	{
		//get axis according to the defined convention
		float rightAxis =	vectorToEar.GetAxis(RIGHT_AXIS);
		float forwardAxis = vectorToEar.GetAxis(FORWARD_AXIS);
		float upAxis =		vectorToEar.GetAxis(UP_AXIS);
		// Error handler:
		if ((rightAxis == 0.0f) && (forwardAxis == 0.0f) && (upAxis == 0.0f)) {
			ASSERT(false, RESULT_ERROR_DIVBYZERO, "Axes are not correctly set. Please, check axis conventions", "Azimuth computed from vector succesfully");
		}
		//get ear position in right axis
		float earRightAxis = earLocalPosition.GetAxis(RIGHT_AXIS);

		//Resolve a quadratic equation to get lambda, which is the parameter that define the line between the ear and the sphere, passing by the source
		// (x_sphere, y_sphere, z_sphere) = earLocalPosition + lambda * vectorToEar 
		// x_sphere^2 + y_sphere^2 + z_sphere^2 = distance^2

	
		float a = forwardAxis * forwardAxis + rightAxis * rightAxis + upAxis * upAxis;
		float b = 2.0f * earRightAxis * rightAxis;
		float c = earRightAxis * earRightAxis - distance * distance;
		float lambda = (-b + sqrt(b*b - 4.0f* a*c))* 0.5f * (1 / a);

		Common::CVector3 cartesianposition;
		
		cartesianposition.SetAxis(FORWARD_AXIS, lambda * forwardAxis);
		cartesianposition.SetAxis(RIGHT_AXIS, (earRightAxis + lambda * rightAxis));
		cartesianposition.SetAxis(UP_AXIS, lambda * upAxis);

		return cartesianposition;
	}

}



