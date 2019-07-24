/**
*
* \brief Functions to handle HRTFs
*
* \date March 2016
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
#include "HRTFFactory.h"
#include <BinauralSpatializer/HRTF.h>
#include <Common/ErrorHandler.h>
#include <SOFA.h>
#include <SOFAExceptions.h>

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

namespace HRTF
{	
	int GetSampleRateFromSofa(const std::string & sofafile)
	{
		try
		{
			// Check file open and format
			const sofa::File theFile(sofafile);
			if (!theFile.IsValid())
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SOFA file");
				return -1;
			}

			// Check SOFA file type
			const sofa::SimpleFreeFieldHRIR hrir(sofafile);
			if (!hrir.IsValid())
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SimpleFreeFieldHRIR file");
				return -1;
			}

			// Check units of sampling rate. It must be Herzs
			sofa::Units::Type srUnits;
			hrir.GetSamplingRateUnits(srUnits);
			if (srUnits != sofa::Units::Type::kHertz)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Sampling rate units are not Herzs");
				return -1;
			}

			// Get sample rate and cast to int (current type in 3DTi Toolkit core)
			double samplingRate;
			hrir.GetSamplingRate(samplingRate);
			return (int)samplingRate;
		}		
		catch (sofa::Exception &e)
		{
			// the description of the exception will be printed when raised
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
		}
		catch (std::exception &e)
		{
			std::string s("Error when reading samplerate from SOFA");
			s += e.what();
			SET_RESULT(RESULT_ERROR_UNKNOWN, s.c_str());
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when reading samplerate from SOFA");
		}
		return -1;
	}

	//////////////////////////////////////////////////////////////////////

	bool CreateFromSofa(const std::string & sofafile, shared_ptr<Binaural::CListener> listener, bool & specifiedDelays)
	{
		if (LoadHRTFTableFromSOFA(sofafile, listener, specifiedDelays))
		{
			listener->GetHRTF()->EndSetup();
			return true;
		}
		else
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception creating HRTF, please consider previous messages from the sofa library");
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////

	bool Create3DTIFromSofa(const std::string & sofafile, shared_ptr<Binaural::CListener> listener, bool & specifiedDelays)
	{
		if (!LoadHRTFTableFromSOFA(sofafile, listener, specifiedDelays))
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception creating HRTF, please consider previous messages from the sofa library");		
			return false;
		}		
		return true;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	bool LoadHRTFTableFromSOFA(const std::string & sofafile, shared_ptr<Binaural::CListener> listener, bool & specifiedDelays)
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

			const sofa::SimpleFreeFieldHRIR hrir(sofafile);
			if (!hrir.IsValid())
			{				
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Not a valid SimpleFreeFieldHRIR file");
				return false;
			}

			std::vector< std::size_t > dims;
			hrir.GetVariableDimensions(dims, "SourcePosition");
			if (dims.size() != 2)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA File gives invalid number of dimensions for Source Positions"); // TODO: TO_STRING(dims.size());				
				return false;
			}

			std::vector< double > pos;
			pos.resize(dims[0] * dims[1]);
			hrir.GetSourcePosition(&pos[0], dims[0], dims[1]); // dims[0] is the number of positions (187, say), dims[1] is the dimensions of the positions (3).
			const unsigned int nMeasurements = (unsigned int)hrir.GetNumMeasurements();  // The number of HRIRs (187, say)
			if (dims[0] != nMeasurements)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "SOFA gives incoherent number of source positions and measurements");				
				return false;
			}
			
			// const unsigned int nReceivers = (unsigned int) hrir.GetNumReceivers();     // Currently unused. Should be two receivers (stereo)

			std::vector< double > delays;
			hrir.GetDataDelay(delays);

			std::vector< double > data;
			hrir.GetDataIR(data);

			// Prepare HRTF
			const unsigned int nSamples = (unsigned int)hrir.GetNumDataSamples();   // For example: 512 samples.			

			if (delays.size() == data.size() / nSamples){
				specifiedDelays = true;
			}
			else {
				if (delays.size() == 2)
				{
					specifiedDelays = false;
					SET_RESULT(RESULT_WARNING, "This HRTF file does not contain individual delays for each HRIR. Therefore, some comb filter effect can be perceived due to interpolations and custom head radius should not be used");
				}
				else
				{
					SET_RESULT(RESULT_ERROR_BADSIZE, "SOFA gives incoherent number of HRIRs and delays");
					return false;
				}
			}

			double distance = pos[array2DIndex(0, 2, nMeasurements, dims[1])];		//We consider that every HRIR are meased at the same distance, so we get the firts one
			listener->GetHRTF()->BeginSetup(nSamples, distance);

			// This outtermost loop iterates over HRIRs
			for (std::size_t i = 0; i < nMeasurements; i++) // or for( std::size_t i = 0; i < dims[0]; i++ ), should be the same.
			{
				THRIRStruct hrir_value;
				hrir_value.leftHRIR.resize(nSamples);
				hrir_value.rightHRIR.resize(nSamples);
				double azimuth = pos[array2DIndex(i, 0, nMeasurements, dims[1])];
				double elevation = pos[array2DIndex(i, 1, nMeasurements, dims[1])];
				while (elevation < 0) elevation += 360; // TODO: check who should do this
														
				const int left_ear = 0;
				hrir_value.leftDelay = delays[specifiedDelays ? array2DIndex(i, left_ear, nMeasurements, 2) : 0];
				for (std::size_t k = 0; k < nSamples; k++)
				{
					const std::size_t index = array3DIndex(i, left_ear, k, nMeasurements, 2, nSamples);
					hrir_value.leftHRIR[k] = data[index];
				}				
				const int right_ear = 1;
				hrir_value.rightDelay = delays[specifiedDelays ? array2DIndex(i, right_ear, nMeasurements, 2) : 1];
				for (std::size_t k = 0; k < nSamples; k++)
				{
					const std::size_t index = array3DIndex(i, right_ear, k, nMeasurements, 2, nSamples);
					hrir_value.rightHRIR[k] = data[index];
				}								
				listener->GetHRTF()->AddHRIR(azimuth, elevation, std::move(hrir_value));
			}			
			//listener->GetHRTF()->EndSetup();
			return true;
		}
		catch (sofa::Exception &e)
		{
			// the description of the exception will be printed when raised
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Sofa exception, please consider previous messages from the sofa library");
		}
		catch (std::exception &e)
		{
			std::string s("Error when creating HRTF representation");
			s += e.what();
			SET_RESULT(RESULT_ERROR_UNKNOWN, s.c_str());
		}
		catch (...)
		{
			SET_RESULT(RESULT_ERROR_UNKNOWN, "Unknown error when creating HRTF representation");
		}		
		return false;
	}
}
