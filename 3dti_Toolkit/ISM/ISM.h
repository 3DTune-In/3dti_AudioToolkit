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
#include <Common/Buffer.h>
#include <BinauralSpatializer/Core.h>

namespace ISM
{
	
	class CISM 
	{
	public:
		////////////
		// Methods
		////////////

		//ISM();
		
		CISM(Binaural::CCore* _ownerCore);

		/** \brief Initializes the object with a shoebox room
		*	\details creates six walls conforming a shoebox room with 0,0,0 at the center. Wall order is: front, left, right,back, floor, ceiling.
		*			 It must be used right after creating the empty object.
		*	\param [in] width: extension of the room along the Y axis.
		*	\param [in] length: extension of the room along the X axis.
		*	\param [in] height: extension of the room along the Z axis
		*/
		void SetupShoeBoxRoom(float length, float width, float height);

		/** \brief Initializes the object with a shoebox room
		*	\details creates a room with arbitrary geometry by means of defining all its corners and the walls as polygons with those corners
		*	\param [in] roomGeometry: struct containing all the vertices and walls
		*/
		void setupArbitraryRoom(RoomGeometry roomGeometry);

		/** \brief Sets walls' absortion
		*   \details sets the absortion coeficient (absroved energy / incident energy) of each wall of the main room
		*   \param [in] absortions: vector containing an absortion coeficient (frequency independent) of each wall. Same order as in setup
		*/
		void setAbsortion(std::vector<float> _absortionPerWall);

		/** \brief Sets walls' absortion
		*   \details sets the vectror with absortion coeficients (absroved energy / incident energy) of each wall of the main room		*	\details sets the vector with absortion coeficients (absorved energy / incident energy) of each wall of the main room
		*	\param [in] absortions: vector containing the vectors with absortion coeficients of each wall. 
		*/
		void setAbsortion(std::vector<std::vector<float>> _absortionPerBandPerWall);

		/** \brief returns the main room
		*	\details returns a Room object containing the definition of the main room (without image walls)
		*	\param [out] mainRoom.
		*/
		Room getRoom();

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

		/** \brief Sets the maximum distance between the listener and each source image to be considered visible
		*	\details Sources that exceed the maximum distance will be considered non-visible sources.
		*	\param [in] maxDistanceSourcesToListener
		*/
		void setMaxDistanceImageSources(float maxDistanceSourcesToListener);

		/** \brief Returns the maximum distance between the listener and each source image to be considered visible
		*	\details Sources that exceed the maximum distance will be considered non-visible sources.
		*	\param [out] maxDistanceSourcesToListener
		*/
		float getMaxDistanceImageSources();

		/** \brief Returns number of silenced frames
		*	\details calculates the number od silenced frames depending on the maximum distance between the images and the listener
		*	\param [in] maxDistanceSourcesToListener
		*	\param [out] numberOfSilencedFrames
		*/
		int calculateNumOfSilencedFrames(float maxDistanceSourcesToListener);
		
		/** \brief Returns number of silenced samples
		*	\details calculates the number od silenced samples depending on the maximum distance between the images and the listener
		*	\param [in] maxDistanceSourcesToListener
		*	\param [out] numberOfSilencedSamples
		*/
		int calculateNumOfSilencedSamples (float maxDistanceSourcesToListener);

		void setTransitionMetresPerFrame(int transitionMetresPerFrame);
		
		int calculateTransitionMargin();

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
		*	original source (direct path).
		*	\param [out] ImageSourceData: Vector containing the data of the image sources
		*/
		std::vector<ISM::ImageSourceData> getImageSourceData();

		/** \brief Proccess audio buffers to apply wall absortion
		*	\details Process all audio buffers (one per image source) colouring them according to wall absortion
		*			 It does not apply delay, nor attenuation due to distance, nor spatialisation
		*	\param [in] inBuffer: audio input buffer to be copied with colour in outputs
		*	\param [out] outbufffer: vector of buffers with audio to be spatialised for each image source
		*/
		void proccess(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>> &imageBuffers, Common::CVector3 listenerLocation);

		
	private:
		////////////
		// Methods
		////////////

		Binaural::CCore* GetCore() const;		
		shared_ptr<Binaural::CListener> GetListener() const;
		float GetSampleRate();
		/////////////
		// Attributes
		/////////////

		Room mainRoom;							//Main room where the original source reclects. Its walls can be enables or disabled
		//SourceImages originalSource;			//original sound source inside the main room with direct path to the listener
		shared_ptr<SourceImages> originalSource;

		int reflectionOrder;				//Number of reflections t be simulated

		float maxDistanceSourcesToListener; 
		int transitionMetresPerFrame;

		Binaural::CCore* ownerCore;				// owner Core	
		
		friend class SourceImages;
	};

}//namespace ISM

