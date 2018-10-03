# Change Log
All notable changes to the 3DTuneIn Toolkit will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/).

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
