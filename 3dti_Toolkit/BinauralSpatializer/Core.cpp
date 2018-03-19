/**
* \class CCore
*
* \brief Definition of CCore interfaces.
* \date	July 2016
*
* \authors 3DI - DIANA Research Group(University of Malaga), in alphabetical order : M.Cuevas - Rodriguez, C.Garre, D.Gonzalez - Toledo, E.J.de la Rubia - Cuestas, L.Molina - Tanco ||
* Coordinated by, A.Reyes - Lecuona(University of Malaga) and L.Picinali(Imperial College London) ||
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

#include <BinauralSpatializer/Core.h>
#include <BinauralSpatializer/Listener.h>
#include <BinauralSpatializer/Environment.h>
#include <Common/ErrorHandler.h>
#include <string>


namespace Binaural {

	CCore::CCore(Common::TAudioStateStruct _audioState, int _HRTF_resamplingStep)
		:audioState{ _audioState }, HRTF_resamplingStep{ _HRTF_resamplingStep }{
	}


	// Default constructor calls normal one with default values
	CCore::CCore()
		:CCore{ { 44100, 512} , 5 } // AudioState with default values: 44100 Hz sampleRate  & 512 bufferSize 
	{}

	// Set global audio state parameters
	void CCore::SetAudioState(Common::TAudioStateStruct _audioState)
	{		
		//Check if the new buffer size is power of two
		bool powerOfTwo = Common::CFprocessor::CalculateIsPowerOfTwo(_audioState.bufferSize);
		ASSERT(powerOfTwo, RESULT_ERROR_BADSIZE, "Bad Buffer size, it's not power of two", "");
		if (powerOfTwo)
		{
			if (_audioState.sampleRate != audioState.sampleRate)
			{
				audioState = _audioState;	// Change the value for the new one
				RemoveAllSources();			// Remove all sources			
				ResetHRTF_BRIR_ILD();		// Reset HRTF, BRIR, ILD						
			}
			else if (_audioState.bufferSize != audioState.bufferSize)
			{
				audioState = _audioState;	//Change the value for the new one
				CalculateHRTFandBRIR();
			}
		}
    }

	// Get global audio state parameters
	Common::TAudioStateStruct CCore::GetAudioState() const
	{
		// FIXME: How does this depend on Core state?
		return audioState;
	}

	// Get physical magnitudes
	Common::CMagnitudes CCore::GetMagnitudes() const
	{
		return magnitudes;
	}

	// Get physical magnitudes
	shared_ptr<CListener> CCore::GetListener() const
	{
		if (listener!=nullptr) {
			return listener;
		}
		else {
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Does not exist listener");
			return nullptr;
		}

	}

	// Set new physical magnitudes
	void CCore::SetMagnitudes(Common::CMagnitudes _magnitudes)
	{
		magnitudes = _magnitudes;
	}
 
    // Create a new listener
    shared_ptr<CListener> CCore::CreateListener(float listenerRadius)
    {
		if (!listener) {
			// Create new source and add it to this core sources
			try
			{
				std::shared_ptr<CListener> newListener = std::make_shared<CListener>(this, listenerRadius);
				listener = newListener;

				SET_RESULT(RESULT_OK, "Listener created succesfully");
				return newListener;
			}
			catch (std::bad_alloc& ba)
			{
				ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
				return nullptr;
			}
		}
		else {
			SET_RESULT(RESULT_ERROR_NOTALLOWED, "There is already a listener, creating a new one is not allowed. Remove the existing listener first");
			return nullptr;
		}
    }

	//Remove listener Audio source for spatialization
	void CCore::RemoveListener()
	{
		listener.reset();
	}
    
    // Create a new environment for spatialization in a specific room
    shared_ptr<CEnvironment> CCore::CreateEnvironment()
    {
		// Create new environment and add it to this core environment list
		try
		{
			shared_ptr<CEnvironment> newEnvironment(new CEnvironment(this));
			environments.push_back(newEnvironment);

			SET_RESULT(RESULT_OK, "Environment in Core created succesfully");
			return newEnvironment;
		}
		catch (std::bad_alloc& ba)
		{
			//SET_RESULT(RESULT_ERROR_BADALLOC, ba.what());
			ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
			return nullptr;
		}
    }

    
    // Stateless privately called version
    void CCore::RemoveEnvironment(shared_ptr<CEnvironment> environment)
    {
        if (environment == nullptr)
        {
            SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Pointer is NULL when attempting to remove single source DSP");
            return;
        }
        
        bool found = false;
        for( auto it = environments.begin(); it != environments.end(); ++it )
        {
            if (*it == environment )
            {
                environments.erase(it);
                found = true;
                break;
            }
        }
        
        if(found)
            SET_RESULT(RESULT_OK, "Environment removed succesfully");
        else
            SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Environment was not found when attempting to remove");
    }


	// Create a new audio source for spatialization
	shared_ptr<CSingleSourceDSP> CCore::CreateSingleSourceDSP()
	{
		// Create new source and add it to this core sources
		try
		{
			shared_ptr<CSingleSourceDSP> newSource(new CSingleSourceDSP(this));
			audioSources.push_back(newSource);
			if(listener->GetHRTF()->IsHRTFLoaded()){ newSource->ResetSourceConvolutionBuffers(listener); }	//If the HRTF has been already loaded, the convolution buffers have to be set
			
			SET_RESULT(RESULT_OK, "Single source DSP created succesfully");
			return newSource;
		}
		catch (std::bad_alloc& ba)
		{
			//SET_RESULT(RESULT_ERROR_BADALLOC, ba.what());
			ASSERT(false, RESULT_ERROR_BADALLOC, ba.what(), "");
			return nullptr;
		}
	}

	// Stateless privately called version
	void CCore::RemoveSingleSourceDSP( shared_ptr<CSingleSourceDSP> source)
	{
		if (source == nullptr)
		{
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Pointer is NULL when attempting to remove single source DSP");
			return;
		}

		bool found = false;
		for( auto it = audioSources.begin(); it != audioSources.end(); ++it )
		{
			if (*it == source )
			{
				audioSources.erase(it);
				found = true;
				break;
			}
		}

		if(found)
			SET_RESULT(RESULT_OK, "Single source DSP removed succesfully");
		else
			SET_RESULT(RESULT_ERROR_INVALID_PARAM, "Single Source DSP was not found when attempting to remove");
	}
	
	//Reset the convolution buffers of each source
	void CCore::ResetConvolutionBuffers() {
		for (auto eachSource : audioSources)
		{
			eachSource->ResetSourceConvolutionBuffers(listener);
		}
	}

	// Calculate the new coordinates from the source to the listener 
	void CCore::CalculateSourceCoordinates() {
		for (auto eachSource : audioSources)
		{
			eachSource->CalculateSourceCoordinates();
		}
	}

	// Set HRTF resampling step
	void CCore::SetHRTFResamplingStep(int _HRTF_resamplingStep)
	{	
		if ((_HRTF_resamplingStep > 0) && (_HRTF_resamplingStep < 90))
		{			
			if (_HRTF_resamplingStep != HRTF_resamplingStep)			
			{
				HRTF_resamplingStep = _HRTF_resamplingStep;	//Change the value for the new one
				CalculateHRTFandBRIR();
			}
		}
		else
			SET_RESULT(RESULT_ERROR_OUTOFRANGE, "Wrong value for HRTF resampling step; needs to be >0 deg and <90 deg");
	}

	// Get HRTF resampling step
	int CCore::GetHRTFResamplingStep()
	{
		return HRTF_resamplingStep;
	}

	// Reset HRTF and BRIR when buffer size or HRTF resampling step changes	
	void CCore::CalculateHRTFandBRIR()
	{
		//Clean the HRTF and BRIR
		if (listener != nullptr) { listener->CalculateHRTF(); }				 			
		if (environments.size() == 1) {
			if (environments[0] != nullptr) { environments[0]->CalculateBRIR(); }//FIXME Check which environment is currently been used						
		}
		else if (environments.size() > 1)
		{
			//FIXME what happen with more than one environment???	
			SET_RESULT(RESULT_ERROR_BADSIZE, "The are more than one enviroment");
		}
	}
	
	//Reset HRTF, BRIR and ILD when samplerate changes
	void CCore::ResetHRTF_BRIR_ILD() 
	{
		// Clean the HRTF and ILD
		if (listener != nullptr) { 
			listener->ResetHRTF(); 
			listener->ResetILD();			
		}				 			
		if (environments.size() == 1) {
			if (environments[0] != nullptr) { environments[0]->ResetBRIR_ABIR();  }
		}
		else if (environments.size() > 1)
		{
			//FIXME what happen with more than one environment???	
			SET_RESULT(RESULT_ERROR_BADSIZE, "The are more than one enviroment");
		}
	}

	//Remove all sources when samplerate changes
	void CCore::RemoveAllSources()
	{
		audioSources.clear();				//Clear all the sources				
	}
}
