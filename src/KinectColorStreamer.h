#ifndef KINECTCOLORSTREAMER_H
#define KINECTCOLORSTREAMER_H

#include "Base/KinectStreamerBase.h"
#include "Subsystems/KinectColorSubSys.h"

namespace jrl
{
	class KinectColorStreamer:public KinectStreamerBase
	{
	public:


		KinectColorStreamer();
		~KinectColorStreamer();
		
		KinectColorSubSys colorSubSys;

	private:
		//Variables

		HRESULT subsystemInit();
		void subsystemUnInit();		

		DWORD WINAPI subsystemThreadFunction();

	};

}

#endif