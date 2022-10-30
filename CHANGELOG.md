# Change Log
All notable changes to the 3DTuneIn Toolkit will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/).

## [M20221028] Audio Toolkit v2.0 M20221028

### Binaural
`Added` 
 - CSingleSourceDSP: The new CWaveguide class has been incorporated, adding a new simulator for: propagation delay between source and listener and doppler effect in case of source or listener movement.
 - New public methods in CSingleSourceDSP:
	 * void EnablePropagationDelay();
	 * void DisablePropagationDelay();
     * bool IsPropagationDelayEnabled();
	 * float GetCurrentEarAzimuth(Common::T_ear ear) const;
	 * float GetEffectiveEarAzimuth(Common::T_ear ear) const;
	 * float GetCurrentEarElevation(Common::T_ear ear) const;
	 * float GetEffectiveEarElevation(Common::T_ear ear) const;
     * const Common::CTransform & GetCurrentSourceTransform() const;
	 * const Common::CTransform & GetEffectiveSourceTransform() const;
- feat: Makes a waveguide reset when a SingleSourceDSP buffers are reseted.
	 * void CWaveguide::Reset()
`Changed`
 -  Convolutional reverb can now skip a number of initial frames for future hybrid reverb (ISM+Convolution)
	 * old: void CEnvironment::ProcessVirtualAmbisonicReverbAdimensional(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight);
	 *      void CEnvironment::ProcessVirtualAmbisonicReverbBidimensional(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight);     
	 *      void CEnvironment::ProcessVirtualAmbisonicReverbThreedimensional(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight);	 
	 *      void CEnvironment::ProcessVirtualAmbisonicReverb(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight);    
	 *      void CEnvironment::ProcessVirtualAmbisonicReverb(CStereoBuffer<float> & outBuffer)      
	 * new: void CEnvironment::ProcessVirtualAmbisonicReverbAdimensional(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames)     
	 *      void CEnvironment::ProcessVirtualAmbisonicReverbBidimensional(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames)
	 *      void CEnvironment::ProcessVirtualAmbisonicReverbThreedimensional(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames)
	 *      void CEnvironment::ProcessVirtualAmbisonicReverb(CMonoBuffer<float> & outBufferLeft, CMonoBuffer<float> & outBufferRight, int numberOfSilencedFrames)
	 *      void CEnvironment::ProcessVirtualAmbisonicReverb(CStereoBuffer<float> & outBuffer, int numberOfSilencedFrames)
- Now smoothing in attenuation by distance can be disabled or enabled (enabled by default) 
`Removed`
 - Public methods removed from CSingleSourceDSP:
     * void ProcessAnechoic(const CMonoBuffer<float> & inBuffer, CMonoBuffer<float> &outLeftBuffer, CMonoBuffer<float> &outRightBuffer);
	 * void ProcessAnechoic(const CMonoBuffer<float> & inBuffer, CStereoBuffer<float> & outBuffer);
	 * float GetEarAzimuth( Common::T_ear ear ) const;
	 * const Common::CTransform & GetSourceTransform() const;
	 
### ISM
`Added`
 - New classes added to simulate Image Source Method.
	 * class ISM //interface to access all the features of the ISM simulator
	     * CISM(Binaural::CCore* _ownerCore);
	     * void SetupShoeBoxRoom(float length, float width, float height);
	     * void setupArbitraryRoom(RoomGeometry roomGeometry);
	     * void setAbsortion(std::vector<float> _absortionPerWall);
	     * void setAbsortion(std::vector<std::vector<float>> _absortionPerBandPerWall);
	     * Room getRoom();
	     * void enableWall(int wallIndex);
	     * void disableWall(int wallIndex);
	     * void setReflectionOrder(int reflectionOrder);
	     * int getReflectionOrder();
		 * void setMaxDistanceImageSources(float maxDistanceSourcesToListener);
	     * float getMaxDistanceImageSources();
	     * int calculateNumOfSilencedFrames (float maxDistanceSourcesToListener);
	     * void setSourceLocation(Common::CVector3 location);
	     * Common::CVector3 getSourceLocation();
		 * std::vector<Common::CVector3> getImageSourceLocations();
		 * std::vector<ISM::ImageSourceData> getImageSourceData();
		 * void proccess(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation);
	 * class Room
		 * void setupShoeBox(float length, float width, float height);
		 * void setupRoomGeometry(RoomGeometry roomGeometry);
		 * void insertWall(Wall newWall);
		 * void enableWall(int wallIndex);
		 * void disableWall(int wallIndex);
		 * void setWallAbsortion(int wallIndex, float absortion);
         * void setWallAbsortion(int wallIndex, std::vector<float> absortionPerBand);
         * std::vector<Wall> getWalls();
		 * std::vector<Room> getImageRooms();
		 * bool checkPointInsideRoom(Common::CVector3 point, float &distanceNearestWall);
		 * Common::CVector3 getCenter();
	 * class SourceImages
		 * SourceImages(ISM::CISM* _ownerISM);
		 * void setLocation(Common::CVector3 _location);
		 * Common::CVector3 getLocation();
		 * std::vector<weak_ptr <SourceImages>> getImages();
		 * void getImageLocations(std::vector<Common::CVector3> &imageSourceList);
		 * void getImageData(std::vector<ImageSourceData> &imageSourceDataList);
		 * Wall getReflectionWall();
		 * void createImages(Room _room, int reflectionOrder);
		 * void updateImages ();
		 * void processAbsortion(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation);
	 * class Wall
	     * Wall();
		 * int insertCorner(float x, float y, float z);
		 * int insertCorner(Common::CVector3 _corner);
		 * std::vector<Common::CVector3> getCorners();
		 * void setAbsortion(float _absortion);
		 * void setAbsortion(std::vector<float> _absortionPerBand);
		 * std::vector<float> getAbsortionB();
		 * Common::CVector3 getNormal();
		 * Common::CVector3 getCenter();
		 * float getDistanceFromPoint(Common::CVector3 point);
		 * float getMinimumDistanceFromWall(ISM::Wall wall);
		 * Common::CVector3 getImagePoint(Common::CVector3 point);
		 * Wall getImageWall(Wall _wall);
		 * Common::CVector3 getPointProjection(float x, float y, float z);
		 * Common::CVector3 getPointProjection(Common::CVector3 point);
		 * Common::CVector3 getIntersectionPointWithLine(Common::CVector3 point1, Common::CVector3 point2);
		 * int checkPointInsideWall(Common::CVector3 point, float &distanceNearestEdge, float &sharpness);
		 * float calculateDistanceNearestEdge(Common::CVector3 point);
		 * float distancePointToLine(Common::CVector3 point, Common::CVector3 pointLine1, Common::CVector3 pointLine2);
		 * void enable() { active = true; }
		 * void disable() { active = false; }
		 * bool isActive() { return active; }


### Common
`Fixed`
 - Bug fixed in the computation of cross product in CVector3 CVector3::CrossProduct(CVector3 _rightHand).
`Added`
 - New class in CWaveguide. It provides a waveguide simulation, to simulate the distance and doppler effect betwenn a source and the listener
`Changed`
  -  Convolution can now skip a number of initial frames for future hybrid reverb (ISM+Convolution)
	 * old: void CUPCEnvironment::ProcessUPConvolution_withoutIFFT(const CMonoBuffer<float>& inBuffer_Time, const TImpulseResponse_Partitioned & IR, CMonoBuffer<float>& outBuffer)
	 * new: void CUPCEnvironment::ProcessUPConvolution_withoutIFFT(const CMonoBuffer<float>& inBuffer_Time, const TImpulseResponse_Partitioned & IR, CMonoBuffer<float>& outBuffer, int numberOfSilencedFrames)
	 
 
## [M20190724] - AudioToolkit_v1.4 M20190724

### HAHLSimulation
A new frequency smearing algorithm is implemented, based on Moore's algorithm. The developer can choose between this new algorithm or the old one and configure them independently for each ear.

`Added`
 - New enum type SmearingAlgorithm. Contains: 
     * CLASSIC, that represents the old algorithm
     * SUBFRAME, that represents the new algorithm
 - New function GetSmearingAlgorithm, returns the smearing algorithm configured
     * SmearingAlgorithm CFrequencySmearing::GetSmearingAlgorithm();
 - New method SetSmearingAlgorithm lets the developer toggle between frequency smearing algorithms
     * void CFrequencySmearing::SetSmearingAlgorithm(SmearingAlgorithm _smearingAlgorithm);
 - New methods SetDownwardBroadeningFactor and SetUpwardBroadeningFactor let the developer configure Moore's frequency smearing algorithm
     * void CFrequencySmearing::SetDownwardBroadeningFactor(float _downwardBroadeningFactor);
     * void CFrequencySmearing::SetUpwardBroadeningFactor(float _upwardBroadeningFactor);

`Modified`
 - Hearing Loss Sim Setup function now has smearing algorithm enum as an input
     * old: void CHearingLossSim::Setup(int samplingRate, float Calibration_dBs_SPL_for_0_dBs_fs, float iniFreq_Hz, int bandsNumber, int filtersPerBand, int bufferSize);
     * new: void CHearingLossSim::Setup(int samplingRate, float Calibration_dBs_SPL_for_0_dBs_fs, float iniFreq_Hz, int bandsNumber, int filtersPerBand, int bufferSize, **CFrequencySmearing::SmearingAlgorithm _smearingAlgorithm**);

 - Frequency Smearing Setup function now has smearing algorithm enum as an input
     * old:  void CFrequencySmearing::Setup(int _bufferSize, float _samplingRate);
     * new:  void CFrequencySmearing::Setup(int _bufferSize, float _samplingRate**, SmearingAlgorithm _smearingAlgorithm**);

### Common
`Added`
 - New method in CFProcessor, ProcessToPowerPhase. It provides power and phase from a FFT:
    * void CFProcessor::ProcessToPowerPhase(const std::vector<float>& inputBuffer, std::vector<float>& powerBuffer, std::vector<float>& phaseBuffer)
    

### Resource Manager
`Added`
 - Eigen added to third party libraries. It is used in Moore's frequency smearing algorithm.


## [M20181003] - AudioToolkit_v1.3_20181003

### Binaural

`Fixed`
 -  Solved a bug with computation of azimuth and elevation when axis conventions were different from that of the Binaural Test_App.
 -  Solved a crash with reverb when elevation is +90deg or -90deg.
 -  Now it is checked if the source is inside the head for adimensional and three-dimensional reverb. (It was only in the bi-dimensional mode).
 -  HRTF files in 3dti-hrtf format were updated (they were outdated and incompatible with the current version of the toolkit)
 -  Bug fixed in IIR filters for the high performance mode.

`Changed`
 -  BRIR files in the resource folder are updated to adjust levels with provided HRTF. 
 
 
## [M20180726] - AudioToolkit_v1.2_20180726 

### Binaural

`Changed`
 - Reverberation order's default value is now BIDIMENSIONAL

## [M20180706] - AudioToolkit_v1.2_20180706 

### Binaural

`Changed`
 - Modified CalculateDirectionalityAttenuation function name:
     * old: float CalculateDirectionalityAttenuation(float directionalityExtend, float angleToForwardAxis_rad);
     * new: float CalculateDirectionalityAttenuation**_dB**(float directionalityExtend, float angleToForwardAxis_rad);

 - Modified reverb attenuation calculation, now the integral over the directionality pattern is performed on the squared linear directionality, as it represents the addition of numerous incoherent power contributions.

 - All Get/Set directionality public functions now use decibels and the Toolkit internally works in linear units to avoid conversion from decibels to linear units every audio frame.

## [M20180705] - AudioToolkit_v1.1_20180705

### Binaural

`Removed`
 - Removed function CListener::GetMinimumDistanceToSource
   * old: float CListener::GetMinimumDistanceToSource()
   
`Changed`
 - Modified CBRIR::AddBRIR to be a method that returns a boolean value indicating if the BRIR has been added correctly
   * old: ~~void~~ CBRIR::AddBRIR(VirtualSpeakerPosition vsPosition, Common::T_ear vsChannel, TImpulseResponse && newBRIR)
   * new: **bool** CBRIR::AddBRIR(VirtualSpeakerPosition vsPosition, Common::T_ear vsChannel, TImpulseResponse && newBRIR)
 - Modified CBRIR::EndSetup to be a method that returns a boolean value indicating if setup has been successful
   * old: ~~void~~ CBRIR::AddBRIR()
   * new: **bool** CBRIR::AddBRIR()
 - Modified CEnvironment::CalculateABIRPartitioned to be a method that returns a boolean value indicating if ABIR has been calculated correctly
   * old: ~~void~~ CEnvironment::CalculateABIRPartitioned()
   * new: **bool** CEnvironment::CalculateABIRPartitioned()
 - Modified far distance effect, now uses a different cutoff frequency calculation and low-pass filtering
 - Modified directionality attenuation calculation

`Added` 
 - New enumeration type TReverberationOrder in CEnvironment
   * enumerators: ADIMENSIONAL (to only process W channel), BIDIMENSIONAL (to only process X, Y and W channels), TRIDIMENSIONAL (to process X, Y, Z and W channels)
 - New method to set the reverberation order in CEnvironment
   * new: CEnvironment::SetReverberationOrder(TReverberationOrder order)
 - New boolean function to know if a partitioned impulse response is empty
   * new: bool CBRIR::IsIREmpty(const TImpulseResponse_Partitioned& in)
 - New function to get the reverberation order in CEnvironment
   * new: TReverberationOrder CEnvironment::GetReverberationOrder()
 - New static boolean function CMagnitudes::AreSame to know if two float values (a and b) have same value within a margin specified by epsilon
   * new: static bool CMagnitudes::AreSame(float a, float b, float epsilon)

### HAHLSimulation
`Changed`
 - Modified Audiometry maximum attenuation to 120 dB


## [M20180319] - AudioToolkit_v1.0_20180319

### Binaural
`Changed`
- In the BeginSetup method is necessary to indicate the radious of the sphere where the HRTF has been measured.
  * old: void HRTF::BeginSetup(int32_t _HRIRLength);	
  * new: void HRTF::BeginSetup(int32_t _HRIRLength, **float distance**);	
  
- Calculate the azimuth and elevation to get the HRIR in a more precise way (calcutating the intersection between the sphere where the HRTF have been measured and the line that connects the listener's ear to the source). Implemented in the CSingleSourceDSP.

- Modified ProcessHRTF to calculate a more precise ITD 

- Modified the return value of the following methods:
  * change: const **std::vector<CMonoBuffer<float>>** CalculateHRIR_partitioned_FromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const;
  * change: const **std::vector<CMonoBuffer<float>>** GetHRIR_partitioned_InterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const;

- Removed an unnecessary result_warning in CalculateHRIR_InPoles

- Editorial changes in the following methods
  * old: CBRIR::GetBRIROneSubfilterLeng~~ht~~, CILD::AddILDSpatial~~zi~~ationTable, CListener::GetMinimu~~n~~DistanceToSource
  * new: CBRIR::GetBRIROneSubfilterLeng**th**, CILD::AddILDSpatial**iz**ationTable, CListener::GetMinimu**m**DistanceToSource

 `Added` 
 - New interface to know the listener ear local position in CListener
   * new: Common::CVector3 GetListenerEarLocalPosition(Common::T_ear ear) const;
 
 - New method in the CHRTF to know the Distance where the HRTF has been measured, set in the BeginSetup method
   * new: float GetHRTFDistanceOfMeasurement();
   
 - New mechanism to get the interpolated ITD, based on getting the delay of the HRIR from the azimuth and elevation of the listener head center.
   * new: const float GetHRIRDelayInterpolationMethod(Common::T_ear ear, float _azimuth, float _elevation) const;
   * new: const float CalculateHRIRDelayFromBarycentricCoordinates(Common::T_ear ear, TBarycentricCoordinatesStruct barycentricCoordinates, orientation orientation_pto1, orientation orientation_pto2, orientation orientation_pto3)const;
 
### Common
`Changed`

-  Minor changes for MacOS compilation of Error Handler

- Error handler is now enabled

- Editorial changes in method
  * old: CBiquadFilter::UpdateAttributesAfterCross~~F~~ading	
  * new: CBiquadFilter::UpdateAttributesAfterCross**f**ading
  
`Added`
  - New define in the conventions of _3DTI_AXIS_CONVENTION_BINAURAL_TEST_APP: **define LEFT_AXIS AXIS_Y**
  
### HAHLSimulation
`Changed`
- Fixed ratio calculation in Hearing Loss Simulation function CalculateRatioFromDBHL
