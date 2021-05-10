/**
* \class CChannel
*
* \brief Declaration of CChannel interface.
* \date	October 2020
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, D. Gonzalez-Toledo, L. Molina-Tanco ||
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

#include <Common/Waveguide.h>
#include <Common/ErrorHandler.h>

constexpr float EPSILON = 0.0001;

namespace Common {

	/// Enable propagation delay for this waveguide
	void CWaveguide::EnablePropagationDelay() { enablePropagationDelay = true; }
	
	/// Disable propagation delay for this waveguide
	void CWaveguide::DisablePropagationDelay() { 
		enablePropagationDelay = false; 
		circular_buffer.clear();		// reset the circular buffer	
	}

	/// Get the flag for propagation delay enabling for this waveguide
	bool CWaveguide::IsPropagationDelayEnabled() { return enablePropagationDelay; }

	
	/// Insert the new frame into the waveguide
	void CWaveguide::PushBack(CMonoBuffer<float> & _inputBuffer, const Common::TAudioStateStruct& _audioState, float soundSpeed, float currentDistanceToListener) {
		// Save a copy of this most recent Buffer
		mostRecentBuffer = _inputBuffer; 						
		
		//If propagation delay simulation is not enable, do nothing more
		if (!enablePropagationDelay) return;
		
		// Calculate new delay
		int newDelayInSamples = CalculateDelay(_audioState, soundSpeed, currentDistanceToListener);
		
		// Init circular buffer the first time
		if (circular_buffer.capacity() == 0) {
			// Buffer has to grow 			
			ResizeCirculaBuffer(newDelayInSamples + _audioState.bufferSize);
		}
				
		// Save data into the circular_buffer
		int currentDelayInSamples = circular_buffer.size() - _audioState.bufferSize;		//Calculate current delay in samples								
		if (newDelayInSamples == currentDelayInSamples) 
		{			
			circular_buffer.insert(circular_buffer.end(), _inputBuffer.begin(), _inputBuffer.end());

		} else {
			// Circular Buffer has to grow and Input buffer has to be expanded or
			// Circular Buffer has to shrink and Input buffer has to be compressed	

			size_t newBufferSize = newDelayInSamples - currentDelayInSamples + _audioState.bufferSize;	// Calculate the expasion/compression
 			if (newBufferSize < 0) {
				// TO DO Why would this happen?
				SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer in CWaveguide. (newBufferSize < 0)");				
				newBufferSize = 0; // TODO think if this solution is ok?
			}
			// Prepare buffer
			//CMonoBuffer<float> readyToInsertBuffer;			
			//readyToInsertBuffer.resize(newBufferSize);									
			// Expand Buffer
			//ProcessExpansionCompressionMethod(_inputBuffer, readyToInsertBuffer);
			// Change circular_buffer capacity. This is when you throw away the samples, which are already out.
			//RsetCirculaBuffer(newDelayInSamples + _audioState.bufferSize);			
			// Insert into circular buffer 		
			//circular_buffer.insert(circular_buffer.end(), readyToInsertBuffer.begin(), readyToInsertBuffer.end());	
			
			// TODO Alternative version that is more optimal, using push_back in the expansion/compression algorith to insert sample by sample directly, 
			// into the circular buffer. Saves having to create a buffer and insert it at the end. Next two line sinstead the previous four
			
			// Change circular_buffer capacity. This is when you throw away the samples, which are already out.
			RsetCirculaBuffer(newDelayInSamples + _audioState.bufferSize);
			// Expand or compress and insert into the cirular buffer
			ProcessExpansionCompressionMethod(_inputBuffer, newBufferSize);

		}				
	}
	
	/// Return next buffer frame after pass throught the waveguide
	CMonoBuffer<float> CWaveguide::PopFront(const Common::TAudioStateStruct& audioState) const
	{		
		// if the propagation delay is not activated, just return the last input buffer
		if (!enablePropagationDelay) { return mostRecentBuffer; }
		
		// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  		
		CMonoBuffer<float> returnbuffer(circular_buffer.begin(), circular_buffer.begin() + audioState.bufferSize);
		return returnbuffer;
	}

	/// Return last frame introduced into the waveguide
	CMonoBuffer<float> CWaveguide::GetMostRecentBuffer() const
	{
		return mostRecentBuffer;
	}

	/////////////////
	// PRIVATE METHODS
	/////////////////
	
	/// Resize the circular buffer
	void CWaveguide::ResizeCirculaBuffer(size_t newSize) {
		try {			
				circular_buffer.resize(newSize);			
		}
		catch (std::bad_alloc & e)
		{
			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer");
			return;
		}
	}

	/// Changes de circular buffer capacity, throwing away the oldest samples
	void CWaveguide::RsetCirculaBuffer(size_t newSize) { 
		try {						
			circular_buffer.rset_capacity(newSize);
		}
		catch (std::bad_alloc &) {
			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer (pushing back frame)");
			return;
		}	
	}

	/// Calculate the new delay in samples.
	size_t CWaveguide::CalculateDelay(Common::TAudioStateStruct audioState, float soundSpeed, float distanceToListener)
	{
		double delaySeconds = distanceToListener / soundSpeed;		
		size_t delaySamples = std::nearbyint(delaySeconds * audioState.sampleRate);				
		return delaySamples;
	}

	/// Execute a buffer expansion or compression
	void CWaveguide::ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, CMonoBuffer<float>& output)
	{
		int outputSize = output.size();
		//Calculate the compresion factor. See technical report
		float position = 0;					
		float numerator = input.size() - 1;
		float denominator = outputSize - 1;
		float compressionFactor = numerator / denominator;
		
		int j;
		float rest;
		int i;		
		//Fill the output buffer with the new values 
		for (i = 0; i< outputSize -1; i++)
		{
			j = static_cast<int>(position);			
			rest = position - j;
			
			if ((j + 1)<input.size()) {
				output[i] = input[j] * (1 - rest) + input[j + 1] * rest;

			} else {
				// TODO think why this happens. If this solution the ok?				
				output[i] = input[j] * (1 - rest);				
			}			
			position += compressionFactor;
		}							
		output[outputSize - 1] = input[input.size() - 1];		// the last sample has to be the same as the one in the input buffer.		
	}

	/// Execute a buffer expansion or compression
	void CWaveguide::ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, int outputSize)
	{
		//int outputSize = output.size();
		//Calculate the compresion factor. See technical report
		float position = 0;
		float numerator = input.size() - 1;
		float denominator = outputSize - 1;
		float compressionFactor = numerator / denominator;
		
		int j;
		float rest;
		int i;
		//Fill the output buffer with the new values 
		for (i = 0; i < outputSize - 1; i++)
		{
			j = static_cast<int>(position);
			rest = position - j;

			if ((j + 1) < input.size()) {
				//output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
				circular_buffer.push_back(input[j] * (1 - rest) + input[j + 1] * rest);

			}
			else {
				// TODO think why this happens. If this solution the ok?				
				//output[i] = input[j] * (1 - rest);
				circular_buffer.push_back(input[j] * (1 - rest));				
			}
			position += compressionFactor;
		}
		//output[outputSize - 1] = input[input.size() - 1];		// the last sample has to be the same as the one in the input buffer.		
		circular_buffer.push_back(input[input.size() - 1]);
	}

	// TO BE DELETED
	//void CWaveguide::CoutCircularBuffer()
	//{
	//	cout << "CBu:[";
	//	for (int i = 0; i < circular_buffer.size(); i++) {
	//		cout << circular_buffer[i] << ",";
	//	}
	//	cout << "]";// << endl;
	//}
	//void CWaveguide::CoutBuffer(const CMonoBuffer<float> & _buffer, string bufferName) const
	//{
	//	cout << bufferName<<":[";
	//	for (int i = 0; i < _buffer.size(); i++) {
	//		cout << _buffer[i] << ",";
	//	}
	//	cout << "]";// << endl;
	//}
}