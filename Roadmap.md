This document outlines the development plan from a high level and will be updated as progress is made toward the following versions.

> Everything contained in this document is in draft form and subject to change at any time and provided for information purposes only. This is not a guarantee of accuracy of the information contained in this roadmap and the information is provided “as is” with no representations or warranties, express or implied.

# Upcoming changes and updates
- UML driagram. A UML diagram of all classes will be produced for documentation purposes.
- HRTF, BRIR and ILD files at 24000 Hz. The 3D Tune-In Toolkit currently support any sampling frequency. However, HRTF, BRIR and ILD files are currently provided only at 44100 Hz, 48000 Hz and 96000 Hz.  
- Multiple configuration of virtual ambisonic reverberation. Currently, the 3D Tune-In Toolkit implements a 2D virtual ambisonic approach based on three ambisonic channels (W, X and Y). It is planned to make it configurable, allowing the selection of different channel configurations: W (mono), W + Y  (stereo), W + X + Y (2D) and W + X + Y + Z (Full 3D).
- Loudspeaker Spatialisation. Currently the Toolkit is focused on binaural spatialisation. In the near future, spatialisation based on 2nd order virtual Ambisonic for loudspeaker setups will be added.
- Linux support. The library is coded in standard C++ 14, which makes it portable to multiple platforms. However, it has been compiled and tested only in Windows (32 and 64 bits), MacOS, Android (using NDK) and iOS. It is planned to include Linux in the near future.