
#ifndef KINECTDEPTHANDCOLORSTREAMER_H
#define KINECTDEPTHANDCOLORSTREAMER_H

#include "Base/KinectStreamerBase.h"
#include "Subsystems/KinectDepthSubSys.h"
#include "Subsystems/KinectColorSubSys.h"

namespace jrl
{
	class KinectDepthAndColorStreamer:public KinectStreamerBase
	{
	public:

		KinectDepthAndColorStreamer();
		~KinectDepthAndColorStreamer();

		KinectDepthSubSys depthSubSys;
		KinectColorSubSys colorSubSys;

	private:
		//Variables

		HRESULT subsystemInit();
		void subsystemUnInit();

		DWORD WINAPI subsystemThreadFunction();

	};
}
#endif