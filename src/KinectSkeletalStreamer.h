// Declares of KinectStreamerSkeletal class

#ifndef KINECTSKELETALSTREAMER_H
#define KINECTSKELETALSTREAMER_H

#include "Base/KinectStreamerBase.h"
#include "Subsystems/KinectSkeletalSubSys.h"
#include "Subsystems/KinectDepthSubSys.h"

namespace jrl
{
	class KinectSkeletalStreamer:public KinectStreamerBase
	{
	public:


		KinectSkeletalStreamer();
		~KinectSkeletalStreamer();

		KinectDepthSubSys depthSubSys;
		KinectSkeletalSubSys skeletalSubSys;
		
		
	private:
		//Variables


		HRESULT subsystemInit();
		void subsystemUnInit();

		DWORD WINAPI subsystemThreadFunction();

	};
}

#endif