/*
* \class CCore
*
* \brief Definition of AmbisonicDSP class.
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
#include <BinauralSpatializer/AmbisonicDSP.h>
#include <Common/ErrorHandler.h>
#include <string>
#include <vector>

namespace Binaural {

	CAmbisonicDSP::CAmbisonicDSP(CCore* _ownerCore)
		:ownerCore{ _ownerCore }
	{
		interpolation = true;
		AmbisonicOrder = 1;
		normalization = ambisonicNormalization::N3D;
	}

	// Get Core AudioState Struct
	Common::TAudioStateStruct CAmbisonicDSP::GetCoreAudioState() const
	{
		return ownerCore->GetAudioState();
	}

	void CAmbisonicDSP::ResetAmbisonicBuffers()
	{
		if (ownerCore != nullptr)
		{
			int bufferLength = ownerCore->GetAudioState().bufferSize;
			int HRIRLength = ownerCore->GetListener()->GetHRTF()->GetHRIRLength();
			if (HRIRLength > 0)
			{

				//Prepare output buffers to perform UP convolutions in ProcessVirtualAmbisonicReverb			
				int blockLengthFreq = GetAHRBIR().GetDataBlockLength_freq();
				int numberOfBlocks = GetAHRBIR().GetDataNumberOfBlocks();

				left_UPConvolutionVector.clear();
				right_UPConvolutionVector.clear();
			
				for (int i = 0; i < GetTotalChannels(); i++) {
					std::shared_ptr<CUPCAnechoic> leftSetup = std::make_shared< CUPCAnechoic>();					
					leftSetup->Setup(bufferLength, blockLengthFreq, numberOfBlocks, true);
					left_UPConvolutionVector.push_back(leftSetup);

					std::shared_ptr<CUPCAnechoic> rightSetup = std::make_shared< CUPCAnechoic>();
					rightSetup->Setup(bufferLength, blockLengthFreq, numberOfBlocks, true);
					right_UPConvolutionVector.push_back(rightSetup);
				}
			}
		}
	}

	int CAmbisonicDSP::GetOrder()
	{
		return AmbisonicOrder;
	}

	bool CAmbisonicDSP::SetAHRBIR() 
	{
		if (ownerCore != nullptr)
		{
			int HRIRLength = ownerCore->GetListener()->GetHRTF()->GetHRIRLength();

			if (HRIRLength > 0)
			{

				//Configure AIR values (partitions and FFTs)
				bool result = CalculateAHRBIRPartitioned();

				//Prepare output buffers to perform UP convolutions in ProcessVirtualAmbisonicReverb
				ResetAmbisonicBuffers();

				return result;
			}
			else { return false; }
		}
		else { return false; }
	}

	/*bool CAmbisonicDSP::CalculateAHRBIRPartitioned()
	{
		environmentAHRBIR.Setup(ownerCore->GetAudioState().bufferSize, ownerCore->GetListener()->GetHRTF()->GetHRIRLength());
		std::vector<TImpulseResponse_Partitioned> Partitioned_Left;
		std::vector<TImpulseResponse_Partitioned> Partitioned_Right;

		std::vector<float> ambisonicAzimut = GetambisonicAzimut();
		std::vector<float> ambisonicElevation = GetambisonicElevation();

		//1. Get BRIR values for each channel
		for (int i = 0; i < GetTotalLoudspeakers(); i++) {
			std::vector<CMonoBuffer<float>> leftResponse = ownerCore->GetListener()->GetHRTF()->GetHRIR_partitioned(Common::T_ear::LEFT, ambisonicAzimut[i], ambisonicElevation[i], interpolation);
			std::vector<CMonoBuffer<float>> rightResponse = ownerCore->GetListener()->GetHRTF()->GetHRIR_partitioned(Common::T_ear::RIGHT, ambisonicAzimut[i], ambisonicElevation[i], interpolation);


			TImpulseResponse_Partitioned leftResponse_(leftResponse.begin(), leftResponse.end());
			TImpulseResponse_Partitioned rightResponse_(rightResponse.begin(), rightResponse.end());

			Partitioned_Left.push_back(leftResponse_);
			Partitioned_Right.push_back(rightResponse_);
		}

		TImpulseResponse_Partitioned left = Partitioned_Left[0];


		size_t s = left.size();

		if (s == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Buffers should be the same and not zero");
			return false;
		}

		for (int i = 0; i < GetTotalLoudspeakers(); i++) {
			TOneEarHRIRPartitionedStruct TOneEar_left;
			TOneEarHRIRPartitionedStruct TOneEar_right;

			TOneEar_left.HRIR_Partitioned = Partitioned_Left[i];
			TOneEar_right.HRIR_Partitioned = Partitioned_Right[i];

			if (Partitioned_Left[i].size() != s || Partitioned_Right[i].size() != s || ownerCore->GetListener()->GetHRTF()->IsIREmpty(TOneEar_left) || ownerCore->GetListener()->GetHRTF()->IsIREmpty(TOneEar_right)) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "Buffers should be the same and not zero");
				return false;
			}

		}

		//2. Init AIR buffers
		std::vector<TImpulseResponse_Partitioned> newAIR_right;
		std::vector<TImpulseResponse_Partitioned> newAIR_left;

		//Decoding factor
		std::vector<std::vector<float>> factors;
		factors.resize(GetTotalLoudspeakers(), std::vector<float>(GetTotalChannels()));

		DegreesToRadians(ambisonicAzimut);
		DegreesToRadians(ambisonicElevation);

		for (int i = 0; i < GetTotalLoudspeakers(); i++) {
			getRealSphericalHarmonics(ambisonicAzimut[i], ambisonicElevation[i], factors[i]);
		}

		for (int i = 0; i < GetTotalChannels(); i++) {

			TImpulseResponse_Partitioned AIR_left;
			AIR_left.resize(ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters());
			newAIR_left.push_back(AIR_left);

			TImpulseResponse_Partitioned AIR_right;
			AIR_right.resize(ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters());
			newAIR_right.push_back(AIR_right);

			for (int j = 0; j < ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters(); j++)
			{
				newAIR_left[i][j].resize(ownerCore->GetListener()->GetHRTF()->GetHRIRSubfilterLength(), 0.0f);
				newAIR_right[i][j].resize(ownerCore->GetListener()->GetHRTF()->GetHRIRSubfilterLength(), 0.0f);
			}

			//3. AIR codification from BRIR
			for (int j = 0; j < ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters(); j++)
			{
				for (int k = 0; k < ownerCore->GetListener()->GetHRTF()->GetHRIRSubfilterLength(); k++)
				{
					for (int l=0; l < GetTotalLoudspeakers(); l++)
					{
						newAIR_left[i][j][k] += Partitioned_Left[l][j][k] * factors[l][i];
						newAIR_right[i][j][k] += Partitioned_Right[l][j][k] * factors[l][i];


					}
						
				}
			}

		}

		//Setup AIR class
		for (int i = 0; i < GetTotalChannels(); i++) {

			//environmentAHRBIR.AddImpulseResponse(i, Common::T_ear::LEFT, std::move(newAIR_left[i]));
			//environmentAHRBIR.AddImpulseResponse(i, Common::T_ear::RIGHT, std::move(newAIR_right[i]));
		}

		return true;
	}*/

	bool CAmbisonicDSP::CalculateAHRBIRPartitioned()
	{
		environmentAHRBIR.Setup(ownerCore->GetAudioState().bufferSize, ownerCore->GetListener()->GetHRTF()->GetHRIRLength());
		std::vector<TOneEarHRIRPartitionedStruct> HR_left_vector;
		std::vector<TOneEarHRIRPartitionedStruct> HR_right_vector;

		std::vector<float> ambisonicAzimut = GetambisonicAzimut();
		std::vector<float> ambisonicElevation = GetambisonicElevation();

		//1. Get BRIR values for each channel
		for (int i = 0; i < GetTotalLoudspeakers(); i++) {
			TOneEarHRIRPartitionedStruct HR_left;
			TOneEarHRIRPartitionedStruct HR_right;

			HR_left.HRIR_Partitioned = ownerCore->GetListener()->GetHRTF()->GetHRIR_partitioned(Common::T_ear::LEFT, ambisonicAzimut[i], ambisonicElevation[i], interpolation);
			HR_right.HRIR_Partitioned = ownerCore->GetListener()->GetHRTF()->GetHRIR_partitioned(Common::T_ear::RIGHT, ambisonicAzimut[i], ambisonicElevation[i], interpolation);

			HR_left.delay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::LEFT, ambisonicAzimut[i], ambisonicElevation[i], interpolation);
			HR_right.delay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::RIGHT, ambisonicAzimut[i], ambisonicElevation[i], interpolation);

			HR_left_vector.push_back(HR_left);
			HR_right_vector.push_back(HR_right);
		}

		std::vector<CMonoBuffer<float>> left = HR_left_vector[0].HRIR_Partitioned;

		size_t s = left.size();

		if (s == 0)
		{
			SET_RESULT(RESULT_ERROR_BADSIZE, "Buffers should be the same and not zero");
			return false;
		}

		for (int i = 0; i < GetTotalLoudspeakers(); i++) {

			if (HR_left_vector[i].HRIR_Partitioned.size() != s || HR_right_vector[i].HRIR_Partitioned.size() != s || ownerCore->GetListener()->GetHRTF()->IsIREmpty(HR_left_vector[i]) || ownerCore->GetListener()->GetHRTF()->IsIREmpty(HR_right_vector[i])) {
				SET_RESULT(RESULT_ERROR_BADSIZE, "Buffers should be the same and not zero");
				return false;
			}

		}

		//2. Init AIR buffers
		std::vector<TOneEarHRIRPartitionedStruct> newAHRIR_left_vector;
		std::vector<TOneEarHRIRPartitionedStruct> newAHRIR_right_vector;

		//Decoding factor
		std::vector<std::vector<float>> factors;
		factors.resize(GetTotalLoudspeakers(), std::vector<float>(GetTotalChannels()));

		DegreesToRadians(ambisonicAzimut);
		DegreesToRadians(ambisonicElevation);

		for (int i = 0; i < GetTotalLoudspeakers(); i++) {
			getRealSphericalHarmonics(ambisonicAzimut[i], ambisonicElevation[i], factors[i]);
		}

		for (int i = 0; i < GetTotalChannels(); i++) {

			TOneEarHRIRPartitionedStruct AIR_left;
			AIR_left.HRIR_Partitioned.resize(ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters());
			newAHRIR_left_vector.push_back(AIR_left);

			TOneEarHRIRPartitionedStruct AIR_right;
			AIR_right.HRIR_Partitioned.resize(ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters());
			newAHRIR_right_vector.push_back(AIR_right);

			//Delay inicialization
			newAHRIR_left_vector[i].delay = 0;
			newAHRIR_right_vector[i].delay = 0;

			for (int j = 0; j < ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters(); j++)
			{
				newAHRIR_left_vector[i].HRIR_Partitioned[j].resize(ownerCore->GetListener()->GetHRTF()->GetHRIRSubfilterLength(), 0.0f);
				newAHRIR_right_vector[i].HRIR_Partitioned[j].resize(ownerCore->GetListener()->GetHRTF()->GetHRIRSubfilterLength(), 0.0f);
			}
		
			//3. AIR codification from BRIR
			for (int j = 0; j < ownerCore->GetListener()->GetHRTF()->GetHRIRNumberOfSubfilters(); j++)
			{
				for (int k = 0; k < ownerCore->GetListener()->GetHRTF()->GetHRIRSubfilterLength(); k++)
				{
					for (int l = 0; l < GetTotalLoudspeakers(); l++)
					{
						newAHRIR_left_vector[i].HRIR_Partitioned[j][k] += HR_left_vector[l].HRIR_Partitioned[j][k] * factors[l][i];
						newAHRIR_right_vector[i].HRIR_Partitioned[j][k] += HR_right_vector[l].HRIR_Partitioned[j][k] * factors[l][i];

					}

				}
			}

		}

		//Setup AIR class
		for (int i = 0; i < GetTotalChannels(); i++) {

			environmentAHRBIR.AddImpulseResponse(i, Common::T_ear::LEFT, std::move(newAHRIR_left_vector[i]));
			environmentAHRBIR.AddImpulseResponse(i, Common::T_ear::RIGHT, std::move(newAHRIR_right_vector[i]));
		}

		return true;
	}

	void CAmbisonicDSP::getRealSphericalHarmonics(float _ambisonicAzimut, float _ambisonicElevation, std::vector<float> & _factors) {

		switch (GetOrder())
		{
			case 3:
				_factors[9]  = sqrt(35/8) * pow(cos(_ambisonicElevation), 3) * sin(3 * _ambisonicAzimut);
				_factors[10] = (sqrt(105)/2) * sin(_ambisonicElevation) * pow(cos(_ambisonicElevation), 2) * sin(2* _ambisonicAzimut);
				_factors[11] = sqrt(21/8) * cos(_ambisonicElevation) * (5 * pow(sin(_ambisonicElevation), 2) - 1) * sin(_ambisonicAzimut);
				_factors[12] = (sqrt(7) / 2) * sin(_ambisonicElevation) * (5 * pow(sin(_ambisonicElevation), 2) - 3);
				_factors[13] = sqrt(21/8) * cos(_ambisonicElevation) * (5 * pow(sin(_ambisonicElevation), 2) - 1) * cos(_ambisonicAzimut);
				_factors[14] = (sqrt(105)/2) * sin(_ambisonicElevation) * pow(cos(_ambisonicElevation), 2) * cos(2 * _ambisonicAzimut);
				_factors[15] = sqrt(35 / 8) * pow(cos(_ambisonicElevation), 3) * cos(3 * _ambisonicAzimut);

			case 2:
				_factors[4] = (sqrt(15) / 2) * pow(cos(_ambisonicElevation), 2) * sin(2 *_ambisonicAzimut) ;
				_factors[5] = (sqrt(15) / 2) * sin(2*_ambisonicElevation) * sin(_ambisonicAzimut);
				_factors[6] = (sqrt(5) / 2) * (3 * pow(sin(_ambisonicElevation), 2) - 1);
				_factors[7] = (sqrt(15) / 2) * sin(2*_ambisonicElevation) * cos(_ambisonicAzimut);
				_factors[8] = (sqrt(15) / 2) * pow(cos(_ambisonicElevation), 2) * cos(2 * _ambisonicAzimut);


			case 1:
				_factors[0] = 1;
				_factors[1] = sqrt(3) * cos(_ambisonicElevation) * sin(_ambisonicAzimut);
				_factors[2] = sqrt(3) * sin(_ambisonicElevation);
				_factors[3] = sqrt(3) * cos(_ambisonicElevation) * cos(_ambisonicAzimut);
			default:
				break;

		}

		if (normalization == ambisonicNormalization::SN3D) { convertN3DtoSN3D(_factors); }
		else if (normalization == ambisonicNormalization::maxN) { convertN3DtoMaxN(_factors); }

	}

	void CAmbisonicDSP::convertN3DtoSN3D(std::vector<float>& _factors) {
		for (int i = 1; i < _factors.size(); i++) {
			if (i < 4) { _factors[i] *= (1 / sqrt(3)); }
			else if (i < 9) { _factors[i] *= (1 / sqrt(5)); }
			else if (i < 16) { _factors[i] *= (1 / sqrt(7)); }
		}
	}

	void CAmbisonicDSP::convertN3DtoMaxN(std::vector<float>& _factors) {
		switch (GetOrder())
		{
		case 3:
			_factors[9] *= sqrt(8 / 35);
			_factors[10] *= 3 / sqrt(35);
			_factors[11] *= sqrt(45 / 224);
			_factors[12] *= 1 / sqrt(7);
			_factors[13] *= sqrt(45 / 224);
			_factors[14] *= 3 / sqrt(35);
			_factors[15] *= sqrt(8 / 35);

		case 2:
			_factors[4] *= 2 / sqrt(15);
			_factors[5] *= 2 / sqrt(15);
			_factors[6] *= 1 / sqrt(5);
			_factors[7] *= 2 / sqrt(15);
			_factors[8] *= 2 / sqrt(15);


		case 1:
			_factors[0] *= 1 / sqrt(2);
			_factors[1] *= 1 / sqrt(3);
			_factors[2] *= 1 / sqrt(3);
			_factors[3] *= 1 / sqrt(3);
		default:
			break;

		}
	}

	//////////////////////////////////////////////
		// Get AHRBIR for anechoic
	const CAHRBIR& CAmbisonicDSP::GetAHRBIR() const
	{
		return environmentAHRBIR;
	}

	//////////////////////////////////////////////

	// Process virtual ambisonic anechoic for specified buffers
	void CAmbisonicDSP::ProcessVirtualAmbisonicAnechoic(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames)
	{
		if (!environmentAHRBIR.IsInitialized())
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Data is not ready to be processed");
			return;
		}

		// Check outbuffers size
		if (outBufferLeft.size() != 0 || outBufferRight.size() != 0) {
			outBufferLeft.clear();
			outBufferRight.clear();
			SET_RESULT(RESULT_ERROR_BADSIZE, "outBufferLeft and outBufferRight were expected to be empty, they will be cleared. CAmbisonicDSP::ProcessVirtualAmbisonicAnechoic");
		}

		// This would crash if there are no sources created. Rather than reporting error, do nothing
		if (ownerCore->audioSources.size() == 0)
			return;

		std::vector< CMonoBuffer<float> > codAmbisonic_left;
		std::vector< CMonoBuffer<float> > codAmbisonic_right;
		std::vector< CMonoBuffer<float> > codAmbisonic;

		std::vector < CMonoBuffer<float> > Ahrbir_left;
		std::vector < CMonoBuffer<float> > Ahrbir_right;

		CMonoBuffer<float> mixerOutput_left;
		CMonoBuffer<float> mixerOutput_right;

		// We assume all buffers have the same number of samples
		size_t samplesInBuffer = ownerCore->GetAudioState().bufferSize;

		//To initialize the sumation channels
		bool initializedBuffer = false;

		/////////////////////////////////////////
		//      Virtual Ambisonics Encoder
		/////////////////////////////////////////
		// 

		// Go through each source
		for (auto eachSource : ownerCore->audioSources)
		{
			if (eachSource->GetSpatializationMode() != TSpatializationMode::Ambisonic)
				continue;

			if (!eachSource->IsAnechoicProcessReady()) {
				SET_RESULT(RESULT_WARNING, "Attempt to do anechoic process without updating source buffer; please call to SetBuffer before ProcessAnechoic.");
				continue;
			}
			
			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization to this source
			if (eachSource->GetCurrentDistanceSourceListener() < ownerCore->GetListener()->GetHeadRadius())
				continue;

			//Check if the source is muted
			if (!eachSource->enableAnechoic)
				continue;

			/// Return next buffer frame after pass throught the waveguide

			CMonoBuffer<float> sourceBuffer;
			CMonoBuffer<float> outBufferLeft;
			CMonoBuffer<float> outBufferRight;

			Common::CVector3  vectorToListener;
			float distanceToListener;
			float centerElevation; 
			float centerAzimuth; 
			float interauralAzimuth;

			float leftElevation;
			float leftAzimuth;
			float rightElevation; 
			float rightAzimuth;

			Common::CVector3 SourcePosition;
			Common::CTransform SourceTransform;


			eachSource->GetEffectiveBuffer(sourceBuffer, SourcePosition);

			if (eachSource->channelToListener.IsPropagationDelayEnabled()) {
				/*CalculateSourceCoordinates(effectiveSourceTransform, vectorToListener, distanceToListener, centerElevation, centerAzimuth, interauralAzimuth, leftElevation, leftAzimuth, rightElevation, rightAzimuth); */
				SourceTransform.SetPosition(SourcePosition);
			}
			else { 
				SourceTransform = eachSource->GetCurrentSourceTransform();
			}

			eachSource->CalculateSourceCoordinates(SourceTransform, vectorToListener, distanceToListener, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);

			DegreesToRadians(centerElevation);
			DegreesToRadians(centerAzimuth);
			
			//Apply Far distance effect
			if (eachSource->IsFarDistanceEffectEnabled()) { eachSource->ProcessFarDistanceEffect(sourceBuffer, distanceToListener); }

			// Apply distance attenuation
			if (eachSource->IsDistanceAttenuationEnabledAnechoic()) { eachSource->ProcessDistanceAttenuationAnechoic(sourceBuffer, ownerCore->GetAudioState().bufferSize, ownerCore->GetAudioState().sampleRate, distanceToListener); }

			//Degress
			uint64_t leftDelay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::LEFT, leftAzimuth, leftElevation, interpolation);
			uint64_t rightDelay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::RIGHT, rightAzimuth, rightElevation, interpolation);

			//Radians
			DegreesToRadians(leftElevation);
			DegreesToRadians(leftAzimuth);
			DegreesToRadians(rightElevation);
			DegreesToRadians(rightAzimuth);

			eachSource->ProcessAddDelay_ExpansionMethod(sourceBuffer, outBufferLeft, eachSource->leftChannelDelayBuffer, leftDelay);
			eachSource->ProcessAddDelay_ExpansionMethod(sourceBuffer, outBufferRight, eachSource->rightChannelDelayBuffer, rightDelay);

			// Apply Near field effects (ILD)
			eachSource->ProcessNearFieldEffect(outBufferLeft, outBufferRight, distanceToListener, interauralAzimuth);											

			size_t samplesInBufferLeft = outBufferLeft.size();
			size_t samplesInBufferRight = outBufferRight.size();

			// Init sumation channels
			if (!initializedBuffer) {
				for (int i = 0; i < GetTotalChannels(); i++)
				{
					CMonoBuffer<float> emptyChannels_left;
					emptyChannels_left.Fill(samplesInBufferLeft, 0.0f);
					codAmbisonic_left.push_back(emptyChannels_left);

					CMonoBuffer<float> emptyChannels_right;
					emptyChannels_right.Fill(samplesInBufferRight, 0.0f);
					codAmbisonic_right.push_back(emptyChannels_right);

				}
				initializedBuffer = true;
			}
			

			// Get azimuth, elevation and distance from listener to each source
			// We precompute everything, to minimize per-sample computations. 
			std::vector<float> factors_left(GetTotalChannels());
			std::vector<float> factors_right(GetTotalChannels());
				

			getRealSphericalHarmonics(leftAzimuth, leftElevation, factors_left);
			getRealSphericalHarmonics(rightAzimuth, rightElevation, factors_right);


			// Go trough each sample [left]
			for (int nSample = 0; nSample < samplesInBufferLeft; nSample++)
			{
				// Value from the input buffer				
				float newSample = outBufferLeft[nSample];

				// Add partial contribution of this source to each channel								
				for (int i = 0; i < GetTotalChannels(); i++) {
					codAmbisonic_left[i][nSample] += newSample * factors_left[i];
				}
			}

			// Go trough each sample [right]
			for (int nSample = 0; nSample < samplesInBufferRight; nSample++)
			{
				// Value from the input buffer				
				float newSample = outBufferRight[nSample];

				// Add partial contribution of this source to each channel								
				for (int i = 0; i < GetTotalChannels(); i++) {
					codAmbisonic_right[i][nSample] += newSample * factors_right[i];
				}
			}
			
			
			eachSource->readyForAnechoic = false;	// Mark the buffer as already used for anechoic process

		}

		///////////////////////////////////////////
		// Frequency-Domain Convolution with ABIR
		///////////////////////////////////////////	

		///Apply UPC algorithm
		for (int i = 0; i < GetTotalChannels(); i++) {

			CMonoBuffer<float> new_Ahrbir_left_FFT_withoutDelay;
			CMonoBuffer<float> new_Ahrbir_right_FFT_withoutDelay;

			CMonoBuffer<float> new_Ahrbir_left_FFT;
			CMonoBuffer<float> new_Ahrbir_right_FFT;

			TOneEarHRIRPartitionedStruct TOneEar_left;
			TOneEarHRIRPartitionedStruct TOneEar_right;

			TOneEar_left = GetAHRBIR().GetImpulseResponse_Partitioned(i, Common::T_ear::LEFT);
			TOneEar_right = GetAHRBIR().GetImpulseResponse_Partitioned(i, Common::T_ear::RIGHT);

			left_UPConvolutionVector[i]->ProcessUPConvolutionWithMemory(codAmbisonic_left[i], TOneEar_left, new_Ahrbir_left_FFT);
			Ahrbir_left.push_back(new_Ahrbir_left_FFT);

			right_UPConvolutionVector[i]->ProcessUPConvolutionWithMemory(codAmbisonic_right[i], TOneEar_right, new_Ahrbir_right_FFT);
			Ahrbir_right.push_back(new_Ahrbir_right_FFT);
		}
	
		///////////////////////////////////////
		// Mix of channels in Frequency domain
		///////////////////////////////////////
	    mixerOutput_right = mixChannels(Ahrbir_right);
		mixerOutput_left = mixChannels(Ahrbir_left);


		//////////////////////////////////////////////
		// Move channels to output buffers
		//////////////////////////////////////////////			
		outBufferLeft = std::move(mixerOutput_left);			//To use in C++11
		outBufferRight = std::move(mixerOutput_right);			//To use in C++11			

		// WATCHER
		WATCH(WV_ENVIRONMENT_OUTPUT_LEFT, outBufferLeft, CMonoBuffer<float>);
		WATCH(WV_ENVIRONMENT_OUTPUT_RIGHT, outBufferRight, CMonoBuffer<float>);
	}

	void CAmbisonicDSP::ProcessVirtualAmbisonicISM(CMonoBuffer<float>& outBufferLeft, CMonoBuffer<float>& outBufferRight, vector<CSingleSourceDSP>& virtualSources, int numberOfSilencedFrames)
	{
		if (!environmentAHRBIR.IsInitialized())
		{
			SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "Data is not ready to be processed");
			return;
		}

		// Check outbuffers size
		if (outBufferLeft.size() != 0 || outBufferRight.size() != 0) {
			outBufferLeft.clear();
			outBufferRight.clear();
			SET_RESULT(RESULT_ERROR_BADSIZE, "outBufferLeft and outBufferRight were expected to be empty, they will be cleared. CAmbisonicDSP::ProcessVirtualAmbisonicAnechoic");
		}

		// This would crash if there are no sources created. Rather than reporting error, do nothing
		if (ownerCore->audioSources.size() == 0)
			return;

		std::vector< CMonoBuffer<float> > codAmbisonic_left;
		std::vector< CMonoBuffer<float> > codAmbisonic_right;
		std::vector< CMonoBuffer<float> > codAmbisonic;

		std::vector < CMonoBuffer<float> > Ahrbir_left;
		std::vector < CMonoBuffer<float> > Ahrbir_right;

		CMonoBuffer<float> mixerOutput_left;
		CMonoBuffer<float> mixerOutput_right;

		// We assume all buffers have the same number of samples
		size_t samplesInBuffer = ownerCore->GetAudioState().bufferSize;

		//To initialize the sumation channels
		bool initializedBuffer = false;

		/////////////////////////////////////////
		//      Virtual Ambisonics Encoder
		/////////////////////////////////////////
		// 

		// Go through each source
		for (auto eachSource : virtualSources)
		{

			if (!eachSource.IsAnechoicProcessReady()) {
				SET_RESULT(RESULT_WARNING, "Attempt to do anechoic process without updating source buffer; please call to SetBuffer before ProcessAnechoic.");
				continue;
			}

			//Check if the source is in the same position as the listener head. If yes, do not apply spatialization to this source
			if (eachSource.GetCurrentDistanceSourceListener() < ownerCore->GetListener()->GetHeadRadius())
				continue;

			/// Return next buffer frame after pass throught the waveguide

			CMonoBuffer<float> sourceBuffer;
			CMonoBuffer<float> outBufferLeft;
			CMonoBuffer<float> outBufferRight;

			Common::CVector3  vectorToListener;
			float distanceToListener;
			float centerElevation;
			float centerAzimuth;
			float interauralAzimuth;

			float leftElevation;
			float leftAzimuth;
			float rightElevation;
			float rightAzimuth;

			Common::CVector3 SourcePosition;
			Common::CTransform SourceTransform;


			eachSource.GetEffectiveBuffer(sourceBuffer, SourcePosition);

			if (eachSource.channelToListener.IsPropagationDelayEnabled()) {
				/*CalculateSourceCoordinates(effectiveSourceTransform, vectorToListener, distanceToListener, centerElevation, centerAzimuth, interauralAzimuth, leftElevation, leftAzimuth, rightElevation, rightAzimuth); */
				SourceTransform.SetPosition(SourcePosition);
			}
			else {
				SourceTransform = eachSource.GetCurrentSourceTransform();
			}

			eachSource.CalculateSourceCoordinates(SourceTransform, vectorToListener, distanceToListener, leftElevation, leftAzimuth, rightElevation, rightAzimuth, centerElevation, centerAzimuth, interauralAzimuth);

			DegreesToRadians(centerElevation);
			DegreesToRadians(centerAzimuth);

			//Apply Far distance effect
			if (eachSource.IsFarDistanceEffectEnabled()) { eachSource.ProcessFarDistanceEffect(sourceBuffer, distanceToListener); }

			// Apply distance attenuation
			if (eachSource.IsDistanceAttenuationEnabledAnechoic()) { eachSource.ProcessDistanceAttenuationAnechoic(sourceBuffer, ownerCore->GetAudioState().bufferSize, ownerCore->GetAudioState().sampleRate, distanceToListener); }

			//Degress
			uint64_t leftDelay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::LEFT, leftAzimuth, leftElevation, interpolation);
			uint64_t rightDelay = ownerCore->GetListener()->GetHRTF()->GetHRIRDelay(Common::T_ear::RIGHT, rightAzimuth, rightElevation, interpolation);

			//Radians
			DegreesToRadians(leftElevation);
			DegreesToRadians(leftAzimuth);
			DegreesToRadians(rightElevation);
			DegreesToRadians(rightAzimuth);

			eachSource.ProcessAddDelay_ExpansionMethod(sourceBuffer, outBufferLeft, eachSource.leftChannelDelayBuffer, leftDelay);
			eachSource.ProcessAddDelay_ExpansionMethod(sourceBuffer, outBufferRight, eachSource.rightChannelDelayBuffer, rightDelay);

			// Apply Near field effects (ILD)
			eachSource.ProcessNearFieldEffect(outBufferLeft, outBufferRight, distanceToListener, interauralAzimuth);

			size_t samplesInBufferLeft = outBufferLeft.size();
			size_t samplesInBufferRight = outBufferRight.size();

			// Init sumation channels
			if (!initializedBuffer) {
				for (int i = 0; i < GetTotalChannels(); i++)
				{
					CMonoBuffer<float> emptyChannels_left;
					emptyChannels_left.Fill(samplesInBufferLeft, 0.0f);
					codAmbisonic_left.push_back(emptyChannels_left);

					CMonoBuffer<float> emptyChannels_right;
					emptyChannels_right.Fill(samplesInBufferRight, 0.0f);
					codAmbisonic_right.push_back(emptyChannels_right);

				}
				initializedBuffer = true;
			}
			

			// Get azimuth, elevation and distance from listener to each source
			// We precompute everything, to minimize per-sample computations. 
			std::vector<float> factors_left(GetTotalChannels());
			std::vector<float> factors_right(GetTotalChannels());


			getRealSphericalHarmonics(leftAzimuth, leftElevation, factors_left);
			getRealSphericalHarmonics(rightAzimuth, rightElevation, factors_right);


			// Go trough each sample [left]
			for (int nSample = 0; nSample < samplesInBufferLeft; nSample++)
			{
				// Value from the input buffer				
				float newSample = outBufferLeft[nSample];

				// Add partial contribution of this source to each channel								
				for (int i = 0; i < GetTotalChannels(); i++) {
					codAmbisonic_left[i][nSample] += newSample * factors_left[i];
				}
			}

			// Go trough each sample [right]
			for (int nSample = 0; nSample < samplesInBufferRight; nSample++)
			{
				// Value from the input buffer				
				float newSample = outBufferRight[nSample];

				// Add partial contribution of this source to each channel								
				for (int i = 0; i < GetTotalChannels(); i++) {
					codAmbisonic_right[i][nSample] += newSample * factors_right[i];
				}
			}


			eachSource.readyForAnechoic = false;	// Mark the buffer as already used for anechoic process

		}

		///////////////////////////////////////////
		// Frequency-Domain Convolution with ABIR
		///////////////////////////////////////////	

		///Apply UPC algorithm
		for (int i = 0; i < GetTotalChannels(); i++) {

			CMonoBuffer<float> new_Ahrbir_left_FFT_withoutDelay;
			CMonoBuffer<float> new_Ahrbir_right_FFT_withoutDelay;

			CMonoBuffer<float> new_Ahrbir_left_FFT;
			CMonoBuffer<float> new_Ahrbir_right_FFT;

			TOneEarHRIRPartitionedStruct TOneEar_left;
			TOneEarHRIRPartitionedStruct TOneEar_right;

			TOneEar_left = GetAHRBIR().GetImpulseResponse_Partitioned(i, Common::T_ear::LEFT);
			TOneEar_right = GetAHRBIR().GetImpulseResponse_Partitioned(i, Common::T_ear::RIGHT);

			left_UPConvolutionVector[i]->ProcessUPConvolutionWithMemory(codAmbisonic_left[i], TOneEar_left, new_Ahrbir_left_FFT);
			Ahrbir_left.push_back(new_Ahrbir_left_FFT);

			right_UPConvolutionVector[i]->ProcessUPConvolutionWithMemory(codAmbisonic_right[i], TOneEar_right, new_Ahrbir_right_FFT);
			Ahrbir_right.push_back(new_Ahrbir_right_FFT);
		}

		///////////////////////////////////////
		// Mix of channels in Frequency domain
		///////////////////////////////////////
		mixerOutput_right = mixChannels(Ahrbir_right);
		mixerOutput_left = mixChannels(Ahrbir_left);


		//////////////////////////////////////////////
		// Move channels to output buffers
		//////////////////////////////////////////////			
		outBufferLeft = std::move(mixerOutput_left);			//To use in C++11
		outBufferRight = std::move(mixerOutput_right);			//To use in C++11			

		// WATCHER
		WATCH(WV_ENVIRONMENT_OUTPUT_LEFT, outBufferLeft, CMonoBuffer<float>);
		WATCH(WV_ENVIRONMENT_OUTPUT_RIGHT, outBufferRight, CMonoBuffer<float>);
	}

	CMonoBuffer<float> CAmbisonicDSP::mixChannels(std::vector< CMonoBuffer<float>> Ahrbir_FFT) {
		
		// Get size of all sourceBuffers and check they are the same
		size_t bufferSize = 0;
		for (std::vector<CMonoBuffer<float>>::iterator it = Ahrbir_FFT.begin(); it != Ahrbir_FFT.end(); ++it)
		{
			if (bufferSize == 0)
				bufferSize = (*it).size();
			ASSERT((*it).size() == bufferSize, RESULT_ERROR_BADSIZE, "Attempt to mix buffers with different sizes", "");
		}

		CMonoBuffer<float> mixerOutput_FFT;
		// Iterate through all samples
		for (int i = 0; i < bufferSize; i++)
		{
			// Iterate through all source buffers
			float sum = 0.0f;
			for (std::vector<CMonoBuffer<float>>::iterator it = Ahrbir_FFT.begin(); it != Ahrbir_FFT.end(); ++it)
			{
				sum += (*it)[i];
			}
			sum = sum * (1.0 / GetTotalChannels());
			mixerOutput_FFT.push_back(sum);
		}
		return mixerOutput_FFT;
	}


	// Process virtual ambisonic anechoic for specified buffers (LISTO)
	void CAmbisonicDSP::ProcessVirtualAmbisonicAnechoic(CStereoBuffer<float> & outBuffer, int numberOfSilencedFrames)
	{
		CMonoBuffer<float> outLeftBuffer;
		CMonoBuffer<float> outRightBuffer;
		ProcessVirtualAmbisonicAnechoic(outLeftBuffer, outRightBuffer, numberOfSilencedFrames);
		outBuffer.Interlace(outLeftBuffer, outRightBuffer);
	}
	//////////////////////////////////////////////

	void CAmbisonicDSP::SetOrder(int _order)
	{
		AmbisonicOrder = _order;
		ResetAHRBIR();
	}

	int CAmbisonicDSP::GetTotalChannels()
	{
		return pow((AmbisonicOrder + 1),2);
	}

	//brief Calculate the HRTF again
	void CAmbisonicDSP::CalculateHRTF()
	{
		ownerCore->GetListener()->GetHRTF()->CalculateNewHRTFTable();
		SetAHRBIR();
	}

	//brief Reset the HRTF and AHRBIR tables
	void CAmbisonicDSP::ResetAHRBIR()
	{
		ResetAmbisonicBuffers();
		//SetDelayBuffers();
		environmentAHRBIR.Reset();				//Reset ABIR of environment		
		CalculateAHRBIRPartitioned();
	}

	int CAmbisonicDSP::GetTotalLoudspeakers() 
	{
		if (AmbisonicOrder == 1) { return 6; }
		else if (AmbisonicOrder == 2) { return 12; }
		else { return 20; }
	}

	std::vector<float> CAmbisonicDSP::GetambisonicAzimut()
	{
		if (AmbisonicOrder == 1) { return ambisonicAzimut_order1; }
		else if (AmbisonicOrder == 2) { return ambisonicAzimut_order2; }
		else { return ambisonicAzimut_order3; }
	}

	std::vector<float> CAmbisonicDSP::GetambisonicElevation()
	{
		if (AmbisonicOrder == 1) { return ambisonicElevation_order1; }
		else if (AmbisonicOrder == 2) { return ambisonicElevation_order2; }
		else { return ambisonicElevation_order3; }
	}

	void CAmbisonicDSP::DegreesToRadians(std::vector<float> & _degrees) {
		for (int i = 0; i < _degrees.size(); i++) {
			_degrees[i] = (_degrees[i] * PI) / 180;
		}
	}

	void CAmbisonicDSP::DegreesToRadians(float & _degrees) {
			_degrees = (_degrees * PI) / 180;
	}
	
	void CAmbisonicDSP::SetInterpolation(bool _interpolation) {
		interpolation = _interpolation;
		ResetAHRBIR();
	}

	bool CAmbisonicDSP::GetInterpolation() {
		return interpolation;
	}

}