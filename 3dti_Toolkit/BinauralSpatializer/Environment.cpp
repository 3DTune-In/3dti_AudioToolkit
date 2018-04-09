/*
* \class CCore
*
* \brief Definition of CCore class.
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

#include <BinauralSpatializer/Core.h>
#include <BinauralSpatializer/SingleSourceDSP.h>
#include <BinauralSpatializer/Listener.h>
#include <BinauralSpatializer/Environment.h>
#include <Common/ErrorHandler.h>
#include <string>

//#define USE_PROFILER_Environment
#ifdef USE_PROFILER_Environment
#include <Common/Profiler.h>
CProfilerDataSet dsEnvEncoder;
CProfilerDataSet dsEnvConvolver;
CProfilerDataSet dsEnvInvFFT;
#endif

namespace Binaural {

	CEnvironment::CEnvironment(class CCore* _ownerCore)
		:ownerCore{ _ownerCore }
	{
		//Create a pointer to the BRIR, where a reference to the enviroment is needed
		std::unique_ptr<CBRIR> tempBRIR(new CBRIR(this));	
		environmentBRIR = std::move(tempBRIR);
		
	#ifdef USE_PROFILER_Environment
		PROFILER3DTI.InitProfiler();
		PROFILER3DTI.SetAutomaticWrite(dsEnvEncoder, "PROF_Environment_Encoder.txt");
		PROFILER3DTI.SetAutomaticWrite(dsEnvConvolver, "PROF_Environment_Convolver.txt");
		PROFILER3DTI.SetAutomaticWrite(dsEnvInvFFT, "PROF_Environment_InvFFT.txt");
		PROFILER3DTI.StartRelativeSampling(dsEnvEncoder);
		PROFILER3DTI.StartRelativeSampling(dsEnvConvolver);
		PROFILER3DTI.StartRelativeSampling(dsEnvInvFFT);
	#endif

		//Init HA directionality
		HADirectionality_LeftChannel_version = -1;
		HADirectionality_RightChannel_version = -1;
	}
	
	// Get Core AudioState Struct
	Common::TAudioStateStruct CEnvironment::GetCoreAudioState() const
	{
		return ownerCore->GetAudioState();
	}
		
	CBRIR* CEnvironment::GetBRIR() const
	{
		return environmentBRIR.get();
	}

	void CEnvironment::ResetReverbBuffers() 
	{
		if (ownerCore != nullptr)
		{
			int bufferLength = ownerCore->GetAudioState().bufferSize;
			int BRIRLength = environmentBRIR->GetBRIRLength();
			if (BRIRLength > 0)
			{

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 				
				//Prepare output buffers to perform Basic convolutions			
				//WARNING: This setup is valid because it is assumed that BRIRLength = AIRLength
				outputLeft.Setup(bufferLength, BRIRLength);
				outputRight.Setup(bufferLength, BRIRLength);
#else						
				if (reverberationOrder == ReverberationOrder::BIDIM) {

					//Prepare output buffers to perform UP convolutions in ProcessVirtualAmbisonicReverb
					wLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					wRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					xLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					xRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					yLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					yRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
				}
				else {
					if (reverberationOrder == ReverberationOrder::TRIDIM){
						wLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						wRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						xLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						xRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						yLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						yRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						zLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						zRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					} 
					else
					{
						wLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						wRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					}
					ResetReverbBuffers();
				}
#endif
			}	
		}
	}

	void CEnvironment::SetABIR()
	{
		if (ownerCore != nullptr)
		{
			int bufferLength = ownerCore->GetAudioState().bufferSize;
			int BRIRLength = environmentBRIR->GetBRIRLength();
			if (BRIRLength > 0)
			{			

	#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
				//Configure AIR values (partitions and FFTs)
				CalculateABIRwithoutPartitions();
				//Prepare output buffers to perform Basic convolutions			
				//WARNING: This setup is valid because it is assumed that BRIRLength = AIRLength
				outputLeft.Setup(bufferLength, BRIRLength);
				outputRight.Setup(bufferLength, BRIRLength);
	#else
				//Configure AIR values (partitions and FFTs)
				CalculateABIRPartitioned();
				if (reverberationOrder == ReverberationOrder::BIDIM) {

					//Prepare output buffers to perform UP convolutions in ProcessVirtualAmbisonicReverb
					wLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					wRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					xLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					xRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					yLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					yRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
				}
				else {
					if (reverberationOrder == ReverberationOrder::TRIDIM){
						wLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						wRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						xLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						xRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						yLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						yRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						zLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						zRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					} else {
						wLeft_UPConvolution. Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
						wRight_UPConvolution.Setup(bufferLength, GetABIR().GetDataBlockLength_freq(), GetABIR().GetDataNumberOfBlocks(), false);
					}
				}
	#endif
			}
			// TODO if (GET_LAST_RESULT() != OK) { RAISE_NOT_INITIALISED_ERROR(...); }
		}
	}

	void CEnvironment::CalculateABIRPartitioned()
	{
		environmentABIR.Setup(ownerCore->GetAudioState().bufferSize, environmentBRIR->GetBRIRLength());
		if (reverberationOrder == ReverberationOrder::BIDIM) {

			//1. Get BRIR values for each channel
			TImpulseResponse_Partitioned northLeft = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::NORTH, Common::T_ear::LEFT);
			TImpulseResponse_Partitioned southLeft = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::SOUTH, Common::T_ear::LEFT);
			TImpulseResponse_Partitioned eastLeft = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::EAST, Common::T_ear::LEFT);
			TImpulseResponse_Partitioned westLeft = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::WEST, Common::T_ear::LEFT);
			TImpulseResponse_Partitioned northRight = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::NORTH, Common::T_ear::RIGHT);
			TImpulseResponse_Partitioned southRight = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::SOUTH, Common::T_ear::RIGHT);
			TImpulseResponse_Partitioned eastRight = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::EAST, Common::T_ear::RIGHT);
			TImpulseResponse_Partitioned westRight = environmentBRIR->GetBRIR_Partitioned(VirtualSpeakerPosition::WEST, Common::T_ear::RIGHT);

			long s = northLeft.size();

			if (s == 0 ||
				northLeft.size() != s ||
				southLeft.size() != s ||
				eastLeft.size() != s ||
				westLeft.size() != s ||
				northRight.size() != s ||
				southRight.size() != s ||
				eastRight.size() != s ||
				westRight.size() != s)
			{
				SET_RESULT(RESULT_ERROR_BADSIZE, "Buffers should be the same and not zero");
				return;
			}

			//2. Init AIR buffers
			TImpulseResponse_Partitioned newAIR_W_left, newAIR_X_left, newAIR_Y_left;
			TImpulseResponse_Partitioned newAIR_W_right, newAIR_X_right, newAIR_Y_right;
			newAIR_W_left.resize(environmentBRIR->GetBRIRNumberOfSubfilters());
			newAIR_X_left.resize(environmentBRIR->GetBRIRNumberOfSubfilters());
			newAIR_Y_left.resize(environmentBRIR->GetBRIRNumberOfSubfilters());
			newAIR_W_right.resize(environmentBRIR->GetBRIRNumberOfSubfilters());
			newAIR_X_right.resize(environmentBRIR->GetBRIRNumberOfSubfilters());
			newAIR_Y_right.resize(environmentBRIR->GetBRIRNumberOfSubfilters());

			for (int i = 0; i < environmentBRIR->GetBRIRNumberOfSubfilters(); i++)
			{
				newAIR_W_left[i].resize(environmentBRIR->GetBRIROneSubfilterLength(), 0.0f);
				newAIR_X_left[i].resize(environmentBRIR->GetBRIROneSubfilterLength(), 0.0f);
				newAIR_Y_left[i].resize(environmentBRIR->GetBRIROneSubfilterLength(), 0.0f);
				newAIR_W_right[i].resize(environmentBRIR->GetBRIROneSubfilterLength(), 0.0f);
				newAIR_X_right[i].resize(environmentBRIR->GetBRIROneSubfilterLength(), 0.0f);
				newAIR_Y_right[i].resize(environmentBRIR->GetBRIROneSubfilterLength(), 0.0f);
			}

			//3. AIR codification from BRIR
			for (int i = 0; i < environmentBRIR->GetBRIRNumberOfSubfilters(); i++)
			{
				for (int j = 0; j < environmentBRIR->GetBRIROneSubfilterLength(); j++)
				{
					newAIR_W_left[i][j] = 0.707107f * (northLeft[i][j] + southLeft[i][j] + eastLeft[i][j] + westLeft[i][j]);
					newAIR_X_left[i][j] = northLeft[i][j] - southLeft[i][j];
					newAIR_Y_left[i][j] = westLeft[i][j] - eastLeft[i][j];

					newAIR_W_right[i][j] = 0.707107f * (northRight[i][j] + southRight[i][j] + eastRight[i][j] + westRight[i][j]);
					newAIR_X_right[i][j] = northRight[i][j] - southRight[i][j];
					newAIR_Y_right[i][j] = westRight[i][j] - eastRight[i][j];
				}
			}

			//Setup AIR class
			environmentABIR.AddImpulseResponse(TBFormatChannel::W, Common::T_ear::LEFT, std::move(newAIR_W_left));
			environmentABIR.AddImpulseResponse(TBFormatChannel::W, Common::T_ear::RIGHT, std::move(newAIR_W_right));
			environmentABIR.AddImpulseResponse(TBFormatChannel::X, Common::T_ear::LEFT, std::move(newAIR_X_left));
			environmentABIR.AddImpulseResponse(TBFormatChannel::X, Common::T_ear::RIGHT, std::move(newAIR_X_right));
			environmentABIR.AddImpulseResponse(TBFormatChannel::Y, Common::T_ear::LEFT, std::move(newAIR_Y_left));
			environmentABIR.AddImpulseResponse(TBFormatChannel::Y, Common::T_ear::RIGHT, std::move(newAIR_Y_right));
		}
		else {
			if (reverberationOrder == ReverberationOrder::TRIDIM){

			} 
			else
			{

			}
		}
	}


	void CEnvironment::CalculateABIRwithoutPartitions()
	{
		if (reverberationOrder == ReverberationOrder::BIDIM) {

			//1. Get BRIR values for each channel
			TImpulseResponse northLeft = environmentBRIR->GetBRIR(VirtualSpeakerPosition::NORTH, Common::T_ear::LEFT);
			TImpulseResponse southLeft = environmentBRIR->GetBRIR(VirtualSpeakerPosition::SOUTH, Common::T_ear::LEFT);
			TImpulseResponse eastLeft = environmentBRIR->GetBRIR(VirtualSpeakerPosition::EAST, Common::T_ear::LEFT);
			TImpulseResponse westLeft = environmentBRIR->GetBRIR(VirtualSpeakerPosition::WEST, Common::T_ear::LEFT);
			TImpulseResponse northRight = environmentBRIR->GetBRIR(VirtualSpeakerPosition::NORTH, Common::T_ear::RIGHT);
			TImpulseResponse southRight = environmentBRIR->GetBRIR(VirtualSpeakerPosition::SOUTH, Common::T_ear::RIGHT);
			TImpulseResponse eastRight = environmentBRIR->GetBRIR(VirtualSpeakerPosition::EAST, Common::T_ear::RIGHT);
			TImpulseResponse westRight = environmentBRIR->GetBRIR(VirtualSpeakerPosition::WEST, Common::T_ear::RIGHT);

			//2. Init AIR buffers
			TImpulseResponse newAIR_W_left, newAIR_X_left, newAIR_Y_left;
			TImpulseResponse newAIR_W_right, newAIR_X_right, newAIR_Y_right;
			newAIR_W_left.resize(environmentBRIR->GetBRIRLength_frequency(), 0.0f);
			newAIR_X_left.resize(environmentBRIR->GetBRIRLength_frequency(), 0.0f);
			newAIR_Y_left.resize(environmentBRIR->GetBRIRLength_frequency(), 0.0f);
			newAIR_W_right.resize(environmentBRIR->GetBRIRLength_frequency(), 0.0f);
			newAIR_X_right.resize(environmentBRIR->GetBRIRLength_frequency(), 0.0f);
			newAIR_Y_right.resize(environmentBRIR->GetBRIRLength_frequency(), 0.0f);


			//3. AIR codification from BRIR

			for (int j = 0; j < environmentBRIR->GetBRIRLength_frequency(); j++)
			{
				newAIR_W_left[j] = 0.707107f * (northLeft[j] + southLeft[j] + eastLeft[j] + westLeft[j]);
				newAIR_X_left[j] = northLeft[j] - southLeft[j];
				newAIR_Y_left[j] = westLeft[j] - eastLeft[j];

				newAIR_W_right[j] = 0.707107f * (northRight[j] + southRight[j] + eastRight[j] + westRight[j]);
				newAIR_X_right[j] = northRight[j] - southRight[j];
				newAIR_Y_right[j] = westRight[j] - eastRight[j];
			}

			//Setup AIR class
			environmentABIR.Setup(ownerCore->GetAudioState().bufferSize, environmentBRIR->GetBRIRLength());
			environmentABIR.AddImpulseResponse(TBFormatChannel::W, Common::T_ear::LEFT, std::move(newAIR_W_left));
			environmentABIR.AddImpulseResponse(TBFormatChannel::W, Common::T_ear::RIGHT, std::move(newAIR_W_right));
			environmentABIR.AddImpulseResponse(TBFormatChannel::X, Common::T_ear::LEFT, std::move(newAIR_X_left));
			environmentABIR.AddImpulseResponse(TBFormatChannel::X, Common::T_ear::RIGHT, std::move(newAIR_X_right));
			environmentABIR.AddImpulseResponse(TBFormatChannel::Y, Common::T_ear::LEFT, std::move(newAIR_Y_left));
			environmentABIR.AddImpulseResponse(TBFormatChannel::Y, Common::T_ear::RIGHT, std::move(newAIR_Y_right));
		}
		else {
			if (reverberationOrder == ReverberationOrder::TRIDIM){

			}
			else
			{

			}
		}
	}
	
//////////////////////////////////////////////
	// Get ABIR for environment
	const CABIR& CEnvironment::GetABIR() const
	{
		return environmentABIR;
	}

//////////////////////////////////////////////

	// Process virtual ambisonic reverb for specified buffers
	void CEnvironment::ProcessVirtualAmbisonicReverb(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight)
	{
		if (!environmentABIR.IsInitialized())
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Data is not ready to be processed");
			return;
		}
		if(reverberationOrder == ReverberationOrder::BIDIM){

		CMonoBuffer<float> w, x, y;	// B-Format data		
		CMonoBuffer<float> w_AbirW_left_FFT;
		CMonoBuffer<float> w_AbirW_right_FFT;
		CMonoBuffer<float> x_AbirX_left_FFT;
		CMonoBuffer<float> x_AbirX_right_FFT;
		CMonoBuffer<float> y_AbirY_left_FFT;
		CMonoBuffer<float> y_AbirY_right_FFT;
		CMonoBuffer<float> mixerOutput_left_FFT;
		CMonoBuffer<float> mixerOutput_right_FFT;
		CMonoBuffer<float> mixerOutput_left;
		CMonoBuffer<float> mixerOutput_right;
		CMonoBuffer<float> ouputBuffer_temp;



		
			/////////////////////////////////////////
			// 1-st Order Virtual Ambisonics Encoder
			/////////////////////////////////////////
#ifdef USE_PROFILER_Environment
			PROFILER3DTI.RelativeSampleStart(dsEnvEncoder);
#endif

			// This would crash if there are no sources created. Rather than reporting error, do nothing
			if (ownerCore->audioSources.size() == 0)
				return;

			// We assume all buffers have the same number of samples
			size_t samplesInBuffer = ownerCore->GetAudioState().bufferSize;

			// Init sumation for B-Format channels
			w.Fill(samplesInBuffer, 0.0f);
			x.Fill(samplesInBuffer, 0.0f);
			y.Fill(samplesInBuffer, 0.0f);
			float WScale = 0.707107f;

			// Go through each source
			//for (int nSource = 0; nSource < ownerCore->audioSources.size(); nSource++)
			for (auto eachSource : ownerCore->audioSources)
			{
				// Check source flags for reverb process
				if (!eachSource->IsReverbProcessEnabled())
					continue;
				if (!eachSource->IsReverbProcessReady())
				{
					SET_RESULT(RESULT_WARNING, "Attempt to do reverb process without updating source buffer; please call to SetBuffer before ProcessVirtualAmbisonicReverb.");
					continue;
				}

				// Get azimuth, elevation and distance from listener to each source
				// We precompute everything, to minimize per-sample computations. 
				Common::CTransform sourceTransform = eachSource->GetSourceTransform();
				sourceTransform = eachSource->CalculateTransformPositionWithRestrictions(sourceTransform);
				Common::CVector3 vectorToSource = ownerCore->GetListener()->GetListenerTransform().GetVectorTo(sourceTransform);
				float sourceAzimuth = vectorToSource.GetAzimuthRadians();
				float sourceElevation = vectorToSource.GetElevationRadians();
				float sourceDistance = vectorToSource.GetDistance();
				float cosAzimuth = std::cos(sourceAzimuth);
				float sinAzimuth = std::sin(sourceAzimuth);
				float sinElevationAbs = std::fabs(std::sin(sourceElevation));	// TEST: adding power to W channel to compensate for the lack of Z channel
				float cosElevation = std::cos(sourceElevation);
				float cosAcosE = cosAzimuth * cosElevation;
				float sinAcosE = sinAzimuth * cosElevation;
				CMonoBuffer<float> sourceBuffer = eachSource->GetBuffer();
				//ASSERT(sourceBuffer.size() > 0, RESULT_ERROR_NOTSET, "Attempt to process virtual ambisonics reverb without previously feeding audio source buffers", "");

				//Apply Distance Attenuation
				float distanceAttenuation_ReverbConstant = ownerCore->GetMagnitudes().GetReverbDistanceAttenuation();
				if (eachSource->IsDistanceAttenuationEnabledReverb()) {
					eachSource->distanceAttenuatorReverb.Process(sourceBuffer, sourceDistance, distanceAttenuation_ReverbConstant, ownerCore->GetAudioState().bufferSize, ownerCore->GetAudioState().sampleRate);
				}
				// Go trough each sample
				for (int nSample = 0; nSample < samplesInBuffer; nSample++)
				{
					// Value from the input buffer				
					float newSample = sourceBuffer[nSample];

					// Add partial contribution of this source to each B-format channel								
					w[nSample] += newSample * WScale;
					x[nSample] += newSample * cosAcosE;
					x[nSample] += newSample * sinElevationAbs; // Adding power to X channel to compensate for the lack of Z channel
					y[nSample] += newSample * sinAcosE;
				}

				// Set flag for reverb process
				eachSource->SetReverbProcessNotReady();
			}

#ifdef USE_PROFILER_Environment
			PROFILER3DTI.RelativeSampleEnd(dsEnvEncoder);
#endif
			///////////////////////////////////////////
			// Frequency-Domain Convolution with ABIR
			///////////////////////////////////////////
#ifdef USE_PROFILER_Environment
			PROFILER3DTI.RelativeSampleStart(dsEnvConvolver);
#endif		
			//TODO All this could be parallelized

			bool bUPConvolution = true;

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
			///////
			// W //
			///////
			CMonoBuffer<float> w_FFT;
			//Make FFT of W		
			Common::CFprocessor::GetFFT(w, w_FFT, environmentABIR.GetDataLength());
			Common::CFprocessor::ComplexMultiplication(w_FFT, GetABIR().GetImpulseResponse(TBFormatChannel::W, T_ear::LEFT), w_AbirW_left_FFT);
			Common::CFprocessor::ComplexMultiplication(w_FFT, GetABIR().GetImpulseResponse(TBFormatChannel::W, T_ear::RIGHT), w_AbirW_right_FFT);

			///////
			// X //
			///////
			CMonoBuffer<float> x_FFT;
			//Make FFT of X		
			Common::CFprocessor::GetFFT(x, x_FFT, environmentABIR.GetDataLength());
			//Complex Product				
			Common::CFprocessor::ComplexMultiplication(x_FFT, GetABIR().GetImpulseResponse(X, T_ear::LEFT), x_AbirX_left_FFT);
			Common::CFprocessor::ComplexMultiplication(x_FFT, GetABIR().GetImpulseResponse(X, T_ear::RIGHT), x_AbirX_right_FFT);

			///////
			// Y //
			///////		
			CMonoBuffer<float> y_FFT;
			//TBFormatChannelData abirY = GetABIR().GetChannelData(Y);
			//Make FFT of Y				
			Common::CFprocessor::GetFFT(y, y_FFT, environmentABIR.GetDataLength());
			//Complex Product		
			Common::CFprocessor::ComplexMultiplication(y_FFT, GetABIR().GetImpulseResponse(Y, T_ear::LEFT), y_AbirY_left_FFT);
			Common::CFprocessor::ComplexMultiplication(y_FFT, GetABIR().GetImpulseResponse(Y, T_ear::RIGHT), y_AbirY_right_FFT);
#else		

			///Apply UPC algorithm			
			wLeft_UPConvolution.ProcessUPConvolution_withoutIFFT(w, GetABIR().GetImpulseResponse_Partitioned(TBFormatChannel::W, Common::T_ear::LEFT), w_AbirW_left_FFT);
			wRight_UPConvolution.ProcessUPConvolution_withoutIFFT(w, GetABIR().GetImpulseResponse_Partitioned(TBFormatChannel::W, Common::T_ear::RIGHT), w_AbirW_right_FFT);
			xLeft_UPConvolution.ProcessUPConvolution_withoutIFFT(x, GetABIR().GetImpulseResponse_Partitioned(X, Common::T_ear::LEFT), x_AbirX_left_FFT);
			xRight_UPConvolution.ProcessUPConvolution_withoutIFFT(x, GetABIR().GetImpulseResponse_Partitioned(X, Common::T_ear::RIGHT), x_AbirX_right_FFT);
			yLeft_UPConvolution.ProcessUPConvolution_withoutIFFT(y, GetABIR().GetImpulseResponse_Partitioned(Y, Common::T_ear::LEFT), y_AbirY_left_FFT);
			yRight_UPConvolution.ProcessUPConvolution_withoutIFFT(y, GetABIR().GetImpulseResponse_Partitioned(Y, Common::T_ear::RIGHT), y_AbirY_right_FFT);
#endif


#ifdef USE_PROFILER_Environment
			PROFILER3DTI.RelativeSampleEnd(dsEnvConvolver);
#endif

			///////////////////////////////////////
			// Mix of channels in Frequency domain
			///////////////////////////////////////

#ifdef USE_PROFILER_Environment
			PROFILER3DTI.RelativeSampleStart(dsEnvInvFFT);
#endif

			mixerOutput_left_FFT.SetFromMix({ w_AbirW_left_FFT, x_AbirX_left_FFT, y_AbirY_left_FFT });
			mixerOutput_right_FFT.SetFromMix({ w_AbirW_right_FFT, x_AbirX_right_FFT, y_AbirY_right_FFT });

			////////////////////////////////////////
			// FFT-1 Going back to the time domain
			////////////////////////////////////////

			//TODO All this could be parallelized


#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 
			outputLeft.CalculateIFFT_OLA(mixerOutput_left_FFT, mixerOutput_left);
			outputRight.CalculateIFFT_OLA(mixerOutput_right_FFT, mixerOutput_right);
#else
			//Left channel
			Common::CFprocessor::CalculateIFFT(mixerOutput_left_FFT, ouputBuffer_temp);
			//We are left only with the final half of the result
			int halfsize = (int)(ouputBuffer_temp.size() * 0.5f);

			CMonoBuffer<float> temp_OutputBlockLeft(ouputBuffer_temp.begin() + halfsize, ouputBuffer_temp.end());
			mixerOutput_left = std::move(temp_OutputBlockLeft);			//To use in C++11

																		//Right channel
			ouputBuffer_temp.clear();
			Common::CFprocessor::CalculateIFFT(mixerOutput_right_FFT, ouputBuffer_temp);
			//We are left only with the final half of the result
			halfsize = (int)(ouputBuffer_temp.size() * 0.5f);
			CMonoBuffer<float> temp_OutputBlockRight(ouputBuffer_temp.begin() + halfsize, ouputBuffer_temp.end());
			mixerOutput_right = std::move(temp_OutputBlockRight);			//To use in C++11
#endif

																			//////////////////////////////////////////////
																			// Mix of chabbels decoded after convolution 
																			//////////////////////////////////////////////

																			//Interlace		TODO Use the method in bufferClass??
			for (int i = 0; i < mixerOutput_left.size(); i++) {
				outBufferLeft.push_back(mixerOutput_left[i]);
				outBufferRight.push_back(mixerOutput_right[i]);
			}


#ifdef USE_PROFILER_Environment
			PROFILER3DTI.RelativeSampleEnd(dsEnvInvFFT);
#endif
			//////////////////////////////////////////////////////////
			// TO DO: REVERBERATION DELAY
			//////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////
			// HA Directionality in reverb path
			//////////////////////////////////////////////////////////		
			if (ownerCore->GetListener()->IsDirectionalityEnabled(Common::T_ear::LEFT)) {
				ProcessDirectionality(outBufferLeft, ownerCore->GetListener()->GetReverbDirectionalityAttenuation_dB(Common::T_ear::LEFT));
			}
			if (ownerCore->GetListener()->IsDirectionalityEnabled(Common::T_ear::RIGHT)) {
				ProcessDirectionality(outBufferRight, ownerCore->GetListener()->GetReverbDirectionalityAttenuation_dB(Common::T_ear::RIGHT));
			}

			// WATCHER
			WATCH(WV_ENVIRONMENT_OUTPUT_LEFT, outBufferLeft, CMonoBuffer<float>);
			WATCH(WV_ENVIRONMENT_OUTPUT_RIGHT, outBufferRight, CMonoBuffer<float>);
	} 
	else
	{
				if (reverberationOrder == ReverberationOrder::TRIDIM){
				
				}
				else 
				{

				}
				ProcessVirtualAmbisonicReverb(outBufferLeft, outBufferRight);
			}
		
	}

	void CEnvironment::ProcessDirectionality(CMonoBuffer<float> &buffer, float directionalityAttenutaion)
	{		
		buffer.ApplyGain(directionalityAttenutaion);
	}


	// Process virtual ambisonic reverb for specified buffers
	void CEnvironment::ProcessVirtualAmbisonicReverb(CStereoBuffer<float> & outBuffer) 
	{
		CMonoBuffer<float> outLeftBuffer;
		CMonoBuffer<float> outRightBuffer;
		ProcessVirtualAmbisonicReverb(outLeftBuffer, outRightBuffer);
		outBuffer.Interlace(outLeftBuffer, outRightBuffer);
	}
//////////////////////////////////////////////

	// Process reverb for one b-format channel encoded with 1st order ambisonics (useful for some wrappers)
	void CEnvironment::ProcessEncodedChannelReverb(TBFormatChannel channel, CMonoBuffer<float> encoderIn, CMonoBuffer<float> & output)
	{	
		CMonoBuffer<float> channel_FFT;
		CMonoBuffer<float> Convolution_left_FFT;
		CMonoBuffer<float> Convolution_right_FFT;

		// Inverse FFT: Back to time domain
		CMonoBuffer<float> leftOutputBuffer;
		CMonoBuffer<float> rightOutputBuffer;	

		// error handler: Trust in called methods for setting result
		if (reverberationOrder == ReverberationOrder::BIDIM) {

			

#ifdef USE_FREQUENCY_COVOLUTION_WITHOUT_PARTITIONS_REVERB 

			//Make FFT and frequency convolution		
			Common::CFprocessor::GetFFT(encoderIn, channel_FFT, environmentABIR.GetDataLength());
			Common::CFprocessor::ComplexMultiplication(channel_FFT, GetABIR().GetImpulseResponse(channel, T_ear::RIGHT), Convolution_right_FFT);
			Common::CFprocessor::ComplexMultiplication(channel_FFT, GetABIR().GetImpulseResponse(channel, T_ear::LEFT), Convolution_left_FFT);
			//FFT Inverse
			outputLeft.CalculateIFFT_OLA(Convolution_left_FFT, leftOutputBuffer);
			outputRight.CalculateIFFT_OLA(Convolution_right_FFT, rightOutputBuffer);
#else
			///UPC Convolution
			if (channel == TBFormatChannel::W)
			{
				wLeft_UPConvolution.ProcessUPConvolution(encoderIn, GetABIR().GetImpulseResponse_Partitioned(TBFormatChannel::W, Common::T_ear::LEFT), leftOutputBuffer);
				wRight_UPConvolution.ProcessUPConvolution(encoderIn, GetABIR().GetImpulseResponse_Partitioned(TBFormatChannel::W, Common::T_ear::RIGHT), rightOutputBuffer);
			}
			else if (channel == TBFormatChannel::X)
			{
				xLeft_UPConvolution.ProcessUPConvolution(encoderIn, GetABIR().GetImpulseResponse_Partitioned(X, Common::T_ear::LEFT), leftOutputBuffer);
				xRight_UPConvolution.ProcessUPConvolution(encoderIn, GetABIR().GetImpulseResponse_Partitioned(X, Common::T_ear::RIGHT), rightOutputBuffer);
			}
			else if (channel == TBFormatChannel::Y)
			{
				yLeft_UPConvolution.ProcessUPConvolution(encoderIn, GetABIR().GetImpulseResponse_Partitioned(Y, Common::T_ear::LEFT), leftOutputBuffer);
				yRight_UPConvolution.ProcessUPConvolution(encoderIn, GetABIR().GetImpulseResponse_Partitioned(Y, Common::T_ear::RIGHT), rightOutputBuffer);
			}
			else {
				//Error
			}
#endif
		}
		else {
			if (reverberationOrder == ReverberationOrder::TRIDIM){
			
			}
			else
			{

			}
		}
		// Build Stereo buffer
		output.FromTwoMonosToStereo(leftOutputBuffer, rightOutputBuffer);		
	}

	//brief Calculate the BRIR again
	void CEnvironment::CalculateBRIR() 
	{
		environmentBRIR->CalculateNewBRIRTable();
	}
	//brief Reset the BRIR and ARIR tables
	void CEnvironment::ResetBRIR_ABIR() 
	{		
		environmentABIR.Reset();				//Reset ABIR of environment								
		environmentBRIR->Reset();				//Reset BRIR of enviroment			
	}
}