/**
*
* \brief Implementation of 3D Tune In Classification Scale for Hearing loss characterization
* \date	November 2017
*
* \authors 3DI-DIANA Research Group (University of Malaga), in alphabetical order: M. Cuevas-Rodriguez, C. Garre,  D. Gonzalez-Toledo, E.J. de la Rubia-Cuestas, L. Molina-Tanco ||
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
#include <HAHLSimulation/ClassificationScaleHL.h>

namespace HAHLSimulation {

	float GetSlope(int scale)
	{
		switch (scale)
		{
			/* Values used in the first implementation 
			case 0: return 10;  break;
			case 1: return 21;  break;
			case 2: return 33;  break;
			case 3: return 48;  break;
			case 4: return 63;  break;
			case 5: return 81;  break;
			case 6: return 91;  break;
			*/
			case 0: return  0;  break;
			case 1: return 10;  break;
			case 2: return 20;  break;
			case 3: return 30;  break;
			case 4: return 40;  break;
			case 5: return 50;  break;
			case 6: return 60;  break;
		}
		return 0;
	}

	float GetSeverity(int severity)
	{
		switch (severity)
		{
			case 0: return   0;  break;
			case 1: return  21;  break;
			case 2: return  33;  break;
			case 3: return  48;  break;
			case 4: return  63;  break;
			case 5: return  81;  break;
			case 6: return 100;  break;
		}
		return 0;
	}

	//---------------------------------------------------------------------------------------
	void GetClassificationScaleHL(char curve, int slope, int severity, TAudiometry &hl)
	{
		float x = GetSlope   (slope   );
		float s = GetSeverity(severity);

		hl.resize( 9 );		

		switch (curve)
		{
			case 'A': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =     0; hl[4] =     0; hl[5] =     0; hl[6] =   x/2; hl[7] =   x; hl[8] =   x; break;
			case 'B': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =     0; hl[4] =     0; hl[5] =   x/2; hl[6] =     x; hl[7] =   x; hl[8] =   x; break;
			case 'C': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =     0; hl[4] =   x/2; hl[5] =     x; hl[6] =     x; hl[7] =   x; hl[8] =   x; break;
			case 'D': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =   x/2; hl[4] =     x; hl[5] =     x; hl[6] =     x; hl[7] =   x; hl[8] =   x; break;
			case 'E': hl[0] = 0; hl[1] = 0; hl[2] = x/2; hl[3] =     x; hl[4] =     x; hl[5] =     x; hl[6] =     x; hl[7] =   x; hl[8] =   x; break;
			case 'F': hl[0] = 0; hl[1] = 0; hl[2] =   x; hl[3] =   x/2; hl[4] =   x/2; hl[5] =   x/2; hl[6] =   x/2; hl[7] = x/2; hl[8] = x/2; break;
			case 'G': hl[0] = 0; hl[1] = 0; hl[2] = x/2; hl[3] =     x; hl[4] =   x/2; hl[5] =   x/2; hl[6] =   x/2; hl[7] = x/2; hl[8] = x/2; break;
			case 'H': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =   x/2; hl[4] =     x; hl[5] =   x/2; hl[6] =   x/2; hl[7] = x/2; hl[8] = x/2; break;
			case 'I': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =     0; hl[4] =   x/2; hl[5] =     x; hl[6] =   x/2; hl[7] = x/2; hl[8] = x/2; break;
			case 'J': hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =     0; hl[4] =     0; hl[5] =   x/2; hl[6] =     x; hl[7] = x/2; hl[8] = x/2; break;
			case 'K': hl[0] = 0; hl[1] = 0; hl[2] = x/6; hl[3] = 2*x/6; hl[4] = 3*x/6; hl[5] = 4*x/6; hl[6] = 5*x/6; hl[7] =   x; hl[8] =   x; break;
			default : hl[0] = 0; hl[1] = 0; hl[2] =   0; hl[3] =     0; hl[4] =     0; hl[5] =     0; hl[6] =     0; hl[7] =   0; hl[8] =   0; break;
		}
		for (int c = 0; c < hl.size(); c++)
			hl[c] += s;
	}

}// end namespace HAHLSimulation