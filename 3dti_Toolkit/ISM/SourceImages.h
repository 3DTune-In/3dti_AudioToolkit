/**
* \class SourceImages
*
* \brief Declaration of SourceImages interface. This class recursively contains the source images implementing the Image Source Methot (ISM) using 3D Tune-In Toolkit
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
*/
#pragma once
#include "Room.h"
#include <Common/Vector3.h>


struct ImageSourceData
{
	Common::CVector3 location;
	bool visible;
};



class SourceImages
{
	public:
	////////////
	// Methods
	////////////

	/** \brief changes the location of the original source
	*	\details Sets a new location for the original source and updates all images accordingly.
	*   \param [in] _location: new location for the original source.
	*/
	void setLocation(Common::CVector3 _location);

	/** \brief Returns the location of the original source
	*   \param [out] Location: Current location for the original source.
	*/
	Common::CVector3 getLocation();

	/** \brief Returns the first order reflections of the original source
	*   \param [out] Images: vector with the first order reflection images.
	*/
	std::vector<SourceImages> getImages();

	/** \brief Returns the locations of all images but the original source
	*   \details this method recurively goes through the image tree to collect all the image locations
	*   \param [out] imageSourceList: vector containing all image locations.
	*   \param [in] reflectionOrder: needed to trim the recursive tree
	*/
	void getImageLocations(std::vector<Common::CVector3> &imageSourceList, int reflectionOrder);

	/** \brief Returns data of all image sources
	*	\details This method returns the location of all image sources and wether they are visible or not, not including the
		original source (direct path).
	*	\param [out] ImageSourceData: Vector containing the data of the image sources
	*   \param [in] reflectionOrder: needed to trim the recursive tree
	*/
	void getImageData(std::vector<ImageSourceData> &imageSourceDataList, Common::CVector3 listenerLocation, int reflectionOrder);

	/** \brief Returns the  wall where the reflecion produced this image
	*   \param [out] Reflection wall.
	*/
	Wall getReflectionWall();

	/** \brief Returns the number of images whcih are visible to teh listener (real reflections) 
	*	\details Given a location of the listener, this method computes how many images are active because they are visible to the listener
				 To do so it iterates the whole recursive tree up to a given order checking it the sources are visible or not
	*	\param [in] ReflectionOrder: number of reflections cosidered.
	*   \param [in] ListenerLocation: Location of the Listener in the 3DTI toolkit
	*	\param [out] Number_Images: number of visible images.
	*/
	int getNumberOfVisibleImages(int reflectionOrder, Common::CVector3 listenerLocation);

	/** \brief
	*	\details
	*	\param [in]
	*   \param [in]
	*/
	void createImages(Room _room, Common::CVector3 listenerLocation, int reflectionOrder);

	/** \brief
	*	\details
	*	\param [in]
	*   \param [in]
	*/
	void updateImages();

	/** \brief refresh source images
	*	\details removes all the images and creates them again to refresh which ones are active. This method
	*			 should be invoked after disabling or enabling walls
	*	\param [in]
	*   \param [in]
	*/
	void refreshImages(Room _room, Common::CVector3 listenerLocation, int reflectionOrder);

private:

///////////////////
// Private Methods
///////////////////

/** \brief sets the wall where this image was reflected
*	\details The first source, which model the original (not reflected) source has not a reflection wall, but all teh images
			 should have teh wall where the reflecion is modeled in order to make furtehr calculations
*   \param [in] _reflectionWall.
*/
	void setReflectionWall(Wall _reflectionWall);


	////////////
	// Attributes
	////////////


	std::vector<Wall> walls;								//DEPRECATED: List of walls where the source will be reflected. Not used any more, as each image source reminds its reflection wall
	Wall reflectionWall;									//Wall which produced current image as a reflection
	Room surroundingRoom;									//Room to generate further images reflectin in its walls
	Common::CVector3 sourceLocation;						//Original source location

	std::vector<SourceImages> images;						//recursive list of images


	float diffraction = 1.0f;
};

