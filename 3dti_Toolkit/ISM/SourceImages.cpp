#include "SourceImages.h"

//#ifndef THRESHOLD_BORDER
//#define THRESHOLD_BORDER 0.2f
//#endif



//FIXME: the condition of visibility is wrong and should be fixed only the first reflection after the source is checked 
int SourceImages::getNumberOfVisibleImages(int reflectionOrder, Common::CVector3 listenerLocation)
{
	int subtotal = 0;
	if (reflectionOrder > 0)
	{
		reflectionOrder--;
		for (int i = 0; i < images.size(); i++)
		{
			Common::CVector3 reflectionPoint = images.at(i).getReflectionWall().getIntersectionPointWithLine(images[i].getLocation(), listenerLocation);
			float distanceToBorder, sharpness;
			if (images.at(i).getReflectionWall().checkPointInsideWall(reflectionPoint, distanceToBorder, sharpness) > 0)
			{
				subtotal++;
			}
			subtotal += images.at(i).getNumberOfVisibleImages(reflectionOrder, listenerLocation);
		}
		return subtotal;
	}
	else
	{
		return 0;
	}

}

void SourceImages::setLocation(Common::CVector3 _location)
{
	sourceLocation = _location;
	updateImages();
}

Common::CVector3 SourceImages::getLocation()
{
	return sourceLocation;
}

std::vector<SourceImages> SourceImages::getImages()
{
	return images;
}

void SourceImages::getImageLocations(std::vector<Common::CVector3> &imageSourceList,
	int reflectionOrder)
{
	if (reflectionOrder > 0)
	{
		reflectionOrder--;
		for (int i = 0; i < images.size(); i++)
		{
			if (images.at(i).reflectionWall.isActive())
			{
				imageSourceList.push_back(images.at(i).getLocation());
				if (reflectionOrder > 0)
				{
					images.at(i).getImageLocations(imageSourceList, reflectionOrder);
				}
			}
		}
	}

}


void SourceImages::getImageData(std::vector<ImageSourceData> &imageSourceDataList, Common::CVector3 listenerLocation, int reflectionOrder)
{
	if (reflectionOrder > 0)
	{
		reflectionOrder--;
		for (int i = 0; i < images.size(); i++)
		{
			if (images.at(i).reflectionWall.isActive())//////////////////////////////////////////////////////////////////
			{
				ImageSourceData temp;
				temp.location = images.at(i).getLocation();
				Common::CVector3 reflectionPoint = images.at(i).getReflectionWall().getIntersectionPointWithLine(images.at(i).getLocation(), listenerLocation);
				float distanceToBorder, sharpness;
				if (images.at(i).getReflectionWall().checkPointInsideWall(reflectionPoint, distanceToBorder, sharpness) > 0)
				{
					temp.visible = true;
				}
				else
				{
					temp.visible = false;
				}
				imageSourceDataList.push_back(temp);

				if (reflectionOrder > 0)
				{
					images.at(i).getImageData(imageSourceDataList, listenerLocation, reflectionOrder);
				}
			}
		}
	}

}


void SourceImages::setReflectionWall(Wall _reflectionWall)
{
	reflectionWall = _reflectionWall;
}

Wall SourceImages::getReflectionWall()
{
	return reflectionWall;
}

void SourceImages::createImages(Room _room, Common::CVector3 listenerLocation, int reflectionOrder)
{
	reflectionOrder--;
	std::vector<Wall> walls = _room.getWalls();
	for (int i = 0; i < walls.size(); i++)
	{
		if (walls.at(i).isActive())
		{
			SourceImages tempSourceImage;
			Common::CVector3 tempImageLocation = walls[i].getImagePoint(sourceLocation);

			// if the image is closer to the listener than the previous original, that reflection is not real and should not be included
			// this is equivalent to determine wether source and listener are on the same side of the wall or not
			if ((listenerLocation - sourceLocation).GetDistance() < (listenerLocation - tempImageLocation).GetDistance())
			{
				tempSourceImage.setLocation(tempImageLocation);
				tempSourceImage.setReflectionWall(walls.at(i));

				if (reflectionOrder > 0)
				{
					// We need to calculate the image room before asking for all the new images
					Room tempRoom;
					for (int j = 0; j < walls.size(); j++)
					{
						Wall tempWall = walls.at(i).getImageWall(walls.at(j));
						tempRoom.insertWall(tempWall);
					}
					tempSourceImage.createImages(tempRoom, listenerLocation, reflectionOrder);
					surroundingRoom = tempRoom;
				}

				images.push_back(tempSourceImage);
			}
		}
	}
}

void SourceImages::updateImages()
{
	for (int i = 0; i < images.size(); i++)
	{
		//FIXME: When some images disappear or reappear, this has to be done differently
		images[i].setLocation(images.at(i).getReflectionWall().getImagePoint(sourceLocation));
	}
}

void SourceImages::refreshImages(Room _room, Common::CVector3 listenerLocation, int reflectionOrder)
{
	images.clear();
	createImages(_room, listenerLocation, reflectionOrder);
}


