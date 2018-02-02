/**
*
* \brief Functions to handle BRIR
*
* \date	March 2017
*
* \authors 3DI - DIANA Research Group(University of Malaga), in alphabetical order : M.Cuevas - Rodriguez, C.Garre, D.Gonzalez - Toledo, E.J.de la Rubia - Cuestas, L.Molina - Tanco ||
*Coordinated by, A.Reyes - Lecuona(University of Malaga) and L.Picinali(Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Contributions : (additional authors / contributors can be added here)
*
* \b Project : 3DTI(3D - games for TUNing and lEarnINg about hearing aids) ||
*\b Website : http://3d-tune-in.eu/
*
* \b Copyright : University of Malaga and Imperial College London - 2018
*
* \b Licence : This copy of 3dti_AudioToolkit is licensed to you under the terms described in the 3DTI_AUDIOTOOLKIT_LICENSE file included in this distribution.
*
* \b Acknowledgement : This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement No 644051
*/

#include <ostream>
#include "BRIRFactory.h"
#include <BinauralSpatializer/BRIR.h>
#include <Common/ErrorHandler.h>
#include <SOFA.h>
#include <SOFAExceptions.h>
#include <math.h>

#define NORTH_AZIMUTH 0
#define SOUTH_AZIMUTH 180
#define WEST_AZIMUTH 90
#define EAST_AZIMUTH 270
#define MAX_AZIMUTH 360
#define MAX_ANGLE_ERROR 5
// TO DO: zenith, nadir...

//static inline const std::size_t array4DIndex(const unsigned long i,
//	const unsigned long j,
//	const unsigned long k,
//	const unsigned long l,
//	const unsigned long dim1,
//	const unsigned long dim2,
//	const unsigned long dim3,
//	const unsigned long dim4)
//{
//	return dim2 * dim3 * dim4 * i + dim3 * dim4 * j + dim4 * k + l;
//}

static inline const std::size_t array3DIndex(const unsigned long i,
                                             const unsigned long j,
                                             const unsigned long k,
                                             const unsigned long dim1,
                                             const unsigned long dim2,
                                             const unsigned long dim3)
{
    return dim2 * dim3 * i + dim3 * j + k;
}


static inline const std::size_t array2DIndex(const unsigned long i,
                                             const unsigned long j,
                                             const unsigned long dim1,
                                             const unsigned long dim2)
{
    return dim2 * i + j;
}

int TurnNegativeAngleToPositiveInDegrees(int angle)
{
	int newAngle = angle;
	
	newAngle += 360;
	if (newAngle < 0)
		return TurnNegativeAngleToPositiveInDegrees(newAngle);

	return newAngle;
}

bool AnglesAreCloseInDegrees(float a1, float a2)
{		
	// We ignore <1º precission 
	int a1int = (int)a1;
	int a2int = (int)a2;

	// Convert to positive	
	if (a1int < 0)
		a1int = TurnNegativeAngleToPositiveInDegrees(a1int);	
	if (a2int < 0)
		a2int = TurnNegativeAngleToPositiveInDegrees(a2int);

	// Avoid angles greater than 2_PI
	a1int = a1int % MAX_AZIMUTH;
	a2int = a2int % MAX_AZIMUTH;

	// Make comparison
	return (std::abs(a1int - a2int) < MAX_ANGLE_ERROR);
}

float RadiansToDegrees(float angleRadians)
{
	return (angleRadians * 180.0f) / M_PI;
}

namespace BRIR
{
	int GetSampleRateFromSofa(const std::string & sofafile)
	{
		try {
			// Check file open and format
			const sofa::File theFile(sofafile);
			if (!theFile.IsValid())
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
				return -1;
			}

			// Check SOFA file type
			const sofa::SimpleFreeFieldHRIR ir(sofafile);
			if (!ir.IsValid())
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SimpleFreeFieldHRIR file");
				return -1;
			}

			// Check units of sampling rate. It must be Herzs
			sofa::Units::Type srUnits;
			ir.GetSamplingRateUnits(srUnits);
			if (srUnits != sofa::Units::Type::kHertz)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling rate units are not Herzs");
				return -1;
			}

			// Get sample rate and cast to int (current type in 3DTi Toolkit core)
			double samplingRate;
			ir.GetSamplingRate(samplingRate);
			return (int)samplingRate;
		}
		catch (sofa::Exception &e)
		{
			// the description of the exception will be printed when raised
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
		}
		catch (std::exception &e)
		{
			std::string s("Error when reading BRIR");
			s += e.what();
			SET_RESULT(RESULT_ERROR_UNKNOWN, s.c_str());
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading BRIR");
		}
	}

	//////////////////////////////////////////////////////////////////////
		
	bool CreateFromSofa(const std::string & sofafile, shared_ptr<Binaural::CEnvironment> environment)
	{
		if (LoadBRIRTableFromSOFA(sofafile, environment)) 
		{
			environment->GetBRIR()->EndSetup();
			return true;
		}
		else 
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception creating BRIR, please consider previous messages from the sofa library");
			return false;
		}
	}

	bool Create3DTIFromSofa(const std::string & sofafile, shared_ptr<Binaural::CEnvironment> environment)
	{
		if (!LoadBRIRTableFromSOFA(sofafile, environment)) 
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception creating BRIR, please consider previous messages from the sofa library");
			return false;
		}
		return true;
	}

	bool LoadBRIRTableFromSOFA(const std::string & sofafile, shared_ptr<Binaural::CEnvironment> environment) 	
	{
		std::ostream & output = std::cout;
		try {

			const sofa::File theFile(sofafile);
			if (!theFile.IsValid())
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
				return false;
			}
			SET_RESULT(RESULT_OK, "Valid SOFA file");

			const sofa::SimpleFreeFieldHRIR ir(sofafile);
			if (!ir.IsValid())
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SimpleFreeFieldHRIR file");
				return false;
			}

			std::vector< std::size_t > dims;
			ir.GetVariableDimensions(dims, "SourcePosition");
			if (dims.size() != 2)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA File gives invalid number of dimensions for Source Positions"); // TODO: TO_STRING(dims.size());
				return false;
			}

			std::vector< double > pos;
			pos.resize(dims[0] * dims[1]);
			ir.GetSourcePosition(&pos[0], dims[0], dims[1]); // dims[0] is the number of positions (6, say), dims[1] is the dimensions of the positions (3).
			const unsigned int nMeasurements = (unsigned int)ir.GetNumMeasurements();  // The number of HRIRs (6, say)
			if (dims[0] != nMeasurements)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA gives incoherent number of source positions and measurements");
				return false;
			}

			// const unsigned int nReceivers = (unsigned int) hrir.GetNumReceivers();     // Currently unused. Should be two receivers (stereo)

			std::vector< double > delays;
			ir.GetDataDelay(delays);

			std::vector< double > data;
			ir.GetDataIR(data);

			// Prepare BRIR
			const unsigned int nSamples = (unsigned int)ir.GetNumDataSamples();   // For example: 512 samples.
			environment->GetBRIR()->BeginSetup(nSamples);

			//// DEBUG:
			//double maxData = 0.0;
			//for (int i = 0; i < 136130; i++)
			//{
			//	if (data[i] > maxData)
			//	{
			//		maxData = data[i];
			//		int source = i / (11345*2);
			//		string channelstr = "";
			//		if ((i % (11345*2)) < 11345)
			//			channelstr = "Left";
			//		else
			//			channelstr = "Right";
			//		int sample = i % 11345;
			//		output << "Max data = " << maxData << " at " << i << "(Source " << source << ", Channel " << channelstr << ", sample " << sample << ")" << std::endl;
			//	}
			//}
			////

			// This outtermost loop iterates over IRs
			for (std::size_t i = 0; i < nMeasurements; i++) // or for( std::size_t i = 0; i < dims[0]; i++ ), should be the same.
			{
				TImpulseResponse leftBRIRChannel;
				TImpulseResponse rightBRIRChannel;
				leftBRIRChannel.resize(nSamples);
				rightBRIRChannel.resize(nSamples);

				//hrir_value.leftDelay = delays[array2DIndex(i, left_ear, nMeasurements, 2)];	// TO DO: What to do with delay?
				for (std::size_t k = 0; k < nSamples; k++)
				{
					// Source, Receiver, Sample
					const std::size_t index = array3DIndex(i, Common::T_ear::LEFT, k, nMeasurements, 2, nSamples);
					leftBRIRChannel[k] = static_cast<float>(data[index]);
				}

				//hrir_value.rightDelay = delays[array2DIndex(i, right_ear, nMeasurements, 2)];	// TO DO: What to do with delay?
				for (std::size_t k = 0; k < nSamples; k++)
				{
					const std::size_t index = array3DIndex(i, Common::T_ear::RIGHT, k, nMeasurements, 2, nSamples);
					rightBRIRChannel[k] = static_cast<float>(data[index]);
				}

				double azimuth = pos[array2DIndex(i, 0, nMeasurements, dims[1])];
				double elevation = pos[array2DIndex(i, 1, nMeasurements, dims[1])];
				while (elevation < 0) elevation += 360; // TODO: check who should do this

														// SET VIRTUAL SPEAKER

				if ((AnglesAreCloseInDegrees(azimuth, NORTH_AZIMUTH)) && (AnglesAreCloseInDegrees(elevation, 0.0f)))
				{
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::NORTH, Common::T_ear::LEFT, std::move(leftBRIRChannel));
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::NORTH, Common::T_ear::RIGHT, std::move(rightBRIRChannel));
				}
				if ((AnglesAreCloseInDegrees(azimuth, SOUTH_AZIMUTH)) && (AnglesAreCloseInDegrees(elevation, 0.0f)))
				{
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::SOUTH, Common::T_ear::LEFT, std::move(leftBRIRChannel));
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::SOUTH, Common::T_ear::RIGHT, std::move(rightBRIRChannel));
				}
				if ((AnglesAreCloseInDegrees(azimuth, WEST_AZIMUTH)) && (AnglesAreCloseInDegrees(elevation, 0.0f)))
				{
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::WEST, Common::T_ear::LEFT, std::move(leftBRIRChannel));
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::WEST, Common::T_ear::RIGHT, std::move(rightBRIRChannel));
				}
				if ((AnglesAreCloseInDegrees(azimuth, EAST_AZIMUTH)) && (AnglesAreCloseInDegrees(elevation, 0.0f)))
				{
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::EAST, Common::T_ear::LEFT, std::move(leftBRIRChannel));
					environment->GetBRIR()->AddBRIR(VirtualSpeakerPosition::EAST, Common::T_ear::RIGHT, std::move(rightBRIRChannel));
				}
				// TO DO: consider elevations. Read zenith and nadir speakers
			}
			return true;
		}
		catch (sofa::Exception &e)
		{
			// the description of the exception will be printed when raised
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
		}
		catch (std::exception &e)
		{
			std::string s("Error when creating BRIR representation");
			s += e.what();
			SET_RESULT(RESULT_ERROR_UNKNOWN, s.c_str());
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when creating BRIR representation");
		}
		return false;
	}
}