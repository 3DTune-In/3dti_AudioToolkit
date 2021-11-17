/**
*
* \brief This is the header file of the API class for a reverberation renderer based in the Image Source Methot (ISM) 
* \date	July 2021
*
* \authors F. Arebola-Pérez and A. Reyes-Lecuona, members of the 3DI-DIANA Research Group (University of Malaga)
* \b Contact: A. Reyes-Lecuona as head of 3DI-DIANA Research Group (University of Malaga): areyes@uma.es
*
* \b Contributions: (additional authors/contributors can be added here)
*
* \b Project: SAVLab (Spatial Audio Virtual Laboratory) ||
* \b Website:
*
* \b Copyright: University of Malaga - 2021
*
* \b Licence: GPLv3
*
* \b Acknowledgement: This project has received funding from Spanish Ministerio de Ciencia e Innovación under the SAVLab project (PID2019-107854GB-I00)
*
*/#pragma once

#include "Room.h"
#include "SourceImages.h"
#include <Common/Vector3.h>


class ISM
{
public:
	////////////
	// Methods
	////////////

	/** \brief Initializes the object with a shoebox room
	*	\details creates six walls conforming a shoebox room with 0,0,0 at the center. It must be used right after
				 creating the empty object.
	*	\param [in] width: extension of the room along the Y axis.
	*	\param [in] length: extension of the room along the X axis.
	*	\param [in] height: extension of the room along the Z axis
	*/
	void SetupShoeBoxRoom(float length, float width, float height); 

	/** \brief Makes one of the room's walls active
	*	\details Sets the i-th wall of the room as active and therefore reflective.
	*	\param [in] index of the wall to be active.
	*/
	void enableWall(int wallIndex);

	/** \brief Makes one of the room's walls transparent
	*	\details Sets the i-th wall of the room as not active and therefore transparent.
	*	\param [in] index of the wall to be active.
	*/
	void disableWall(int wallIndex);

	/** \brief Sets the number of reflections to be simulated
	*	\details The ISM method simulates reflections using images. This parameter sets the number of reflections simulated
	*	\param [in] reflectionOrder
	*/
	void setReflectionOrder(int reflectionOrder);

	/** \brief Returns the number of reflections to be simulated
	*	\details The ISM method simulates reflections using images. This parameter sets the number of reflections simulated
	*	\param [out] reflectionOrder
	*/
	int getReflectionOrder();

	/** \brief Sets the source location
	*	\details This method sets the location of the original source (direct path).
	*	\param [in] location: location of the direct path source
	*/
	void setSourceLocation(Common::CVector3 location);

	/** \brief Returns the source location
	*	\details This method returns the location of the original source (direct path).
	*	\param [out] location: location of the direct path source
	*/
	Common::CVector3 getSourceLocation();

	/** \brief Returns the location of image sources
	*	\details This method returns a vector with the location of the image sources (reflectionsImage). The original source is not included
	*	\param [out] location: location of the direct path source
	*/
	std::vector<Common::CVector3> getImageSourceLocations();

	/** \brief Returns data of all image sources
	*	\details This method returns the location of all image sources and wether they are visible or not, not including the 
		original source (direct path). 
	*	\param [out] ImageSourceData: Vector containing the data of the image sources
	*/
	std::vector<ImageSourceData> getImageSourceData(Common::CVector3 listenerLocation);

private:
	////////////
	// Methods
	////////////


	/////////////
	// Attributes
	/////////////

	Room mainRoom;							//Main room where the original source reclects. Its walls can be enables or disabled
	SourceImages originalSource;			//original sound source inside the main room with direct path to the listener
	int reflectionOrder = 1;				//Number of reflections t be simulated

};

