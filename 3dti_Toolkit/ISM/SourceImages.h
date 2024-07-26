/**
* \class SourceImages
*
* \brief Declaration of SourceImages interface. This class recursively contains the source images implementing the Image Source Methot (ISM) using 3D Tune-In Toolkit
* \date	July 2021
*
* \authors F. Arebola-P�rez and A. Reyes-Lecuona, members of the 3DI-DIANA Research Group (University of Malaga)
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
* \b Acknowledgement: This project has received funding from Spanish Ministerio de Ciencia e Innovaci�n under the SAVLab project (PID2019-107854GB-I00)
*
*/
#pragma once
#include "Room.h"
#include <Common/Vector3.h>
#include <Common/CascadeGraphicEq9OctaveBands.h>

#define VISIBILITY_MARGIN	0.2
#define FRAMES_OF_MARGIN 1    //Number of Frames for Transition


namespace ISM
{
	class CISM;

	//Struct to store all the data of the image sources
	struct ImageSourceData
	{
		Common::CVector3 location;						//Location of the image source
		bool visible;									//If the source is visible it should be rendered
		float visibility;								//1 if visible, 0 if not, something in the middle if in the transition, where the transition is +/-VISIBILITY_MARGIN width
		std::vector<Wall> reflectionWalls;				//list of walls where the source has reflected (last reflection first)
		std::vector<float> reflectionBands;             //coeficients, for each octave Band, to be applied to simulate walls' absortion
	};



	class SourceImages
	{
	public:
		////////////
		// Methods
		////////////
		//SourceImages();
		
		SourceImages(ISM::CISM* _ownerISM);

		//void temp(CISM* a);
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
		std::vector<weak_ptr <SourceImages>> getImages();

		/** \brief Returns the locations of all images but the original source
		*   \details this method recurively goes through the image tree to collect all the image locations
		*   \param [out] imageSourceList: vector containing all image locations.
		*/
		void getImageLocations(std::vector<Common::CVector3> &imageSourceList);

		/** \brief Returns data of all image sources
		*	\details This method returns the location of all image sources and wether they are visible or not, not including the
			original source (direct path).
		*	\param [out] imageSourceDataList: Vector containing the data of the image sources
		*/
		void getImageData(std::vector<ImageSourceData> &imageSourceDataList);

		/** \brief Returns the  wall where the reflecion produced this image
		*   \param [out] Reflection wall.
		*/
		Wall getReflectionWall();

		/** \brief creates all the image sources reflected in the walls upto the reflection order
		*	\details Creates a recursive tree of imagesources using all active walls up to the reflection order depth. This methos should be
					 called every time the room geometry changes (walls are set as active or inactive) or the reflection order changes
		*	\param [in] Room: the original room surounding the original source
		*	\param [in] reflectionOrder: depth of the recursive tree
		*/
		void createImages(Room _room, int reflectionOrder);

		/** \brief updates imege source location, reflection and visibility
		*	\details Updates the recursive image source tree with the source locations and computes refelction coefficients and visibility 
					 to be applied when process
		*/
		void updateImages ();


		/** \brief Adds wall absortion to the sound
		*	\details Recursively process all source images providing an independent buffer for each of them with the original sound filtered
					 by the wall absortions. For non visible sources the output buffer contains zeros
		*	\param [in] inBuffer: original buffer used for all images
		*   \param [out] imageBuffers: vector of buffers with the sound filtered (One buffer per image)
		*	\param [in] listenerLocation: needed to know visibility of sources
		*/
		void processAbsortion(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation);

	private:

		///////////////////
		// Private Methods
		///////////////////

		/** \brief sets the vector of walls where this image was reflected
		*   \param [in] _reflectionWalls.
		*/
		void setReflectionWalls(std::vector<Wall> reflectionWalls);

		/** \brief
		*	\details
		*	\param [in]
		*   \param [in]
		*/
		void createImages(Room _room, int reflectionOrder, std::vector<Wall> reflectionWalls);

		////////////
		// Attributes
		////////////


		std::vector<Wall> reflectionWalls;		//vector containing the walls where the sound has been reflected in inverse order (last reflection first)
		Common::CVector3 sourceLocation;		//Original source location
		//std::vector<SourceImages> images;					//recursive list of images		
		std::vector<shared_ptr<SourceImages>> images;		//recursive list of images

		float visibility = 1.0f;				//1.0 if visible, 0.0 if not, something in the middle if the ray is close to the border of walls
		bool visible = true;					//false when visibility = 0, true otherwise
		std::vector<float> reflectionBands;     //coeficients, for each octave Band, to be applied to simulate walls' absortion

		Common::CascadeGraphicEq9OctaveBands eq; //Graphic Equalizer to simulate walls' absortion

		ISM::CISM* ownerISM;					// TO CHECK if this possible using smart pointer
	};

}//namespace ISM