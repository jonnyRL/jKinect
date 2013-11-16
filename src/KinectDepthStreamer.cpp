#include "KinectDepthStreamer.h"

namespace jrl
{
	///Constructor
	KinectDepthStreamer::KinectDepthStreamer()
	{
	}

	///Destructor
	KinectDepthStreamer::~KinectDepthStreamer()
	{
	}

	///Base class initialization extension.
	HRESULT KinectDepthStreamer::subsystemInit()
	{
		HRESULT hr;
		//DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
		//hr = m_pNuiSensor->NuiInitialize( nuiFlags );
		DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH;
		hr = m_pNuiSensor->NuiInitialize( nuiFlags );

		if ( E_NUI_SKELETAL_ENGINE_BUSY == hr ){
			nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
			hr = m_pNuiSensor->NuiInitialize( nuiFlags) ;
		}

		if ( FAILED( hr ) ){
			if ( E_NUI_DEVICE_IN_USE == hr ){

			}else{
			}
			return hr;
		}

		hr = depthSubSys.subsystemInit(m_pNuiSensor);
		return hr;
	}

	///Base class uninitialization extension.
	void KinectDepthStreamer::subsystemUnInit()
	{
		depthSubSys.subsystemUnInit();
	}
	//-------------------------------------------------------------------
	///Thread function
	///@return The color-only thread function.
	DWORD WINAPI KinectDepthStreamer::subsystemThreadFunction()
	{
		const int numEvents = 2;
		HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, depthSubSys.getEventHandle()};
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
			}
		}
		return 0;
	}

}