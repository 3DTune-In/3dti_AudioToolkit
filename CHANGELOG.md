# Change Log
All notable changes to the 3DTuneIn Toolkit will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/).

## [Unreleased]

### Binaural
`Changed`
- In the BeginSetup method is necessary to indicate the radious of the sphere where the HRTF has been measured.
  * old: void HRTF::BeginSetup(int32_t _HRIRLength);	
  * new: void HRTF::BeginSetup(int32_t _HRIRLength, **float distance**);	
  
- Calculate the azimuth and elevation to get the HRIR in a more precise way (calcutating the intersection between the sphere where the HRTF have been measured and the line that connects the listener's ear to the source). Implemented in the CSingleSourceDSP.

- Modified GetHRIR_partitioned and ProcessHRTF to calculate a more precise ITD 

- Removed an unnecessary result_warning in CalculateHRIR_InPoles

- Editorial changes in the following methods
  * old: CBRIR::GetBRIROneSubfilterLeng~~ht~~, CILD::AddILDSpatial~~zi~~ationTable, CListener::GetMinimu~~n~~DistanceToSource
  * new: CBRIR::GetBRIROneSubfilterLeng**th**, CILD::AddILDSpatial**iz**ationTable, CListener::GetMinimu**m**DistanceToSource

 `Added` 
 - New interface to know the listener ear local position in CListener
   * new: Common::CVector3 GetListenerEarLocalPosition(Common::T_ear ear) const;
 
 - New method in the CHRTF to know the Distance where the HRTF has been measured, set in the BeginSetup method
   * new: float GetHRTFDistanceOfMeasurement();
 
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
