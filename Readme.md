# 3dti AudioToolkit

### 3D Audio Spatialiser and Hearing Aid and Hearing Loss Simulation

The 3DTI Toolkit is a standard C++ library for audio spatialisation and simulation using  headphones developed within the 3D Tune-In (3DTI) project (http://www.3d-tune-in.eu), which aims at using 3D sound and simulating hearing loss and hearing aids within virtual environments and games. The Toolkit allows the design and rendering of highly realistic and immersive 3D audio, and the simulation of virtual hearing aid devices and of different typologies of hearing loss.

Technical details about the 3D Tune-In Toolkit spatialiser are described in:

Cuevas-Rodríguez M, Picinali L, González-Toledo D, Garre C, de la Rubia-Cuestas E, Molina-Tanco L and Reyes-Lecuona A. (2019) 3D Tune-In Toolkit: An open-source library for real-time binaural spatialisation. PLOS ONE 14(3): e0211899. https://doi.org/10.1371/journal.pone.0211899

**The structure of the repository is as follows:**
```
3dti_AudioToolkit
├── 3dti_Toolkit
│   ├── BinauralSpatialiser
│   ├── HLHASimulator
│   ├── Common
│   └── ISM
├── 3dti_ResourceManager
│   ├── HRTF
│   ├── BRIR
│   ├── ILD
│   └── third_party_libraries
├── resources
│   ├── AudioSamples
│   ├── BRIR
│   ├── HRTF
│   └── ILD
└── docs
    ├── doxygen
    ├── examples
    └── images
```

## 3dti AudioToolkit Components

### Toolkit
**Binaural Spatialiser**

This contains the declaration and definition files which are used for binaural spatialization. The library includes a real-time 3D binaural audio renderer offering full 3D spatialization. The features of the spatializer are listed below:

* HRIR convolution based on a standard uniformly partitioned Overlap-Save algorithms.
* HRIR barycentric interpolation approach is used among the three closest available HRIRs.
* The acoustic parallax effect is taken into account; left and right HRIRs are selected independently according to the relative angle between each ear and the sound source.
* ITD is managed separatly from the HRIR, also calculated with barycentric interpolation or customized (computing them for a specific user-inputted head circumference).
* ILD simulation, adding an extra ‘shadow’ in the contralateral ear for near-field sound sources, according to the spherical head model.
* Far-field sources simulation, with a low-pass filter emulating frequency-dependent air absorption.
* Spatial reverberation is simulated in real time, using a uniformly partitioned convolution with BRIRs employing a virtual Ambisonic approach.
* The Toolkit supports different sampling rates and can work with different frame size.
* The Toolkit allows to move not only the sound sources, but also the listener, managing all the required geometric calculations.

**Hearing Loss (HL) and Hearing Aid (HA) Simulator**

This contains the declaration and definition files which are used for hearing loss and hearing aid simulation. Hearing loss classes implement the process of hearing loss simulation, through the following components:  

* Multi-band dynamic range compressor/expander, to emulate the frequency- and level-dependent features of hearing loss.
* Automatic configurator of hearing loss emulation from the user-input audiogram.
* Frequency smearing algorithm, for emulating the broadening of the auditory filters.
* Temporal distortion (jitter), for emulating the decrease in the precision of neural synchronization in the midbrain.

Hearing Aid classes implement the process of hearing aid simulation, through a set of components:
* Dynamic equalization and dynamic range compression/expansion, to compensate for different hearing loss curves at different signal levels.
* Band-pass filtering and re-quantisation (i.e. bitrate reduction), in order to simulate the specific acoustic and AD/DA conversion features of a given hearing aid.
* Directional processing (e.g. omnidirectional, cardioid, etc.).
* Easy-to-access integrated controls for general compression rate and tone control (i.e. control of levels for low, mid and high frequency response).

**Common**

This contains the declaration and definition files which are shared by many or all of the other Toolkit components. These files can be grouped in four categories: geometric transformation, signal processing, support for developers and general (audio state, magnitudes, buffer).

### Resource Manager
This folder contains the source code for the tools implemented for the format conversion and reading of the different resource files needed to setup the listener model. These tools include:

- **To manage HRTF files:** SOFA format file reader for HRTF and the 3DTI-HRTF binary format reader (the 3DTI-HRTF is a  cross-platform portable binary format for HRTF data).

- **To manage BRIR files:** SOFA format file reader for BRIR and 3DTI-BRIR binary format reader.

- **To manage ILD files:** 3DTI-ILD binary format reader.


## Third party libraries

The 3D Tune-In Toolkit has partially integrated the Takuya OOURA General purpose FFT library (http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html) and Eigen library for linear algebra (https://eigen.tuxfamily.org/).

The 3D Tune-In Toolkit Resource Management Package uses: 
* Libsofa (Copyright (c) 2013-2014, UMR STMS 9912-Ircam-Centre Pompidou/CNRS/UPMC. https://github.com/sofacoustics/API_Cpp). 

* Cereal - A C11 library for serialization (Grant, W. Shane and Voorhies, Randolph (2017) http://uscilab.github.io/cereal).  


## External content distributed together with this software 

*	HRTF files, corresponding to IRC_1008, IRC_1013, IRC_1022, IRC_1031, IRC_1032, IRC_1048 and IRC_1053, are extracted from the LISTEN database and processed to extract ITD, shortened in different lengths and resampled at different sampling frequencies. 

*	Audio clips “anechoic Guitar” and “Anechoic Speech” are extracted from Music from Archimedes, Bang&Olufsen, 1992. 

## Additional tools

The 3D Tune-In Toolkit includes a Test Application that allows user to test all the capabilities of the 3D Tune-In Toolkit library through a comprehensive GUI. The latest version of the test application can be downloaded from the Release section of this repository (https://github.com/3DTune-In/3dti_AudioToolkit/releases) for three different platforms: Windows, Mac and Linux.

In addition, there are different tools that make use of the Toolkit and allow it to be integrated into different platforms. These tools are listed below, together with the repository where they are available.

*	3DTI Unity Wrapper. The Unity Wrapper allows integration of some of the components of the 3DTI Toolkit in a Unity Scene. These components are packed in the form of a Unity Package that can be found in the Release version of the 3dti_AudioToolkit_UnityWrapper repository. Code and documentation with further details available at: https://github.com/3DTune-In/3dti_AudioToolkit_UnityWrapper
*	3DTI VST Plugin. This plugin is a VST interface for the 3D Tune-In Toolkit. Installers for Windows and Mac can be found in the Release section of the 3dti_AudioToolkit_VST_Plugins repository.  Code and documentation with further details available at: https://github.com/3DTune-In/3dti_AudioToolkit_VST_Plugins. 
*	3DTI Examples. Basic spatialization examples implemented in C++ to show how to use the 3D Tune-In Toolkit library. Code and documentation with further details available at: https://github.com/3DTune-In/3dti_AudioToolkit_Examples
*	3DTI JavaScript Wrapper. JavaScript port of the 3D Tune-In Toolkit that exposed and ported some of the features of the 3DTI Toolkit. Code and documentation with further details available at: https://github.com/3DTune-In/3dti_AudioToolkit_JavaScript.


## Further Reading

For complete documentation on the 3D Tune-In Toolkit, see the doc directory of this distribution.



## Credits

This software is being developed by a team coordinated by 
-	[Arcadio Reyes-Lecuona](https://github.com/areyesl) ([University of Malaga](https://www.uma.es/)). Contact: areyes@uma.es  
-	Lorenzo Picinali ([Imperial College London](https://www.imperial.ac.uk/)). Contact: l.picinali@imperial.ac.uk

The current members of the development team are (in alphabetical order):
- [Maria Cuevas-Rodriguez](https://github.com/mariacuevas) ([University of Malaga](https://www.uma.es/))
- [Daniel Gonzalez-Toledo](https://github.com/dgonzalezt) ([University of Malaga](https://www.uma.es/))
- [Luis Molina-Tanco](https://github.com/lmtanco) ([University of Malaga](https://www.uma.es/))

Former members of the development team are (in alphabetical order):
- [Carlos Garre](https://github.com/carlosgarre) (Left the team Jan 2018)
- [Ernesto de la Rubia](https://github.com/ernestodelarubia) (Left the team Apr 2018)
- [Angel Rodriguez-Rivero](https://github.com/ardgzrivero) (Joined the team Feb 2018 to Jul 2019)

Other contributors:
- David Poirier-Quinot (Imperial College London) produced filter coefficients to simulate near field effects and high performance spatialization. He also contributed in the design of algorithms implemented in the Hearing Loss simulator.
- Mike Krzyzaniak (Imperial College London) implemented the gammatone filters included in the Hearing Loss Simulator.

## Copyright and License

The 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package are both Copyright (c) University of Malaga and Imperial College London – 2018.

As copyright owners, University of Malaga and Imperial College London can license the 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package under different license terms, and offer the following licenses for the 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package:

- GPL v3, a popular open-source license with strong copyleft conditions (the default license)
- Commercial or closed-source licenses

If you license the 3D Tune-In Toolkit or the 3D Tune-In Resource Management Package under GPL v3, there is no license fee or signed license agreement: you just need to comply with the GPL v3 terms and conditions. See [3DTI_AUDIOTOOLKIT_LICENSE](3DTI_AUDIOTOOLKIT_LICENSE) and [LICENSE](LICENSE) for further information.

If you purchase a commercial or closed-source license for the 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package, you must comply with the terms and conditions listed in the associated license agreement; the GPL v3 terms and conditions do not apply. For more information about the commercial license, contact Arcadio Reyes-Lecuona (areyes@uma.es) or Lorenzo Picinali (l.picinali@imperial.ac.uk).

The 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package software themself remain the same: the only difference between an open-source 3D Tune-In Toolkit and a commercial 3D Tune-In Toolkit are the license terms. That is also the case of the 3D Tune-In Resource Management Package.

## Acknowledgements 

![European Union](docs/images/EU_flag.png "European Union") This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No 644051. 

We would like to acknowledge Dr. Brian FG Katz and his team for their contributions in the field of sound spatialization, which were used as the basis for part of this software.

The Baer&Moore frequency smearing model is an implementation of code written by members of the Auditory Perception Group, University of Cambridge, translated and curated in MATLAB by former member Michael Stone, now at University of Manchester. The smearing algorithm is described here: T. Baer and B.C.J. Moore, Effects of spectral smearing on the intelligibility of sentences in the presence of noise, J. Acoust. Soc. Am. 94: 1229-1241 (1993)
