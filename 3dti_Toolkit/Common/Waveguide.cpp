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
		Reset();
		//previousListenerPositionInitialized = false;
		//previousListenerPosition = CVector3(0, 0, 0);	// Init previous Listener position
		//circular_buffer.clear();						// reset the circular buffer	
		//sourcePositionsBuffer.clear();					// reset the source position buffer		
	}

	/// Get the flag for propagation delay enabling for this waveguide
	bool CWaveguide::IsPropagationDelayEnabled() { return enablePropagationDelay; }

	
	/// Insert the new frame into the waveguide
	void CWaveguide::PushBack(const CMonoBuffer<float> & _inputBuffer, const CVector3 & _sourcePosition, const CVector3 & _listenerPosition, const Common::TAudioStateStruct& _audioState, float _soundSpeed) {
		// Save a copy of this most recent Buffer
		mostRecentBuffer = _inputBuffer; 						
		
		//If propagation delay simulation is not enable, do nothing more
		if (!enablePropagationDelay) return;
		
		// If it is enabled we compute here the source movement
		ProcessSourceMovement(_inputBuffer, _sourcePosition, _listenerPosition, _audioState, _soundSpeed);
						
	}
	
	/// Return next buffer frame after pass throught the waveguide
	void CWaveguide::PopFront(CMonoBuffer<float> & outbuffer, const CVector3 & _listenerPosition, CVector3 & sourcePositionWhenWasEmitted, const Common::TAudioStateStruct& _audioState, float _soundSpeed)	{
		
		// if the propagation delay is not activated, just return the last input buffer
		if (!enablePropagationDelay) { 
			outbuffer = mostRecentBuffer; 
		}
		else {
			// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  		
			ProcessListenerMovement(outbuffer, _audioState, sourcePositionWhenWasEmitted, _listenerPosition, _soundSpeed);
		}				
	}

	/// Return last frame introduced into the waveguide
	CMonoBuffer<float> CWaveguide::GetMostRecentBuffer() const
	{
		return mostRecentBuffer;
	}

	/// <summary> Reset waveguide to an initial state </summary>
	void CWaveguide::Reset() {	
		bool previousPropagationDelayState = enablePropagationDelay;		
		// Initialize all
		enablePropagationDelay = false;
		previousListenerPositionInitialized = false;
		previousListenerPosition = CVector3(0, 0, 0);	// Init previous Listener position
		circular_buffer.clear();						// reset the circular buffer	
		SetCirculaBufferCapacity(0);
		sourcePositionsBuffer.clear();					// reset the source position buffer
		mostRecentBuffer.clear();
		// Go back to previous state
		enablePropagationDelay = previousPropagationDelayState;		
	}

	//////////////////////
	// PRIVATE METHODS
	///////////////////////
	
	void CWaveguide::ProcessSourceMovement(const CMonoBuffer<float> & _inputBuffer, const CVector3 & _sourcePosition, const CVector3 & _listenerPosition, const Common::TAudioStateStruct& _audioState, float _soundSpeed) {		
		// First time we initialized the listener position							
		if (!previousListenerPositionInitialized) { previousListenerPosition = _listenerPosition; previousListenerPositionInitialized = true; }
		
		// Calculate the new delay taking into account the source movement with respect to the listener previous position
		float currentDistanceToListener = CalculateDistance(_sourcePosition, previousListenerPosition);			
		float oldDistanceToListener = CalculateDistance(GetLastSourcePosition(), previousListenerPosition);		
		float distanceDiferenteToListener = currentDistanceToListener - oldDistanceToListener;		
		int changeInDelayInSamples = CalculateDistanceInSamples(_audioState, _soundSpeed, distanceDiferenteToListener);				

		if (circular_buffer.capacity() == 0) {				
			// This is the first Time
			// We initialize the buffers the first time, this is when its capacity is zero
			int newDelayInSamples = CalculateDistanceInSamples(_audioState, _soundSpeed, currentDistanceToListener);
			ResizeCirculaBuffer(newDelayInSamples + _audioState.bufferSize);			// Buffer has to grow, full of Zeros			
			InitSourcePositionBuffer(changeInDelayInSamples, _sourcePosition);			// Introduce first data into the sourcePositionBuffer.
			// Save data into the circular_buffer	
			circular_buffer.insert(circular_buffer.end(), _inputBuffer.begin(), _inputBuffer.end());	// Introduce the input buffer into the circular buffer			
			InsertBackSourcePositionBuffer(_inputBuffer.size(), _sourcePosition);							// Introduce the source positions into its buffer
		}											
		else if (changeInDelayInSamples == 0)
		{
			//No movement
			circular_buffer.insert(circular_buffer.end(), _inputBuffer.begin(), _inputBuffer.end());	//introduce the input buffer into the circular buffer			
			InsertBackSourcePositionBuffer(_inputBuffer.size(), _sourcePosition);	// introduce the source positions into its buffer						
		}
		else {
			// There a Source Movement			
			// If source moves towards the listener --> Distance decreases --> Time compression --> insertBufferSize < bufferSize
			// If source moves away from listener   --> Distance increases --> Time expansion
			
			int currentDelayInSamples	= circular_buffer.size() - _audioState.bufferSize;		// Calculate current delay in samples		
			int newDelayInSamples		= changeInDelayInSamples + currentDelayInSamples;		// Calculate the new delay in samples
			int insertBufferSize		= changeInDelayInSamples + _audioState.bufferSize;		// Calculate the expasion/compression
			
			if (insertBufferSize <= 0) {								
				// When soundsource approaches to the lister faster than the sound velocity. Insert nothing to the circular buffer
				SetCirculaBufferCapacity(newDelayInSamples + _audioState.bufferSize);		// Remove samples from circular buffer															
				ResizeSourcePositionsBuffer(circular_buffer.size());						// Remove samples for source positions buffer				
				InsertBackSourcePositionBuffer(1, _sourcePosition);							// Insert the last position with zero samples into the source position buffer				
			}
			else {				
				// Change circular_buffer capacity. This is when you throw away the samples, which are already out.				
				RsetCirculaBuffer(newDelayInSamples + _audioState.bufferSize);				
				// Expand or compress and insert into the cirular buffer			
				ProcessExpansionCompressionMethod(_inputBuffer, insertBufferSize);
				// Introduce the source positions into its buffer	
				InsertBackSourcePositionBuffer(insertBufferSize, _sourcePosition);	
			}								
		}
		CheckIntegritySourcePositionsBuffer();	//To be deleted
	}


	void CWaveguide::ProcessListenerMovement(CMonoBuffer<float> & outbuffer, const Common::TAudioStateStruct& _audioState, CVector3 & _sourcePositionWhenWasEmitted, const CVector3 & _listenerPosition, float soundSpeed) {
		
		// Get the source position when the nexts samples were emited
		_sourcePositionWhenWasEmitted = GetNextSourcePosition(_audioState.bufferSize);			
		// Calculate the new delay taking into account the current listener position respect to the source position when the samples were emmited
		float currentDistanceToEmitedSource = CalculateDistance(_listenerPosition, _sourcePositionWhenWasEmitted);
		float oldDistanceToEmitedSource = CalculateDistance(previousListenerPosition, _sourcePositionWhenWasEmitted);
		float distanceDiferenceToEmmitedSource = currentDistanceToEmitedSource - oldDistanceToEmitedSource;
		// Update Listener position		
		previousListenerPosition = _listenerPosition;

		// Calculate delay in samples. 
		// if it's < 0 --> moving towards the source. 	// if it's > 0 --> moving away from the source
		int changeInDelayInSamples = CalculateDistanceInSamples(_audioState, soundSpeed, distanceDiferenceToEmmitedSource);
		
		// Calculate samples to be extracted from the ciruclar buffer		
		// An observer moving towards the source measures a higher frequency  --> More than 512 --> Time compression 
		// An observer moving away from the source measures a lower frequency --> Less than 512 --> Time expansion		
		int samplesToBeExtracted = _audioState.bufferSize - changeInDelayInSamples;			

		// In case the listener is moving away from the source faster than the sound speed
		if (samplesToBeExtracted <= 0) {	
			samplesToBeExtracted *= -1;		// To operate with a positive number
			// Increase the buffer capacity so that no samples are lost.
			RsetCirculaBuffer(circular_buffer.capacity() + _audioState.bufferSize + samplesToBeExtracted);
			// Introduce zeros at the begging of the buffer
			CMonoBuffer<float> insertBuffer(samplesToBeExtracted);
			circular_buffer.insert(circular_buffer.begin(), insertBuffer.begin(), insertBuffer.end());
			//Introduce the new sample into the buffer of source positions
			ShiftRightSourcePositionsBuffer(samplesToBeExtracted);
			InsertFrontSourcePositionBuffer(samplesToBeExtracted, CVector3(0,0,0));
			// Output a frame of zeros			
			outbuffer.resize(_audioState.bufferSize);			
		}

		// In other case Get samples from buffer		
		if (samplesToBeExtracted == _audioState.bufferSize) { 
			// If it doesn't needed to do and expasion or compression
			//outbuffer.clear();			
			outbuffer.insert(outbuffer.begin(), circular_buffer.begin(), circular_buffer.begin() + samplesToBeExtracted);
			ShiftLeftSourcePositionsBuffer(samplesToBeExtracted);		// Delete samples that have left the buffer storing the source positions.			
		}
		else {
			//In case we need to expand or compress
			CMonoBuffer<float> extractingBuffer(circular_buffer.begin(), circular_buffer.begin() + samplesToBeExtracted /*_audioState.bufferSize*/);
			ShiftLeftSourcePositionsBuffer(samplesToBeExtracted);		// Delete samples that have left the buffer storing the source positions.					
			// the capacity of the circular buffer must be increased with the samples that have not been removed		
			RsetCirculaBuffer(circular_buffer.capacity() + _audioState.bufferSize - samplesToBeExtracted);
			// Expand or compress the buffer and return it			
			outbuffer.resize(_audioState.bufferSize);							// Prepare buffer			
			ProcessExpansionCompressionMethod(extractingBuffer, outbuffer);		// Expand or compress the buffer			
		}							
		// Pop really doesn't pop. The next time a buffer is pushed, it will be removed.  				
	}

	/////////////////////////////
	/// CIRCULAR BUFFER
	/////////////////////////////

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

	/// Changes de circular buffer capacity, throwing away the oldest samples.	
	void CWaveguide::SetCirculaBufferCapacity(size_t newSize) {
		try {
			/// It adds space to future samples on the back side and throws samples from the front side
			circular_buffer.set_capacity(newSize);
		}
		catch (std::bad_alloc &) {
			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer (pushing back frame)");
			return;
		}
	}


	/// Changes de circular buffer capacity, throwing away the oldest samples.	
	void CWaveguide::RsetCirculaBuffer(size_t newSize) { 
		try {						
			/// It adds space to future samples on the back side and throws samples from the front side
			circular_buffer.rset_capacity(newSize);
		}
		catch (std::bad_alloc &) {
			SET_RESULT(RESULT_ERROR_BADALLOC, "Bad alloc in delay buffer (pushing back frame)");
			return;
		}	
	}

	////////////////
	// DISTANCE
	///////////////

	/// Calculate the distance in meters between two positions
	const float CWaveguide::CalculateDistance(const CVector3 & position1, const CVector3 & position2) const
	{				
		float distance = (position1.x - position2.x) * (position1.x - position2.x) + (position1.y - position2.y) * (position1.y - position2.y) + (position1.z - position2.z) * (position1.z - position2.z);
		return std::sqrt(distance);		
	}

	/// Calculate the distance in samples
	size_t CWaveguide::CalculateDistanceInSamples(Common::TAudioStateStruct audioState, float soundSpeed, float distanceInMeters)
	{
		double delaySeconds = distanceInMeters / soundSpeed;		
		size_t delaySamples = std::nearbyint(delaySeconds * audioState.sampleRate);				
		return delaySamples;
	}

	/////////////////////////
	// Expansion-Compresion
	/////////////////////////

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

	/// Initialize the source Position Buffer at the begining. It is going to be supposed that the source was in that position since ever.
	void CWaveguide::InitSourcePositionBuffer(int _numberOFZeroSamples, const CVector3 & _sourcePosition) {		
		sourcePositionsBuffer.clear();	
		TSourcePosition temp(0, _numberOFZeroSamples - 1, _sourcePosition); 
		sourcePositionsBuffer.push_back(temp);
	}

	/// Insert at the buffer back the source position for a set of samples
	void CWaveguide::InsertBackSourcePositionBuffer(int bufferSize, const CVector3 & _sourcePosition) {
		int begin = circular_buffer.size() - bufferSize;
		int end = circular_buffer.size() - 1;
		TSourcePosition temp(begin, end, _sourcePosition);
		sourcePositionsBuffer.push_back(temp);						// introduce in to the sourcepositionsbuffer		
	}

	/// Insert at the buffer fromt the source position for a set of samples
	void CWaveguide::InsertFrontSourcePositionBuffer(int samples, const CVector3 & _sourcePosition) {				
		TSourcePosition temp(0, samples-1, _sourcePosition);
		sourcePositionsBuffer.insert(sourcePositionsBuffer.begin(), temp);
	}
	
	/// Shifts all buffer positions to the left, deleting any that become negative. 
	/// This is because it is assumed that samples will have come out of the circular buffer from the front.
	void CWaveguide::ShiftLeftSourcePositionsBuffer(int samples){

		vector<int> positionsToDelete;
		if (samples <= 0) { return; }
		
		//int positionToDelete = -1;
		int index = 0;
		for (auto &element : sourcePositionsBuffer) {
			element.beginIndex = element.beginIndex - samples;
			element.endIndex = element.endIndex - samples;
			if (element.endIndex < 0) { 
				// Delete
				//positionToDelete = index;
				positionsToDelete.push_back(index);
			}else if (element.beginIndex < 0) { 
				element.beginIndex = 0;
			}			
			index++;
		}
		for (int i = positionsToDelete.size() - 1; i > -1; i--) {			
				sourcePositionsBuffer.erase(sourcePositionsBuffer.begin() + positionsToDelete[i]);			
		}

	}

	/// Shifts all buffer positions to the right. 
	/// This is because it is assumed that samples will have been added on the front in the circular buffer.
	void CWaveguide::ShiftRightSourcePositionsBuffer(int samples) {

		if (samples <= 0) { return; }
		
		for (auto &element : sourcePositionsBuffer) {
			element.beginIndex = element.beginIndex + samples;
			element.endIndex = element.endIndex + samples;
		
		}	
	}

	/// Remove samples from the back side of the buffer in order to have the same size that the circular buffer	
	void CWaveguide::ResizeSourcePositionsBuffer(int newSize) {
		
		vector<int> positionsToDelete;
		if (newSize <= 0) { return; }
				
		for (int i = sourcePositionsBuffer.size() - 1; i > -1; i--) {
			
			if (sourcePositionsBuffer[i].beginIndex > newSize - 1) {
				positionsToDelete.push_back(i);
			}
			else if (sourcePositionsBuffer[i].endIndex > newSize - 1) {
				sourcePositionsBuffer[i].endIndex = newSize -1 ;
			}			
		}

		for (auto &element : positionsToDelete) {			
			sourcePositionsBuffer.erase(sourcePositionsBuffer.begin() + element);
		}
	}

	/// Get the last source position
	CVector3 CWaveguide::GetLastSourcePosition() {		
		if (sourcePositionsBuffer.size() == 0) {			
			CVector3 previousSourcePosition(0, 0, 0);
			return previousSourcePosition;
		}
		else {			
			//return sourcePositionsBuffer.back.GetPosition();
			return sourcePositionsBuffer[sourcePositionsBuffer.size() - 1].GetPosition();			
		}	
	}

	// Get the next buffer source position
	CVector3 CWaveguide::GetNextSourcePosition(int bufferSize) {
		/// TODO Check the buffer size to select the sourceposition, maybe the output buffer will include more than the just first position of the source position buffer

		return sourcePositionsBuffer.front().GetPosition();
	}
	
	void CWaveguide::CheckIntegritySourcePositionsBuffer() {
		if (sourcePositionsBuffer.size() == 0) { return; }
		if ((sourcePositionsBuffer[sourcePositionsBuffer.size() - 1].endIndex) != (circular_buffer.size() - 1)) {
			cout << "error";
		}
	}	
}