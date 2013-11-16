/**
\class jrl::KinectStreamerBase

The base class for all the Kinect Streamers.

*/

#include "KinectStreamerBase.h"
//#include "resource.h"
#include <mmsystem.h>
#include <assert.h>
#include <strsafe.h>
#include <iostream>

namespace jrl{
	
	///The default constructor.
	KinectStreamerBase::KinectStreamerBase():
		m_hInstance(NULL)
	{
		setSetpointAngle(0);
		Nui_Zero();
	}

	///The default destructor.
	KinectStreamerBase::~KinectStreamerBase()
	{
		Nui_Zero();
		//NuiSetDeviceStatusCallback(
	}

	//-------------------------------------------------------------------

	///Initializes the Kinect
	///param[in] deviceIndex The ID of the device to be initialized.
	///return The status code of the operation. True if succesful, false otherwise.
	HRESULT KinectStreamerBase::init(int deviceIndex){
		//m_colorStreamEnabled = colorStream;
		//m_activeNumOfStreams = 0;

		HRESULT  hr;

		//Initialize dummy sensor
		INuiSensor * pNuiSensor;

		// Create the sensor so we can check status
		hr = NuiCreateSensorByIndex(deviceIndex, &pNuiSensor);
        if (FAILED(hr))
        {
            //std::cout<<"Kinect "<<deviceIndex<<" Initialization Failed."<<std::endl;
			return hr;
        }

		// Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_pNuiSensor = pNuiSensor;
			m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
        } else {
			pNuiSensor->Release();
			return hr;
		}
		
		//Initialize sensor
		
		//if ( !m_pNuiSensor )
		//{
		//	hr = NuiCreateSensorByIndex(deviceIndex, &m_pNuiSensor);
		//	if ( FAILED(hr) )
		//	{
		//		return hr;
		//	}
		//	///TODO: Check line, causes error in debug mode
		//	//SysFreeString(m_instanceId);
		//	m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
		//}

		hr = subsystemInit();
		if ( FAILED( hr ) )
		{
			return hr;
		}
	
		// Start the Nui processing thread
		m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
		m_hThNuiProcess = CreateThread(
			NULL,					//default security attributes
			0,						//default stack size
			Nui_ProcessThread,		//thread function name
			this,					//argument to thread function
			0,						//use default creation flags
			NULL					//if a pointer is given, returns the thread identifier
			);


		//Thread Error Handling
		if (m_hThNuiProcess == NULL){
			//Error while creating thread, choose action to take
			unInit();
			//ExitProcess(3);
		}


		setDeviceAngle(m_kinectSetpointAngle);


		////Set exposure values
		//INuiColorCameraSettings* pNuiCameraSettings = nullptr;
		//HRESULT cameraInterfaceObtained = m_pNuiSensor->NuiGetColorCameraSettings(&pNuiCameraSettings);

		//if(SUCCEEDED(cameraInterfaceObtained)){
		//	//Exposure
		//	pNuiCameraSettings->SetAutoExposure(true);
		//	//pNuiCameraSettings->SetGain(2);
		//	pNuiCameraSettings->SetBrightness(0.1);

		//	//Color
		//	pNuiCameraSettings->SetAutoWhiteBalance(false);
		//	pNuiCameraSettings->SetGamma(1.6);
		//	pNuiCameraSettings->SetContrast(0.5);
		//	pNuiCameraSettings->SetWhiteBalance(2700); //White balance is measured in temperature range, from 2700 to 6500 K
		//}

		return hr;
	}

	//-------------------------------------------------------------------
	/// Uninitializes the Kinect.
	void KinectStreamerBase::unInit( )
	{
		// Stop the Nui processing thread
		if ( NULL != m_hEvNuiProcessStop )
		{
			// Signal the thread
			SetEvent(m_hEvNuiProcessStop);

			// Wait for thread to stop
			if ( NULL != m_hThNuiProcess )
			{
				WaitForSingleObject( m_hThNuiProcess, INFINITE );
				CloseHandle( m_hThNuiProcess );
				std::cout<<"Handle Closed: m_hThNuiProcess"<<std::endl;
			}
			CloseHandle( m_hEvNuiProcessStop );
			std::cout<<"Handle Closed: m_hEvNuiProcessStop"<<std::endl;
		}

		if ( m_pNuiSensor )
		{
			m_pNuiSensor->NuiShutdown( );
			std::cout<<"Command executed: NuiShutdown"<<std::endl;
		}

		subsystemUnInit();

		if ( m_pNuiSensor )
		{
			m_pNuiSensor->Release();
			m_pNuiSensor = NULL;
			std::cout<<"Sensor Released"<<std::endl;
		}

	}


	//std::ostringstream KinectStreamerBase::printResult(HRESULT result){
	//	std::ostringstream st;
	//	st<<"Jajaja";
	//	return st;
	//}

	void KinectStreamerBase::setDeviceStatusChangeCallback(NuiStatusProc callback, void* pUserData){
		NuiSetDeviceStatusCallback(callback, pUserData);
	}

	OLECHAR* KinectStreamerBase::getDeviceConnectionID(){
		return m_pNuiSensor->NuiDeviceConnectionId();
	}

	///@warning Microsoft states unique id function has issues. Check SDK website for updates.
	OLECHAR* KinectStreamerBase::getDeviceUniqueID(){
		return m_pNuiSensor->NuiUniqueId();
	}

	void KinectStreamerBase::printConnectionID(){
		wprintf_s(L"Connection ID: %s\n", getDeviceConnectionID());
	}
	//void KinectStreamerBase::printResult()
	HRESULT KinectStreamerBase::getSensorCount(int& sensorCount){
		return NuiGetSensorCount(&sensorCount);
	}

	char* KinectStreamerBase::sensorStatusToString(HRESULT sensorStatus){
		switch (sensorStatus){
		case S_OK:
			return "Connected";
		case S_NUI_INITIALIZING:
			return "Initializing";
		case E_NUI_NOTCONNECTED:
			return "Disconnected";
		case E_NUI_NOTREADY:
			return "NotReady";
		case E_NUI_NOTPOWERED:
			return "NotPowered";
		case E_NUI_NOTGENUINE:
			return "NotGenuine";
		case E_NUI_NOTSUPPORTED:
			return "NotSupported";
		case E_NUI_INSUFFICIENTBANDWIDTH:
			return "InsufficientBandwidth";
		default:
			;
		}
		return "Error";
	}

	//-------------------------------------------------------------------
	///Zeroes out member variables.
	void KinectStreamerBase::Nui_Zero()
	{
		if (m_pNuiSensor)
		{
			///@todo Check why release causes acces exception.
			///Possible Answer: Release is being called on initialization, so previous "if" statement
			///might return a true value.
			//m_pNuiSensor->Release();
			m_pNuiSensor = NULL;
		}
		m_hThNuiProcess = NULL;
		m_hEvNuiProcessStop = NULL;
		//ZeroMemory(m_Pen,sizeof(m_Pen));
		//m_SkeletonDC = NULL;
		//m_SkeletonBMP = NULL;
		//m_SkeletonOldObj = NULL;
		//m_PensTotal = 6;
		//ZeroMemory(m_Points,sizeof(m_Points));
		
		//m_bScreenBlanked = false;

	}


	//-------------------------------------------------------------------
	///Reads the Kinect's true tilt angle.
	///@return The tilt angle of the Kinect.
	LONG KinectStreamerBase::getDeviceAngle(){
		m_pNuiSensor->NuiCameraElevationGetAngle(&m_deviceAngle);

		return m_deviceAngle;
	}

	//-------------------------------------------------------------------
	///Sets the Kinect angle.
	///@param[in] angle The desired angle.
	void KinectStreamerBase::setDeviceAngle(LONG angle){
		if(floor(getDeviceAngle()+0.5)!=angle){
			//if(getDeviceAngle()!=angle){
			//std::cout<<"Angle is being set";
			m_pNuiSensor->NuiCameraElevationSetAngle(angle);
		}
	}


	//-------------------------------------------------------------------
	///Defines the processing thread.
	///@param[in] pParam
	///@return The processing thread function that matches the initialization parameters.
	DWORD WINAPI KinectStreamerBase::Nui_ProcessThread(LPVOID pParam)
	{
		KinectStreamerBase* pthis = static_cast<KinectStreamerBase*>(pParam);
		return pthis->subsystemThreadFunction();
	}


}
