
# Change Log
All notable changes to the 3DTuneIn Toolkit will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/).

## [Unreleased]
### Binaural
`Changed`
- Added new parameters to GetHRIR_partitioned to calculate ITD
  * old method: oneEarHRIR_Partitioned_struct GetHRIR_partitioned(Common::T_ear ear, float _azimuth, float _elevation, bool runTimeInterpolation);
  * new method: oneEarHRIR_Partitioned_struct GetHRIR_partitioned(Common::T_ear ear, float _azimuth, float _elevation, float _azimuthCenter, float _elevationCenter, bool runTimeInterpolation);

### Common
`Changed`
- Annotation 1
  * Referenced code (class, function,...)
- Annotation 2
  * Referenced code (class, function,...)
 
