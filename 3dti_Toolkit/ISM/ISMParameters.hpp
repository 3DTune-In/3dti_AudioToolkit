#ifndef _CISM_PARAMETERS_HPP_
#define _CISM_PARAMETERS_HPP_

#include "Room.h"


namespace ISM {
	class CISMParameters {
	public:

		CISMParameters() 
			: sampleRate{ 48000 }
			, maxDistanceSourcesToListener{ 100 }
			, transitionMeters { 0 }
			, staticDistanceCriterion{ false }
			, listenerLocation{ Common::CVector3(0, 0, 0) }
		{			
		}


		int sampleRate;					///< Default sample rate in samples/seconds
		
		Room room;
		float transitionMeters;          // Transition meters associated with the _windowSlopeDistance		
		float maxDistanceSourcesToListener;		// Maximum distance between the listener and each source image to be considered visible
		
		bool staticDistanceCriterion;    // When enabled, the number of potential images is smaller.NO SABEMOS SI DEBE ESTAR. ES UNA SITUACION ESTATICA (NO SE VAN A MOVER LAS FUENTES) AHORRA FUENTES		

		Common::CVector3 listenerLocation;

		

	private:

	};
}
#endif