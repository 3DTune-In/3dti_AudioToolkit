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
		previousSourcePositionsBuffer.clear();
	}

	/// Get the flag for propagation delay enabling for this waveguide
	bool CWaveguide::IsPropagationDelayEnabled() { return enablePropagationDelay; }

	
	/// Insert the new frame into the waveguide
	void CWaveguide::PushBack(const CMonoBuffer<float> & _inputBuffer, const CVector3 & _sourcePosition, const Common::TAudioStateStruct& _audioState, float _soundSpeed/*, float currentDistanceToListener*/) {
		// Save a copy of this most recent Buffer
		mostRecentBuffer = _inputBuffer; 						
		
		//If propagation delay simulation is not enable, do nothing more
		if (!enablePropagationDelay) return;
		
		ProcessSourceMovement(_inputBuffer, _audioState, _soundSpeed, _sourcePosition);
						
	}
	
	/// Return next buffer frame after pass throught the waveguide
	CMonoBuffer<float> CWaveguide::PopFront(const CVector3 & _listenerPosition, const Common::TAudioStateStruct& _audioState, float _soundSpeed)	{
		
		// if the propagation delay is not activated, just return the last input buffer
		if (!enablePropagationDelay) { return mostRecentBuffer; }
		
		// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  		
		/*CMonoBuffer<float> returnbuffer(circular_buffer.begin(), circular_buffer.begin() + _audioState.bufferSize);
		ShiftSourcePositionsBuffer(_audioState.bufferSize);
		return returnbuffer;*/

		return ProcessListenerMovement(_audioState, _listenerPosition, _soundSpeed);
	}

	/// Return last frame introduced into the waveguide
	CMonoBuffer<float> CWaveguide::GetMostRecentBuffer() const
	{
		return mostRecentBuffer;
	}

	/////////////////
	// PRIVATE METHODS
	/////////////////
	
	void CWaveguide::ProcessSourceMovement(const CMonoBuffer<float> & _inputBuffer, const Common::TAudioStateStruct& _audioState, float soundSpeed, const CVector3 & _sourcePosition) {

		// Calculate new delay
		//int newDelayInSamples = CalculateDelay(_audioState, soundSpeed, currentDistanceToListener);
		float changeInSourceDistance = CalculateSourceDistanceChange(_sourcePosition);
		int newDelayInSamples = CalculateDelay(_audioState, soundSpeed, changeInSourceDistance);


		// Init circular buffer the first time
		if (circular_buffer.capacity() == 0) {
			ResizeCirculaBuffer(newDelayInSamples + _audioState.bufferSize);		// Buffer has to grow, full of Zeros			
			InitSourcePositionBuffer(newDelayInSamples);							// Introduce first data into the sourcePositionBuffer
		}

		// Save data into the circular_buffer
		int currentDelayInSamples = circular_buffer.size() - _audioState.bufferSize;		//Calculate current delay in samples								
		if (newDelayInSamples == currentDelayInSamples)
		{
			circular_buffer.insert(circular_buffer.end(), _inputBuffer.begin(), _inputBuffer.end());	//introduce the input buffer into the circular buffer			
			InsertSourcePositionBuffer(_inputBuffer.size(), _sourcePosition);	// introduce the source positions into its buffer			

			cout << "cf: " << circular_buffer[previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].beginIndex] << " " << circular_buffer[previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].endIndex] << endl;
			cout << "in: " << _inputBuffer[0] << _inputBuffer[_inputBuffer.size() - 1] << endl << endl;
		}
		else {
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
			int currentSamples = circular_buffer.capacity();
			RsetCirculaBuffer(newDelayInSamples + _audioState.bufferSize);
			//ShiftSourcePositionsBuffer(currentSamples- newDelayInSamples - _audioState.bufferSize);

			// Expand or compress and insert into the cirular buffer			
			ProcessExpansionCompressionMethod(_inputBuffer, newBufferSize);
			InsertSourcePositionBuffer(newBufferSize, _sourcePosition);	// introduce the source positions into its buffer			

			cout << "cf: " << circular_buffer[previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].beginIndex] << " " << circular_buffer[previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].endIndex] << endl;
			cout << "in: " << _inputBuffer[0] << _inputBuffer[_inputBuffer.size() - 1] << endl << endl;
		}

	}

	CMonoBuffer<float> CWaveguide::ProcessListenerMovement(const Common::TAudioStateStruct& _audioState, const CVector3 & _listenerPosition, float soundSpeed) {

		

		float changeInListenerDistance = CalculateListenerDistanceChange(_listenerPosition);
		previousListenerPosition = _listenerPosition;													// Update Listener position		
		int newDelayInSamples = CalculateDelay(_audioState, soundSpeed, changeInListenerDistance);

		// An observer moving towards the source measures a higher frequency  --> Time compressin
		// An observer moving away from the source measures a lower frequency --> Time expansion
		
		size_t newBufferSize = newDelayInSamples + _audioState.bufferSize;	// Calculate the expasion/compression

		cout << newBufferSize;

		// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  		
		CMonoBuffer<float> returnbuffer(circular_buffer.begin(), circular_buffer.begin() + _audioState.bufferSize);
		ShiftSourcePositionsBuffer(_audioState.bufferSize);

		return returnbuffer;
	}

	int CWaveguide::GetIndexOfCirculaBuffer(boost::circular_buffer<float>::iterator it) {
		return (it - circular_buffer.begin());
	}

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

	float CWaveguide::CalculateSourceDistanceChange(const CVector3 & newSourcePosition) {
		
		CVector3 sourceOldPosition = GetLastSourcePosition();
		float distance = CalculateDistance(newSourcePosition, previousListenerPosition) - CalculateDistance(sourceOldPosition, previousListenerPosition);	
		return distance;
	}

	const float CWaveguide::CalculateDistance(const CVector3 & position1, const CVector3 & position2) const
	{
				
		float distance = (position1.x - position2.x) * (position1.x - position2.x) + (position1.y - position2.y) * (position1.y - position2.y) + (position1.z - position2.z) * (position1.z - position2.z);
		return std::sqrt(distance);
		//return distance;
	}


	float CWaveguide::CalculateListenerDistanceChange(const CVector3 & newListenerPosition) {

		CVector3 sourcePositionWhenEmited = GetSourcePositionWhenEmmited();
		float distance = CalculateDistance(newListenerPosition, sourcePositionWhenEmited) - CalculateDistance(previousListenerPosition, sourcePositionWhenEmited);
		return distance;
	}

	//////////////////////////////////////////////


	/// Calculate the new delay in samples.
	size_t CWaveguide::CalculateDelay(Common::TAudioStateStruct audioState, float soundSpeed, float distanceInMeters)
	{
		double delaySeconds = distanceInMeters / soundSpeed;		
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

	////////////////////////////
	// Source Positions Buffer
	////////////////////////////
	void CWaveguide::InitSourcePositionBuffer(int numberOFZeroSamples) {		
		
		previousSourcePositionsBuffer.clear();
		CVector3 sourcePosition(0, 0, 0);				
		TSourcePosition temp(0, numberOFZeroSamples - 1, sourcePosition);
		previousSourcePositionsBuffer.push_back(temp);
	}

	void CWaveguide::InsertSourcePositionBuffer(int bufferSize, const CVector3 & _sourcePosition) {
		int begin = circular_buffer.size() - bufferSize;
		int end = circular_buffer.size() - 1;
		InsertSourcePositionBuffer(begin, end, _sourcePosition);			// introduce in to the sourcepositionsbuffer	
	}

	void CWaveguide::InsertSourcePositionBuffer(int begin, int end, const CVector3 & sourcePosition) {
	
		//ShiftSourcePositionsBuffer(end - begin + 1);
		
		TSourcePosition temp(begin, end, sourcePosition);
		previousSourcePositionsBuffer.push_back(temp);
	}
	void CWaveguide::ShiftSourcePositionsBuffer(int samples){
		
		if (samples <= 0) { return; }
		
		int positionToDelete = -1;
		int index = 0;
		for (auto &element : previousSourcePositionsBuffer) {
			element.beginIndex = element.beginIndex - samples;
			element.endIndex = element.endIndex - samples;
			if (element.endIndex < 0) { 
				// Delete
				positionToDelete = index;
			}else if (element.beginIndex < 0) { 
				element.beginIndex = 0;
			}			
			index++;
		}
		if (positionToDelete != -1) { 
			previousSourcePositionsBuffer.erase(previousSourcePositionsBuffer.begin() + positionToDelete);
		}

	}
	
	/// Get the last source position
	CVector3 CWaveguide::GetLastSourcePosition() {		
		if (previousSourcePositionsBuffer.size() == 0) {			
			CVector3 oldPosition(0, 0, 0);
			return oldPosition;
		}
		else {
			CVector3 oldPosition(previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].x, previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].y, previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].z);			
			return previousSourcePositionsBuffer[previousSourcePositionsBuffer.size() - 1].GetPosition();			
		}	
	}

	// Get the next buffer source position
	CVector3 CWaveguide::GetSourcePositionWhenEmmited() {		
		/// TODO Check the buffer size to select the sourceposition, maybe the output buffer will include more than the just first position of the source position buffer

		return previousSourcePositionsBuffer.front.GetPosition();
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