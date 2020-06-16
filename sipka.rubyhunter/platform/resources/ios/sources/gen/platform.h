#ifndef RHFW_PLATFORM_H_
#define RHFW_PLATFORM_H_

#define RHFW_PLATFORM_IOS

#define OPENAL_HEADER_FILE <OpenAL/OpenAL.h>
#define OPENGLES20_HEADER_FILE <OpenGLES/ES2/gl.h>

#define PLATFORM_BYTEORDER_HEADER <appleplatform/byteorder.h>

#define RHFW_HARDWARE_KEYBOARD_EVENTS_UNSUPPORTED

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
#define WINDOW_EXACT_CLASS_INCLUDE <iosplatform/IosWindow.h>
#define WINDOW_EXACT_CLASS_TYPE IosWindow

#define NETWORKADAPTER_EXACT_CLASS_INCLUDE <appleplatform/network/AppleNetworkAdapter.h>
#define NETWORKADAPTER_EXACT_CLASS_TYPE AppleNetworkAdapter

#define UDPIPV4SOCKET_EXACT_CLASS_INCLUDE <appleplatform/network/ipv4/udp/AppleUDPIPv4Socket.h>
#define UDPIPV4SOCKET_EXACT_CLASS_TYPE AppleUDPIPv4Socket

#define TCPIPV4SOCKET_EXACT_CLASS_INCLUDE <appleplatform/network/ipv4/tcp/AppleTCPIPv4Socket.h>
#define TCPIPV4SOCKET_EXACT_CLASS_TYPE AppleTCPIPv4Socket
#define TCPIPV4CONNECTION_EXACT_CLASS_INCLUDE <appleplatform/network/ipv4/tcp/AppleTCPIPv4Connection.h>
#define TCPIPV4CONNECTION_EXACT_CLASS_TYPE AppleTCPIPv4Connection

#define RANDOMCONTEXT_EXACT_CLASS_INCLUDE <iosplatform/random/IosRandomContext.h>
#define RANDOMCONTEXT_EXACT_CLASS_TYPE IosRandomContext

#define SENSORMANAGER_EXACT_CLASS_INCLUDE <iosplatform/sensor/IosSensorManager.h>
#define SENSORMANAGER_EXACT_CLASS_TYPE IosSensorManager
#define ATTITUDESENSOR_EXACT_CLASS_INCLUDE <iosplatform/sensor/IosAttitudeSensor.h>
#define ATTITUDESENSOR_EXACT_CLASS_TYPE IosAttitudeSensor
#define ACCELEROMETERSENSOR_EXACT_CLASS_INCLUDE <iosplatform/sensor/IosAccelerometerSensor.h>
#define ACCELEROMETERSENSOR_EXACT_CLASS_TYPE IosAccelerometerSensor

#define CAMERAMANAGER_EXACT_CLASS_INCLUDE <iosplatform/camera/IosCameraManager.h>
#define CAMERAMANAGER_EXACT_CLASS_TYPE IosCameraManager
#define CAMERA_EXACT_CLASS_INCLUDE <iosplatform/camera/IosCamera.h>
#define CAMERA_EXACT_CLASS_TYPE IosCamera
#define CAMERAINFO_EXACT_CLASS_INCLUDE <iosplatform/camera/IosCameraInfo.h>
#define CAMERAINFO_EXACT_CLASS_TYPE IosCameraInfo
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE <iosplatform/camera/IosCameraTextureInputSource.h>
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE IosCameraTextureInputSource

#endif /* RHFW_PLATFORM_H_ */
