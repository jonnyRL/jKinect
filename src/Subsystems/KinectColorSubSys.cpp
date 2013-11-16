/**
\class jrl::KinectColorSubSys

A class that includes functions to access the Kinect's color subsystem.

*/

#include "KinectColorSubsys.h"
#include <iostream>

namespace jrl
{
	///The default constructor.
	KinectColorSubSys::KinectColorSubSys(){
		setImageType(KINECT_IMAGE_TYPE_COLOR);
		setResolution(KINECT_COLOR_RESOLUTION_640x480);
		streamerInit();
	}

	///The default destructor.
	KinectColorSubSys::~KinectColorSubSys()
	{
		streamerUnInit();
	}

	///Sets the image type that will be used by the color subsystem.
	///@warning Only "KINECT_IMAGE_TYPE_COLOR" is working.
	///@todo Integrate all the color image types to the color system.
	///@param[in] type The selected image type.
	void KinectColorSubSys::setImageType(KINECT_IMAGE_TYPE type){
		switch(type){
		case KINECT_IMAGE_TYPE_COLOR:
			//Data:X8R8G8B8   Format:32-bit   Resolution:1280x960, 640x480
			colorImageType = NUI_IMAGE_TYPE_COLOR;
			bytesPerPixel = 4;
			break;
		case KINECT_IMAGE_TYPE_COLOR_INFRARED:
			//Data:10-MSB   Format:16-bit   Resolution:640x480
			colorImageType = NUI_IMAGE_TYPE_COLOR_INFRARED;
			bytesPerPixel = 2;
			break;
		case KINECT_IMAGE_TYPE_COLOR_RAW_BAYER:
			//Data:X8R8G8B8   Format:32-bit   Resolution:1280x960, 640x480
			colorImageType = NUI_IMAGE_TYPE_COLOR_RAW_BAYER;
			bytesPerPixel = 4;
			break;
		case KINECT_IMAGE_TYPE_COLOR_RAW_YUV:
			//Data:UYVY Format:16-bit Resolution:640x480   FPS:15 
			colorImageType = NUI_IMAGE_TYPE_COLOR_RAW_YUV;
			bytesPerPixel = 2;
			break;
		case KINECT_IMAGE_TYPE_COLOR_YUV:
			//Data:UYVY Format:16-bit Resolution:640x480
			colorImageType = NUI_IMAGE_TYPE_COLOR_YUV;
			bytesPerPixel = 2;
			break;
		default:
			colorImageType = NUI_IMAGE_TYPE_COLOR;
			bytesPerPixel = 4;
			//Error Invalid image type selected
			break;
		}
		
	}

	///Sets the resolution of the color image to be used.
	///@note All resolutions are not valid for all image types.
	///@todo Make resolution validation for selected image type.
	///@param[in] res The selected resolution.
	void KinectColorSubSys::setResolution(KINECT_COLOR_RESOLUTION res){
		switch(res){
		case KINECT_COLOR_RESOLUTION_640x480:
			colorResolution = NUI_IMAGE_RESOLUTION_640x480;
			frameWidth = 640;
			frameHeight = 480;
			break;
		case KINECT_COLOR_RESOLUTION_1280x960:
			colorResolution = NUI_IMAGE_RESOLUTION_1280x960;
			frameWidth = 1280;
			frameHeight = 960;
			break;
		default:
			colorResolution = NUI_IMAGE_RESOLUTION_INVALID;
			break;
		}
		widthByHeight = frameWidth * frameHeight;
		totalColorBytes = widthByHeight * bytesPerPixel;
		m_pLockedColorBuffer = new unsigned char[totalColorBytes];
	}

	///Initializes the color subsystem.
	///@param[in] A pointer to the sensor.
	///@return The status of the operation. A value greater than zero if "SUCCEEDED" other value if "FAILED".
	HRESULT KinectColorSubSys::subsystemInit(INuiSensor* kSensor)
	{
		//sensor=kSensor;
		////Color Stream
		//HRESULT hr;

		////if(m_colorStreamEnabled){
		//m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );//Init colorFrameEvent

		//hr = sensor->NuiImageStreamOpen( //Open Image Stream
		//	NUI_IMAGE_TYPE_COLOR,
		//	NUI_IMAGE_RESOLUTION_640x480,
		//	0,
		//	2,
		//	m_hNextColorFrameEvent,
		//	&m_pVideoStreamHandle );
		//	//++m_activeNumOfStreams;
		////}
		//return hr;

		sensor=kSensor;
		//Color Stream
		HRESULT hr;

		//if(m_colorStreamEnabled){
		m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );//Init colorFrameEvent

		hr = sensor->NuiImageStreamOpen( //Open Image Stream
			colorImageType,
			colorResolution,
			0,
			2,
			m_hNextColorFrameEvent,
			&m_pVideoStreamHandle );
			//++m_activeNumOfStreams;
		//}
		return hr;
	}

	///Frees the resources used by the subsystem.
	void KinectColorSubSys::subsystemUnInit()
	{
		delete[] m_pLockedColorBuffer;
		if ( m_hNextColorFrameEvent && ( m_hNextColorFrameEvent != INVALID_HANDLE_VALUE ) )
		{
			CloseHandle( m_hNextColorFrameEvent );
			m_hNextColorFrameEvent = NULL;
		}
	}

	//-------------------------------------------------------------------
	///Handles incoming color data.
	void KinectColorSubSys::gotColorAlert()
	{
		
		NUI_IMAGE_FRAME imageFrame;
		HRESULT hr = sensor->NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &imageFrame );

		if ( FAILED( hr ) )
		{
			return;
		}

		INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
		NUI_LOCKED_RECT LockedRect;
		pTexture->LockRect( 0, &LockedRect, NULL, 0 );
		if ( LockedRect.Pitch != 0 )
		{	
			unsigned char* targetBuffer = m_pLockedColorBuffer; //Watch for race condition at this point
			//byte* pBufferRun = cRes.LockedRect.pBits;
			byte* pBufferRun = LockedRect.pBits;
			byte* pBufferEnd = pBufferRun + totalColorBytes;
			--targetBuffer;
			while(pBufferRun<pBufferEnd){
				
				*(++targetBuffer) = *(pBufferRun+2);
				*(++targetBuffer) = *(pBufferRun+1);
				*(++targetBuffer) = *(pBufferRun);
				*(++targetBuffer)|=255;
				pBufferRun+=4;
			}
			//std::cout<<"Sending Image";
		}
		else
		{
			OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
		}
		pTexture->UnlockRect( 0 );
		sensor->NuiImageStreamReleaseFrame( m_pVideoStreamHandle, &imageFrame );
	}


	///Sets the resolution of the depth field that will be used when performing depth space to color space transformations.
	///@param[in] The selected resolution.
	void KinectColorSubSys::setDepthResolutionForSpaceTransformation(NUI_IMAGE_RESOLUTION res){
		depthResolution = res;
	}

	///Maps a coordinate from color space to depth space given a set of depth coordinates.
	///@todo Check if function can be parallelized.
	///@param[in] depthX The depth pixel x coordinate. 
	///@param[in] depthY The depth pixel y coordinate.
	///@param[in] depthVal The depth value 'z' of coordinates x, y.
	///@param[out] xTarget The placeholder for the mapped x coordinate.
	///@param[out] yTarget The placeholder for the mapped y coordinate.
	///@return The status of the operation. A value greater than zero if "SUCCEEDED" other value if "FAILED".
	HRESULT KinectColorSubSys::colorSpaceToDepthSpace(LONG depthX, LONG depthY, USHORT depthVal, LONG *xTarget, LONG *yTarget)
	{
		return sensor->NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
				colorResolution,
				depthResolution,
				NULL,
				depthX,
				depthY,
				depthVal,
				xTarget,
				yTarget
				);
	}


	
	///Maps an index from color space to depth space given a set of depth coordinates.
	///@param[in] depthX The x pixel depth coordinate. 
	///@param[in] depthY The y pixel depth coordinate.
	///@param[in] depthVal The z depth value of coordinates x, y.
	///@param[out] colorSetIndex The variable where the index corresponding to the first value of the RBGA mapped values will be stored. 
	///@return The status of the operation. A value greater than zero if "SUCCEEDED" other value if "FAILED".
	HRESULT KinectColorSubSys::colorSpaceToDepthSpaceIndex(LONG depthX, LONG depthY, USHORT depthVal, LONG* colorSetIndex)
	{	
		LONG xTarget, yTarget;
		HRESULT hr = colorSpaceToDepthSpace(depthX,depthY,depthVal, &xTarget, &yTarget);
		*colorSetIndex =  4*(getFrameWidth()*yTarget + xTarget);
		return hr;
	}

	///Returns a pointer of the color space pixel corresponding to the pixel in depth space given a set of depth coordinates.
	///@param[in] depthX The x pixel depth coordinate. 
	///@param[in] depthY The y pixel depth coordinate.
	///@param[in] depthVal The z depth value of coordinates x, y.
	///@return A pointer to a color pixel.
	unsigned char* KinectColorSubSys::colorSpacePixelToDepthSpace(LONG depthX, LONG depthY, USHORT depthVal)
	{	
		LONG xTarget, yTarget;
		HRESULT hr = colorSpaceToDepthSpace(depthX,depthY,depthVal, &xTarget, &yTarget);
		return getColorBuffer() + 4*(getFrameWidth()*yTarget + xTarget);
	}

	///Returns a pointer of the input pixel corresponding to the pixel in depth space given a set of depth coordinates.
	///@param[in] src A pointer to the input image to be mapped.
	///@param[in] depthX The x pixel depth coordinate. 
	///@param[in] depthY The y pixel depth coordinate.
	///@param[in] depthVal The z depth value of coordinates x, y.
	///@return A pointer to a pixel in the input image.
	unsigned char* KinectColorSubSys::colorSpacePixelToDepthSpace(unsigned char* src, LONG depthX, LONG depthY, USHORT depthVal)
	{	
		LONG xTarget, yTarget;
		HRESULT hr = colorSpaceToDepthSpace(depthX,depthY,depthVal, &xTarget, &yTarget);
		return src + 4*(getFrameWidth()*yTarget + xTarget);
	}

	///Initializes the class variables.
	void KinectColorSubSys::streamerInit()
	{
		//Color Vars
		//m_pFreeColorBuffer = &m_colorBuffer[0][0];
		//m_pLockedColorBuffer = &m_colorBuffer[1][0];
		m_newColorDataAvailable = false;

		////Set exposure values
		//INuiColorCameraSettings* pNuiCameraSettings = nullptr;
		//HRESULT cameraInterfaceObtained = sensor->NuiGetColorCameraSettings(&pNuiCameraSettings);

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
	}

	///Clears the class variables.
	void KinectColorSubSys::streamerUnInit()
	{
		m_hNextColorFrameEvent = NULL;
		m_pVideoStreamHandle = NULL;
	}


	//-------------------------------------------------------------------
	///Reads the "newColorData" flag.
	///@note The flag is reset to false each time it is read so it works as a one-shot trigger.
	///@return True for new data, false if flag has been read before new data arrives.
	bool KinectColorSubSys::newColorData(){
		bool available = m_newColorDataAvailable;
		m_newColorDataAvailable = false;
		return available;
	}

}