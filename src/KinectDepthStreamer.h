// Declares of KinectStreamerDepth class

#ifndef KINECTDEPTHSTREAMER_H
#define KINECTDEPTHSTREAMER_H

#include "Base/KinectStreamerBase.h"
#include "Subsystems/KinectDepthSubSys.h"

namespace jrl
{
	class KinectDepthStreamer:public KinectStreamerBase
	{
	public:

		KinectDepthStreamer();
		~KinectDepthStreamer();

		KinectDepthSubSys depthSubSys;

	private:
		//Variables

		HRESULT subsystemInit();
		void subsystemUnInit();

		DWORD WINAPI subsystemThreadFunction();

	};
}
#endif