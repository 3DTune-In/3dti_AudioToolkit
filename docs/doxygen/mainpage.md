# 3D-Tune-In AudioToolkit

### 3D Audio Spatialiser and Hearing Aid and Hearing Loss Simulation

The 3DTI Toolkit is a standard C++ library for audio spatialisation and simulation using  headphones developed within the 3D Tune-In (3DTI) project (http://www.3d-tune-in.eu), which aims at using 3D sound and simulating hearing loss and hearing aids within virtual environments and games. The Toolkit allows the design and rendering of highly realistic and immersive 3D audio, and the simulation of virtual hearing aid devices and of different typologies of hearing loss.

## 3dti AudioToolkit Components

### Toolkit
**[Binaural Spatialiser](@ref Binaural)**

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

**[Hearing Loss (HL) and Hearing Aid (HA) Simulator](@ref HAHLSimulation)**

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



## Credits

This software was developed by a team coordinated by 

-	Arcadio Reyes-Lecuona ([University of Malaga](https://www.uma.es/)). Contact: <areyes@uma.es>
 
-	Lorenzo Picinali ([Imperial College London](https://www.imperial.ac.uk/)).Contact: <l.picinali@imperial.ac.uk>

The members of the development team are (in alphabetical order):
- [Maria Cuevas-Rodriguez](https://github.com/mariacuevas) (University of Malaga)
- [Carlos Garre](https://github.com/carlosgarre) (University of Malaga)
- [Daniel Gonzalez-Toledo](https://github.com/dgonzalezt) (University of Malaga)
- [Luis Molina-Tanco](https://github.com/lmtanco) (University of Malaga)
- [Ernesto de la Rubia](https://github.com/ernestodelarubia) (University of Malaga)

Other contributors:
- David Poirier-Quinot (Imperial College London) produced filter coefficients to simulate near field effects and high performance spatialization. He also contributed in the design of algorithms implemented in the Hearing Loss simulator.

## Copyright and License

The 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package are both Copyright (c) University of Malaga and Imperial College London – 2018.

As copyright owners, University of Malaga and Imperial College London can license the 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package under different license terms, and offer the following licenses for the 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package:

- GPL v3, a popular open-source license with strong copyleft conditions (the default license)
- Commercial or closed-source licenses

If you license the 3D Tune-In Toolkit or the 3D Tune-In Resource Management Package under GPL v3, there is no license fee or signed license agreement: you just need to comply with the GPL v3 terms and conditions. See [3DTI_AUDIOTOOLKIT_LICENSE](3DTI_AUDIOTOOLKIT_LICENSE.html) and [LICENSE](LICENSE.html) for further information.

If you purchase a commercial or closed-source license for the 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package, you must comply with the terms and conditions listed in the associated license agreement; the GPL v3 terms and conditions do not apply. For more information about the commercial license, contact Arcadio Reyes-Lecuona (<areyes@uma.es>) or Lorenzo Picinali (<l.picinali@imperial.ac.uk>).

The 3D Tune-In Toolkit and the 3D Tune-In Resource Management Package software themself remain the same: the only difference between an open-source 3D Tune-In Toolkit and a commercial 3D Tune-In Toolkit are the license terms. That is also the case of the 3D Tune-In Resource Management Package.

## Acknowledgements 

![](EU_Flag.png) This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement No 644051. 

We would like to acknowledge Dr. Brian FG Katz and his team for their contributions in the field of sound spatialization, which were used as the basis for part of this software.
