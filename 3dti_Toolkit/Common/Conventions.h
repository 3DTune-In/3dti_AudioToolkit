/**
* \class CConventions
*
* \brief Declaration of conventions. 
* \details Defines for global conventions for the core (axis and spherical angles for azimuth and elevation).
* \date	July 2016
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

#ifndef _CONVENTIONS_H_
#define _CONVENTIONS_H_

/*! \file */

// Axis convention pre-sets
// Your project could define, as global preprocessor difinitions, one of the axis convention presets, as a preprocessor directive: 
// _3DTI_AXIS_CONVENTION_OPENFRAMEWORK,	// Openframeworks test apps
// _3DTI_AXIS_CONVENTION_UNITY,			// Unity 5.x
//
// If no convention is defined, the Ambisonic convention is used:
// UP_AXIS is Z					
// RIGHT_AXIS is -Y			
// FORWARD_AXIS is X		


// Spherical angle convention pre-sets
// Your app should define one of the spherical angle convention presets, as a preprocessor directive: 
// _3DTI_ANGLE_CONVENTION_DEFAULT,	// Same as _3DTI_ANGLE_CONVENTION_LISTEN
// _3DTI_ANGLE_CONVENTION_LISTEN 	// IRCAM LISTEN database

///////////////////////////////////////////////////////////////////////

// Name definitions
//typedef enum { AXIS_X, AXIS_Y, AXIS_Z, AXIS_MINUS_X, AXIS_MINUS_Y, AXIS_MINUS_Z } TAxis;
typedef int TAxis;			///< Type definition for defining axis 
#define AXIS_X 0			///< Axis X for defining axis conventions
#define AXIS_Y 1			///< Axis Y for defining axis conventions
#define AXIS_Z 2			///< Axis Z for defining axis conventions
#define AXIS_MINUS_X 3		///< Axis -X for defining axis conventions
#define AXIS_MINUS_Y 4		///< Axis -Y for defining axis conventions
#define AXIS_MINUS_Z 5		///< Axis -Z for defining axis conventions

//typedef enum { CLOCKWISE, ANTICLOCKWISE } TCircularMotion;
typedef int TCircularMotion;	///< Type definition for defining spherical motion
#define CLOCKWISE 0				///< Clockwise motion for defining spherical angle conventions
#define ANTICLOCKWISE 1			///< Anti-clockwise motion for defining spherical angle conventions


/////////////////////////////////////////////////////////////////////
// PRESET IMPLEMENTATION


#if defined(_3DTI_AXIS_CONVENTION_BINAURAL_TEST_APP)

	#define UP_AXIS AXIS_Z			        ///< In the test app Z is the UP direction
	#define RIGHT_AXIS AXIS_MINUS_Y			///< In the test app -Y is the RIGHT direction
	#define FORWARD_AXIS AXIS_X		        ///< In the test app X is the FORWARD direction

#elif defined(_3DTI_AXIS_CONVENTION_UNITY)
	#define UP_AXIS AXIS_Y					///< In Unity 5.x, Y is the UP direction
	#define RIGHT_AXIS AXIS_X				///< In Unity 5.x, X is the RIGHT direction
	#define FORWARD_AXIS AXIS_Z				///< In Unity 5.x, Z is the FORWARD direction

#elif defined ( _3DTI_AXIS_CONVENTION_OPENFRAMEWORK )
	#define UP_AXIS AXIS_MINUS_Z			///< In Open Framework test apps, -Z is the UP direction
	#define RIGHT_AXIS AXIS_X				///< In Open Framework test apps, X is the RIGHT direction
	#define FORWARD_AXIS AXIS_MINUS_Y		///< In Open Framework test apps, -Y is the FORWARD direction

#elif defined(_3DTI_AXIS_CONVENTION_WEBAUDIOAPI)
	#define UP_AXIS AXIS_Y					///< In the Web Audio API, Y is the UP direction
	#define RIGHT_AXIS AXIS_X				///< In the Web Audio API, X is the RIGHT direction
	#define FORWARD_AXIS AXIS_MINUS_Z		///< In the Web Audio API, -Z is the FORWARD direction

#else
	#define UP_AXIS AXIS_Z					///< In Ambisonics, Z is the UP direction
	#define RIGHT_AXIS AXIS_MINUS_Y			///< In Ambisonics, -Y is the RIGHT direction
	#define FORWARD_AXIS AXIS_X				///< In Ambisonics, X is the FORWARD direction

#endif

/////////////////////////////////////////////////////////////////////

#if defined(_3DTI_ANGLE_CONVENTION_DEFAULT)
	#define _3DTI_ANGLE_CONVENTION_LISTEN
#endif

#if defined(_3DTI_ANGLE_CONVENTION_LISTEN)
	#define AZIMUTH_ZERO FORWARD_AXIS		///< In LISTEN database, azimuth=0 is in the front
	#define AZIMUTH_MOTION ANTICLOCKWISE	///< In LISTEN database, azimuth motion is anti-clockwise
	#define ELEVATION_ZERO FORWARD_AXIS		///< In LISTEN database, elevation=0 is in the front
	#define ELEVATION_MOTION ANTICLOCKWISE	///< In LISTEN database, elevation motion is anti-clockwise
#endif

#endif