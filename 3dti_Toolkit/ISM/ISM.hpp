#ifndef _ISM_HPP_
#define _ISM_HPP_

#include "Room.h"
#include "SourceImages.hpp"
#include "ISMParameters.hpp"
#include <Common/Vector3.h>
#include <Common/Buffer.h>
#include <BinauralSpatializer/Core.h>

namespace ISM
{

	class CISM {
	public:
		
		CISM(Binaural::CCore* _ownerCore) 
			: ownerCore{ _ownerCore }
			, setupDone{ false }
			, reflectionOrder{ 1 }
			, ISMParameters{ std::make_shared<CISMParameters>() }			
			, sourceLocation{ Common::CVector3(0, 0, 0) }
			, imageSources{ nullptr }
			, imageSourcesPositionList{ std::vector<Common::CVector3>() }
			, imageSourcesDataList{ std::vector<ImageSourceData>() }
		{			
			ISMParameters->sampleRate = ownerCore->GetAudioState().sampleRate;						
		}

		/**
		 * @brief Initializes or reinitializes the image source model with the specified parameters for room acoustics simulation.
		 * @param order The reflection order to use for the image source model.
		 * @param _maxDistanceSourcesToListener The maximum allowed distance from image sources to the listener.
		 * @param _windowSlopeDistance The window slope distance parameter used in distance calculations.
		 * @param _room The room configuration to use for the simulation.
		 * @return Returns true if the setup was successful; returns false if the parameters are invalid or setup fails.
		 */
		bool Setup(const int& order, const float& _maxDistanceSourcesToListener, const float& _windowSlopeDistance, const Room& _room)
		{
			if (setupDone)
			{
				setupDone = false;
				imageSources->Reset();
				imageSources.reset();				
			}

			ISMParameters->room = _room;			
			reflectionOrder = order;
			bool result = setMaxDistanceImageSources(_maxDistanceSourcesToListener, _windowSlopeDistance);
			if (!result)
			{
				SET_RESULT(RESULT_ERROR_INVALID_PARAM, "MaxDistanceSourcesToListener must be smaller than windowSlopeDistance/2");
				return false;
			}

			UpdateListenerPosition();

			imageSources = std::make_shared<SourceImages>(ISMParameters);
			imageSources->createImagesTree(ISMParameters->room, reflectionOrder, sourceLocation);
			UpdateImageSourceDataFromImageTree();
			// TODO check if everything went fine before setting setupDone to true
			setupDone = true;
			return true;
		}
			
		/** \brief Sets walls' absortion
		*   \details sets the absortion coeficient (absroved energy / incident energy) of each wall of the main room
		*   \param [in] absortions: vector containing an absortion coeficient (frequency independent) of each wall. Same order as in setup
		*/
		//void SetRoomWallsAbsortion(std::vector<float> absortionPerWall)		
		//{
		//	// Check if dimensions of input vctor and walls fit
		//	if (absortionPerWall.size() != ISMParameters->room.getWalls().size())
		//	{
		//		SET_RESULT(RESULT_ERROR_BADSIZE, "Size of vector of absortions per wall and numbar of walls are different");
		//		return;
		//	}

		//	for (int i = 0; i < ISMParameters->room.getWalls().size(); i++)
		//	{
		//		ISMParameters->room.setWallAbsortion(i, absortionPerWall.at(i));
		//	}
		//	//originalSource->createImages(mainRoom, reflectionOrder);
		//	//TODO CALL UpdateImagesAbsortionCoeficients
		//}

		/** \brief Sets walls' absortion
		*   \details sets the vectror with absortion coeficients (absroved energy / incident energy) of each wall of the main room		*	\details sets the vector with absortion coeficients (absorved energy / incident energy) of each wall of the main room
		*	\param [in] absortions: vector containing the vectors with absortion coeficients of each wall.
		*/
		//void setAbsortion(std::vector<std::vector<float>> absortionPerBandPerWall)		
		//{
		//	// Check the number of bands and the number of walls
		//	if (absortionPerBandPerWall.size() != ISMParameters->room.getWalls().size())
		//	{
		//		SET_RESULT(RESULT_ERROR_BADSIZE, "Size of vector of absortion profiles per wall and numbar of walls are different");
		//		return;
		//	}

		//	for (int i = 0; i < ISMParameters->room.getWalls().size(); i++)
		//	{
		//		ISMParameters->room.setWallAbsortion(i, absortionPerBandPerWall.at(i));
		//	}
		//	//originalSource->createImages(mainRoom, reflectionOrder);
		//	//TODO CALL UpdateImagesAbsortionCoeficients
		//}

		/** \brief returns the main room
		*	\details returns a Room object containing the definition of the main room (without image walls)
		*	\param [out] mainRoom.
		*/
		Room getRoom()		
		{
			return ISMParameters->room;
		}

		/** \brief Makes one of the room's walls active
		*	\details Sets the i-th wall of the room as active and therefore reflective.
		*	\param [in] index of the wall to be active.
		*/
		//void enableWall(int wallIndex)		
		//{
		//	ISMParameters->room.enableWall(wallIndex);
		//	//imageSources->createImages(mainRoom, reflectionOrder);

		//	//TODO DO SETUP AGAIN
		//}

		/** \brief Makes one of the room's walls transparent
		*	\details Sets the i-th wall of the room as not active and therefore transparent.
		*	\param [in] index of the wall to be active.
		*/
		//void disableWall(int wallIndex)		
		//{
		//	ISMParameters->room.disableWall(wallIndex);
		//	//imageSources->createImages(mainRoom, reflectionOrder);
		//	//TODO DO SETUP AGAIN
		//}

		/** \brief Sets the number of reflections to be simulated
		*	\details The ISM method simulates reflections using images. This parameter sets the number of reflections simulated
		*	\param [in] reflectionOrder
		*/
		/*void setReflectionOrder(int _reflectionOrder)		
		{
			reflectionOrder = _reflectionOrder;
			originalSource->createImages(mainRoom, reflectionOrder);
		}*/

		/** \brief Returns the number of reflections to be simulated
		*	\details The ISM method simulates reflections using images. This parameter sets the number of reflections simulated
		*	\param [out] reflectionOrder
		*/
		int getReflectionOrder()
		{
			return reflectionOrder;
		}

		

		/** \brief Returns the maximum distance between the listener and each source image to be considered visible
		*	\details Sources that exceed the maximum distance will be considered non-visible sources.
		*	\param [out] maxDistanceSourcesToListener
		*/
		float getMaxDistanceImageSources()		
		{
			return ISMParameters->maxDistanceSourcesToListener;
		}
		
		/** \brief Sets the source location
		*	\details This method sets the location of the original source (direct path).
		*	\param [in] location: location of the direct path source
		*/
		void setSourceLocation(const Common::CVector3& location)		
		{
			sourceLocation = location;
			if (!setupDone) 
				return;
			
			if (imageSources != nullptr)
			{
				UpdateListenerPosition();
				imageSources->UpdateSourceLocation(location);
				UpdateImageSourceDataFromImageTree();
			}			
		}

		/** \brief Returns the source location
		*	\details This method returns the location of the original source (direct path).
		*	\param [out] location: location of the direct path source
		*/
		Common::CVector3 getSourceLocation()		
		{
			return sourceLocation;			
		}

		/** \brief Returns the location of image sources
		*	\details This method returns a vector with the location of the image sources (reflectionsImage). The original source is not included
		*	\param [out] location: location of the direct path source
		*/
		std::vector<Common::CVector3> getImageSourceLocations()		
		{
			return imageSourcesPositionList;
			/*std::vector<Common::CVector3> imageSourceList;
			imageSources->getImageLocations(imageSourceList);
			return imageSourceList;*/
		}

		/** \brief Returns data of all image sources
		*	\details This method returns the location of all image sources and wether they are visible or not, not including the
		*	original source (direct path).
		*	\param [out] ImageSourceData: Vector containing the data of the image sources
		*/
		std::vector<ISM::ImageSourceData> getImageSourceData()	
		{
			std::lock_guard<std::mutex> l(mutex);
			return imageSourcesDataList;
		}

		void SetListenerPosition() {
			UpdateListenerPosition();
			imageSources->UpdateImagesTreeVisibilities();
			UpdateImageSourceDataFromImageTree();
		}


		/** \brief Proccess audio buffers to apply wall absortion
		*	\details Process all audio buffers (one per image source) colouring them according to wall absortion
		*			 It does not apply delay, nor attenuation due to distance, nor spatialisation
		*	\param [in] inBuffer: audio input buffer to be copied with colour in outputs
		*	\param [out] outbufffer: vector of buffers with audio to be spatialised for each image source
		*/
		void proccess(CMonoBuffer<float> inBuffer, std::vector<CMonoBuffer<float>>& imageBuffers, Common::CVector3 listenerLocation)		
		{
			if (!setupDone) {
				SET_RESULT(RESULT_ERROR_NOTINITIALIZED, "ISM not setup. Call Setup() before processing");
				return;
			}
			UpdateListenerPosition();
			imageSources->processAbsortion(inBuffer, imageBuffers, listenerLocation);

			std::vector<ImageSourceData> images = getImageSourceData();
			ASSERT(imageBuffers.size() == images.size(), RESULT_ERROR_BADSIZE, "Vector of buffers to be processed by ISM should be the same size as the number of image sources", "");

			for (int i = 0; i < imageBuffers.size(); i++)
			{
				if (images.at(i).visible)
				{
					for (int j = 0; j < inBuffer.size(); j++)
					{
						imageBuffers.at(i).at(j) = images.at(i).visibility * imageBuffers.at(i).at(j);
					}
				}
				else
				{
					for (int j = 0; j < inBuffer.size(); j++)
					{
						imageBuffers.at(i).at(j) = 0.0f;
					}
				}
			}

		}

		/** \brief Enable static distance criterion
		*	\details This method reduces the number of potential image sources to be considered according to the distance criterion
		*/
		void enableStaticDistanceCriterion()
		{
			ISMParameters->staticDistanceCriterion = true;
		}

		/** \brief Disable static distance criterion
		*	\details This method establishes a dynamic distance criterion and considers all possible sources as potential sources
		*/
		void disableStaticDistanceCriterion()
		{
			ISMParameters->staticDistanceCriterion = false;
		}

		float GetSampleRate() {
			return ownerCore->GetAudioState().sampleRate;
		}

	private:					

		/** \brief Sets the maximum distance between the listener and each source image to be considered visible
		*	\details Sources that exceed the maximum distance will be considered non-visible sources.
		*	\param [in] maxDistanceSourcesToListener
		*	\param [in] windowSlopeDistance in meters (related to windowSlope time(s) in class CBRIR)
		*/
		bool setMaxDistanceImageSources(float _MaxDistanceSourcesToListener, float _windowSlopeDistance)
		{
			if (_windowSlopeDistance / 2 < _MaxDistanceSourcesToListener)
			{
				ISMParameters->maxDistanceSourcesToListener = _MaxDistanceSourcesToListener;
				ISMParameters->transitionMeters = _windowSlopeDistance;
				return true;
			}
			return false;
		}


		void UpdateListenerPosition() {
			Common::CTransform listenerTransform = ownerCore->GetListener()->GetListenerTransform();
			ISMParameters->listenerLocation = listenerTransform.GetPosition();
		}

		void  UpdateImageSourceDataFromImageTree()
		{
			if (imageSources == nullptr) return;
			std::lock_guard<std::mutex> l(mutex);
			imageSourcesDataList.clear();			
			UpdateListenerPosition();
			imageSources->getImageSourcesData(imageSourcesDataList);
			UpdateImageSourcesPositionList();
		}

		void UpdateImageSourcesPositionList() {
			imageSourcesPositionList.clear();
			for (auto& image : imageSourcesDataList) {
				imageSourcesPositionList.push_back(image.location);
			}
		}

		////////////////
		/// Attributes
		////////////////
		
		mutable std::mutex mutex; // Thread management

		Binaural::CCore* ownerCore;				// owner Core	

		//int sampleRate;							// System sample rate in Hz
		int reflectionOrder;				    // Number of reflections t be simulated
		//float transitionMeters;                 // Transition meters associated with the _windowSlopeDistance
		//float maxDistanceSourcesToListener;		// Maximum distance between the listener and each source image to be considered visible
		//bool staticDistanceCriterion;           // When enabled, the number of potential images is smaller.NO SABEMOS SI DEBE ESTAR. ES UNA SITUACION ESTATICA (NO SE VAN A MOVER LAS FUENTES) AHORRA FUENTES
		//Room room;
		std::shared_ptr<CISMParameters> ISMParameters;

		Common::CVector3 sourceLocation;		// Location of the original source		

		std::shared_ptr<SourceImages> imageSources;
		std::vector<Common::CVector3> imageSourcesPositionList;
		std::vector<ImageSourceData> imageSourcesDataList;

		bool setupDone;

		friend class SourceImages;
	};
}
#endif