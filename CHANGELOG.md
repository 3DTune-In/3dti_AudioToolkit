# Change Log
All notable changes to the 3DTuneIn Toolkit will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/).

## [Unreleased]

### Binaural
`Changed`
- Modified GetHRIR_partitioned and ProcessHRTF to calculate a more precise ITD 

- Removed an unnecessary result_warning in CalculateHRIR_InPoles

- Editorial changes in the following methods
  * old: CBRIR::GetBRIROneSubfilterLeng~~ht~~, CILD::AddILDSpatial~~zi~~ationTable, CListener::GetMinimu~~n~~DistanceToSource

  * new: CBRIR::GetBRIROneSubfilterLeng**th**, CILD::AddILDSpatial**iz**ationTable, CListener::GetMinimu**m**DistanceToSource

  
### Common
`Changed`
- Error handler is now enabled

- Editorial changes in method
  * old: CBiquadFilter::UpdateAttributesAfterCross~~F~~ading	
  * new: CBiquadFilter::UpdateAttributesAfterCross**f**ading
  
### HAHLSimulation
`Changed`
- Fixed ratio calculation in Hearing Loss Simulation function CalculateRatioFromDBHL