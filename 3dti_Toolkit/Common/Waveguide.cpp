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

#include <iostream> //TODO DELETE
#include <Common/Waveguide.h>
#include <Common/ErrorHandler.h>
//#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>

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

	//void CChannel::PushBack(CMonoBuffer<float> & _buffer, float currentDistance) {
	//	float changeInDistance = currentDistance - lastDistance;
	//	lastDistance = currentDistance;
	//	float step = 1.0f / 44100; // FIXME Get this from audio framework
	//	// Distance is shrinking or augmenting 
	//	if (std::fabs(changeInDistance) > EPSILON) {
	//		////boost::math::interpolators::cardinal_cubic_b_spline<float> spline(_buffer.begin(), _buffer.end(), 0, step);
	//		//float changeInMilliseconds = changeInDistance / 343.0f; // FIXME Get this from audio framework. 
	//		//float changeInNSamples = changeInMilliseconds * 44.1; // FIXME Get this from audio framework. 
	//		//size_t sizeOfInterpolatedBuffer = std::nearbyint(_buffer.size() + changeInNSamples);
	//		//CMonoBuffer<float> interpolatedBuffer(sizeOfInterpolatedBuffer);
	//		//float factor = _buffer.size() / (_buffer.size() + changeInNSamples);
	//		//for (size_t i = 0; i < sizeOfInterpolatedBuffer; i++) {
	//		//	float j = i * factor;
	//		//	//interpolatedBuffer[i] = spline(j);
	//		//}
	//		try {
	//			// Now change the capacity to acommodate the new delay. 
	//			//circular_buffer.rset_capacity(circular_buffer.capacity() - 512 + sizeOfInterpolatedBuffer); // FIXME hardcoded frame size to 512
	//		}
	//		catch (std::bad_alloc &) {
	//			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer (pushing back frame)");
	//			return;
	//		}
	//		// and add the new frame
	//		//circular_buffer.insert(circular_buffer.end(), interpolatedBuffer.begin(), interpolatedBuffer.end());
	//	}
	//	// No change
	//	else  {
	//		// Just add frame at the end of the buffer.Circular buffer is fixed size
	//		// so the latest frame (at the front) will be deleted. 
	//		circular_buffer.insert(circular_buffer.end(), _buffer.begin(), _buffer.end());
	//	}
	//	// Save copy of this most recent Buffer
	//	mostRecentBuffer = _buffer; 
	//}

	/// Insert the new frame into the channel
	// TODO ¿new name?
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
			//circular_buffer.resize(newDelayInSamples + _audioState.bufferSize);
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
				cout << "ERROR";
				newBufferSize = 0; // TODO think if this solution is ok?
			}
			// Prepare buffer
			CMonoBuffer<float> readyToInsertBuffer;			
			readyToInsertBuffer.resize(newBufferSize);
			// Expand Buffer
			ProcessExpansionCompressionMethod(_inputBuffer, readyToInsertBuffer);
			// Change circular_buffer capacity. This is when you throw away the samples, which are already out.
			RsetCirculaBuffer(newDelayInSamples + _audioState.bufferSize);
			
			// Insert into circular buffer 		
			circular_buffer.insert(circular_buffer.end(), readyToInsertBuffer.begin(), readyToInsertBuffer.end());	

			// TODO Alternative version that is more optimal, using push_back to insert sample by sample directly, 
			// the expasion/compression, into the circular buffer. Saves having to create a buffer and insert it at the end.
			// Change circular_buffer capacity. This is when you throw away the samples, which are already out.
			//RsetCirculaBuffer(newDelayInSamples + _audioState.bufferSize);
			// Expand or compress and insert into the cirular buffer
			//ProcessExpansionCompressionMethod(_inpuBuffer, newBufferSize);			

		}				
	}
	
	CMonoBuffer<float> CWaveguide::PopFront(const Common::TAudioStateStruct& audioState) const
	{		
		// if the propagation delay is not activated, just return the last input buffer
		if (!enablePropagationDelay) { return mostRecentBuffer; }
		
		// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  		
		CMonoBuffer<float> returnbuffer(circular_buffer.begin(), circular_buffer.begin() + audioState.bufferSize);
		return returnbuffer;
	}

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

	void CWaveguide::RsetCirculaBuffer(size_t newSize) { 
		try {						
			circular_buffer.rset_capacity(newSize);
		}
		catch (std::bad_alloc &) {
			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer (pushing back frame)");
			return;
		}	
	}


	//void CChannel::SetDelayInSamples(int samples)
	//{
	//	try {
	//		if (circular_buffer.capacity() < samples) { // Buffer has to grow

	//				// FIXME: For the moment this is just a fixed delay, 
	//				// FIXME: hardcoded frameSize as 512. 
	//			circular_buffer.resize(samples + 512); // Is it samples or samples + frame size??

	//		}
	//		else { // Buffer has to shrink
	//			// FIXME: hardcoded frameSize as 512. 
	//			circular_buffer.rset_capacity(samples + 512); // FIXME: should be rset_capacity so that we get rid of the oldest samples

	//		}
	//	}
	//	catch (std::bad_alloc & e)
	//	{
	//		SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer");
	//		return;
	//	}
	//}
	CMonoBuffer<float> CWaveguide::GetMostRecentBuffer() const
	{
		return mostRecentBuffer;
	}

	/// Calculate the new delay in samples.
	size_t CWaveguide::CalculateDelay(Common::TAudioStateStruct audioState, float soundSpeed, float distanceToListener)
	{
		double delaySeconds = distanceToListener / soundSpeed;		
		size_t delaySamples = std::nearbyint(delaySeconds * audioState.sampleRate);				
		return delaySamples;
	}



	/// ExpanderCompressor.Apply doppler effect simulation
	//void CWaveguide::ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, size_t resultSize)
	//{
	//	//Prepare the outbuffer		
	//	//if (output.size() != newDelay) { output.resize(newDelay); }

	//	//Prepare algorithm variables
	//	float position = 0;
	//	float numerator = resultSize - 1;
	//	float denominator = resultSize - 1 + resultSize - input.size();
	//	float compressionFactor = numerator / denominator;
	//	
	//	int j;
	//	float rest;
	//	//int forLoop_end;
	//	////The last loop iteration must be addressed in a special way if newDelay = 0 (part 1)
	//	//if (newDelay == 0) { forLoop_end = input.size() - 1; }
	//	//else { forLoop_end = input.size(); }

	//	//Fill the output buffer with the new values 
	//	for (int i = 0; i < resultSize - 1; i++)
	//	{
	//		j = static_cast<int>(position);
 //  			rest = position - j;
	//		//output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
	//		circular_buffer.push_back(input[j] * (1 - rest) + input[j + 1] * rest);
	//		position += compressionFactor;
	//	}
	//	output[resultSize - 1] = input[input.size() - 1];			
	//	//circular_buffer.push_back(input[input.size() - 1]);		
	//}


	void CWaveguide::ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, CMonoBuffer<float>& output)
	{
		//Calculate the compresion factor. See technical report
		float position = 0;
		float numerator = output.size() - 1;
		float denominator = output.size() - 1 + output.size() - input.size();
		float compressionFactor = numerator / denominator;
				
		int j;
		float rest;
		int i;
		
		//Fill the output buffer with the new values 
		for (i = 0; i< output.size() -1; i++)
		{
			j = static_cast<int>(position);
			rest = position - j;
			
			if ((j + 1)<input.size()) {
				output[i] = input[j] * (1 - rest) + input[j + 1] * rest;

			} else {
				// TODO think why this happens. If this solution the ok?				
				output[i] = input[j] * rest;
			}			
			position += compressionFactor;
		}							
		output[output.size() - 1] = input[input.size() - 1];		// the last sample has to be the same as the one in the input buffer.		
	}

//void CChannel::ProcessExpansionCompressionMethod(const CMonoBuffer<float>& input, CMonoBuffer<float>& output)
	//{
	//	//Prepare the outbuffer		
	//	//if (output.size() != newDelay) { output.resize(newDelay); }

	//	//Prepare algorithm variables
	//	float position = 0;
	//	float numerator = output.size() - 1;
	//	float denominator = output.size() - 1 + output.size() - input.size();
	//	float compressionFactor = numerator / denominator;

	//	//Add samples to the output from buffer
	//	/*for (int i = 0; i < delayBuffer.size(); i++)
	//	{
	//		output[i] = delayBuffer[i];
	//	}*/

	//	//Fill the others buffers
	//	//if the delay is the same one as the previous frame use a simplification of the algorithm
	//	//if (newDelay == delayBuffer.size())
	//	//{
	//	//	//Copy input to output
	//	//	int j = 0;
	//	//	for (int i = delayBuffer.size(); i < input.size(); i++)
	//	//	{
	//	//		output[i] = input[j++];
	//	//	}
	//	//	//Fill delay buffer
	//	//	for (int i = 0; i < newDelay; i++)
	//	//	{
	//	//		delayBuffer[i] = input[j++];
	//	//	}
	//	//}
	//	//else, apply the expansion/compression algorihtm
	//	//else
	//	{
	//		int j;
	//		float rest;
	//		//int forLoop_end;
	//		////The last loop iteration must be addressed in a special way if newDelay = 0 (part 1)
	//		//if (newDelay == 0) { forLoop_end = input.size() - 1; }
	//		//else { forLoop_end = input.size(); }

	//		//Fill the output buffer with the new values 
	//		for (int i = 0; i< output.size()-1; i++)
	//		{
	//			j = static_cast<int>(position);
	//			rest = position - j;
	//			output[i] = input[j] * (1 - rest) + input[j + 1] * rest;
	//			position += compressionFactor;
	//		}
	//		output[output.size() - 1] = input[input.size() - 1];

	//		////The last loop iteration must be addressed in a special way if newDelay = 0 (part 2)
	//		//if (newDelay == 0)
	//		//{
	//		//	output[input.size() - 1] = input[input.size() - 1];
	//		//	delayBuffer.clear();
	//		//}
	//		////if newDelay!=0 fill out the delay buffer
	//		//else
	//		//{
	//		//	//Fill delay buffer 			
	//		//	CMonoBuffer<float> temp;
	//		//	temp.reserve(newDelay);
	//		//	for (int i = 0; i < newDelay - 1; i++)
	//		//	{
	//		//		int j = int(position);
	//		//		float rest = position - j;
	//		//		temp.push_back(input[j] * (1 - rest) + input[j + 1] * rest);
	//		//		position += compressionFactor;
	//		//	}
	//		//	//Last element of the delay buffer that must be addressed in a special way
	//		//	temp.push_back(input[input.size() - 1]);
	//		//	//delayBuffer.swap(temp);				//To use in C++03
	//		//	delayBuffer = std::move(temp);			//To use in C++11
	//		//}
	//	}
	//}//End 

	// TO BE DELETED
	void CWaveguide::CoutCircularBuffer()
	{
		cout << "CBu:[";
		for (int i = 0; i < circular_buffer.size(); i++) {
			cout << circular_buffer[i] << ",";
		}
		cout << "]";// << endl;
	}
	void CWaveguide::CoutBuffer(const CMonoBuffer<float> & _buffer, string bufferName) const
	{
		cout << bufferName<<":[";
		for (int i = 0; i < _buffer.size(); i++) {
			cout << _buffer[i] << ",";
		}
		cout << "]";// << endl;
	}
}