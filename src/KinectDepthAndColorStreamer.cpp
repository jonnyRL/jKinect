/**
\class jrl::KinectDepthAndColorStreamer

A class that integrates the functionality of the Kinect's Depth and Color Subsystem.
*/

#include "KinectDepthAndColorStreamer.h"
#include <iostream>

namespace jrl
{

	///The default constructor.
	KinectDepthAndColorStreamer::KinectDepthAndColorStreamer()
	{
	}

	///The default destructor.
	KinectDepthAndColorStreamer::~KinectDepthAndColorStreamer()
	{
	}

	///Initializes the subsystem.
	HRESULT KinectDepthAndColorStreamer::subsystemInit()
	{
		HRESULT hr;
		//DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
		//hr = m_pNuiSensor->NuiInitialize( nuiFlags );
		DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX  |  NUI_INITIALIZE_FLAG_USES_COLOR;
		hr = m_pNuiSensor->NuiInitialize( nuiFlags );

		if ( FAILED( hr ) ){
			if ( E_NUI_DEVICE_IN_USE == hr ){
				///@ todo Add message: Device in use.
			}else{
			}
			return hr;
		}

		hr = colorSubSys.subsystemInit(m_pNuiSensor);
		hr = depthSubSys.subsystemInit(m_pNuiSensor);

		colorSubSys.setDepthResolutionForSpaceTransformation(depthSubSys.depthResolution);
		
		return hr;
	}

	///Calls the necessary functions to exit the system correctly.
	void KinectDepthAndColorStreamer::subsystemUnInit()
	{
		colorSubSys.subsystemUnInit();
		depthSubSys.subsystemUnInit();
	}
	//-------------------------------------------------------------------
	///Defines the parameters and the main loop that will be running in a separate thread.
	///@return The color and depth thread function.
	DWORD WINAPI KinectDepthAndColorStreamer::subsystemThreadFunction()
	{
		const int numEvents = 3;
		HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, depthSubSys.getEventHandle(), colorSubSys.getEventHandle()};
		int    nEventIdx;

		// Main thread loop
		bool continueProcessing = true;
		while ( continueProcessing ){
			// Wait for any of the events to be signalled
			nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );
			switch ( nEventIdx ){
			case WAIT_TIMEOUT:
				continue;
				// If the stop event, stop looping and exit
			case WAIT_OBJECT_0:
				continueProcessing = false;
				continue;

			case WAIT_OBJECT_0 + 1:
				depthSubSys.gotDepthAlert();
				break;

			case WAIT_OBJECT_0 + 2:
				colorSubSys.gotColorAlert();
				break;
			}
		}
		return 0;
	}

}