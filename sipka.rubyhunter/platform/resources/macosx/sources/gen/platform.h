#ifndef RHFW_PLATFORM_H_
#define RHFW_PLATFORM_H_

#define RHFW_PLATFORM_MACOSX

#define OPENAL_HEADER_FILE <OpenAL/OpenAL.h>

#define PLATFORM_BYTEORDER_HEADER <appleplatform/byteorder.h>

#define FILE_SEPARATOR "/"
#define FILE_SEPARATOR_CHAR '/'
namespace rhfw {
typedef char FilePathChar;
} // namespace rhfw

#define ASSETFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <appleplatform/asset/AppleAssetDescriptor.h>
#define ASSETFILEDESCRIPTOR_EXACT_CLASS_TYPE AppleAssetDescriptor
#define MUTEX_EXACT_CLASS_INCLUDE <appleplatform/threading/Mutex.h>
#define MUTEX_EXACT_CLASS_TYPE AppleMutex
#define SEMAPHORE_EXACT_CLASS_INCLUDE <appleplatform/threading/Semaphore.h>
#define SEMAPHORE_EXACT_CLASS_TYPE AppleSemaphore
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_INCLUDE <appleplatform/storage/AppleStorageDirectoryDescriptor.h>
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_TYPE AppleStorageDirectoryDescriptor
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <appleplatform/storage/AppleStorageFileDescriptor.h>
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_TYPE AppleStorageFileDescriptor
#define THREAD_EXACT_CLASS_INCLUDE <appleplatform/threading/Thread.h>
#define THREAD_EXACT_CLASS_TYPE AppleThread
#define WINDOW_EXACT_CLASS_INCLUDE <macosxplatform/MacOsxWindow.h>
#define WINDOW_EXACT_CLASS_TYPE MacOsxWindow

#define NETWORKADAPTER_EXACT_CLASS_INCLUDE <appleplatform/network/AppleNetworkAdapter.h>
#define NETWORKADAPTER_EXACT_CLASS_TYPE AppleNetworkAdapter

#define UDPIPV4SOCKET_EXACT_CLASS_INCLUDE <appleplatform/network/ipv4/udp/AppleUDPIPv4Socket.h>
#define UDPIPV4SOCKET_EXACT_CLASS_TYPE AppleUDPIPv4Socket

#define TCPIPV4SOCKET_EXACT_CLASS_INCLUDE <appleplatform/network/ipv4/tcp/AppleTCPIPv4Socket.h>
#define TCPIPV4SOCKET_EXACT_CLASS_TYPE AppleTCPIPv4Socket
#define TCPIPV4CONNECTION_EXACT_CLASS_INCLUDE <appleplatform/network/ipv4/tcp/AppleTCPIPv4Connection.h>
#define TCPIPV4CONNECTION_EXACT_CLASS_TYPE AppleTCPIPv4Connection

#define RANDOMCONTEXT_EXACT_CLASS_INCLUDE <macosxplatform/random/MacOsxRandomContext.h>
#define RANDOMCONTEXT_EXACT_CLASS_TYPE MacOsxRandomContext

#define SENSORMANAGER_EXACT_CLASS_INCLUDE <macosxplatform/sensor/MacOsxSensorManager.h>
#define SENSORMANAGER_EXACT_CLASS_TYPE MacOsxSensorManager
#define ATTITUDESENSOR_EXACT_CLASS_INCLUDE <macosxplatform/sensor/MacOsxAttitudeSensor.h>
#define ATTITUDESENSOR_EXACT_CLASS_TYPE MacOsxAttitudeSensor
#define ACCELEROMETERSENSOR_EXACT_CLASS_INCLUDE <macosxplatform/sensor/MacOsxAccelerometerSensor.h>
#define ACCELEROMETERSENSOR_EXACT_CLASS_TYPE MacOsxAccelerometerSensor

#define CAMERAMANAGER_EXACT_CLASS_INCLUDE <macosxplatform/camera/MacOsxCameraManager.h>
#define CAMERAMANAGER_EXACT_CLASS_TYPE MacOsxCameraManager
#define CAMERA_EXACT_CLASS_INCLUDE <macosxplatform/camera/MacOsxCamera.h>
#define CAMERA_EXACT_CLASS_TYPE MacOsxCamera
#define CAMERAINFO_EXACT_CLASS_INCLUDE <macosxplatform/camera/MacOsxCameraInfo.h>
#define CAMERAINFO_EXACT_CLASS_TYPE MacOsxCameraInfo
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE <macosxplatform/camera/MacOsxCameraTextureInputSource.h>
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE MacOsxCameraTextureInputSource

#endif /* RHFW_PLATFORM_H_ */
