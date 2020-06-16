#ifndef RHFW_PLATFORM_H_
#define RHFW_PLATFORM_H_

#define RHFW_PLATFORM_ANDROID

#define PLATFORM_BYTEORDER_HEADER <androidplatform/byteorder.h>

#define FILE_SEPARATOR "/"
#define FILE_SEPARATOR_CHAR '/'
namespace rhfw {
typedef char FilePathChar;
} // namespace rhfw

#define ASSETFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <androidplatform/asset/AndroidAssetDescriptor.h>
#define ASSETFILEDESCRIPTOR_EXACT_CLASS_TYPE AndroidAssetDescriptor
#define MUTEX_EXACT_CLASS_INCLUDE <androidplatform/threading/Mutex.h>
#define MUTEX_EXACT_CLASS_TYPE AndroidMutex
#define SEMAPHORE_EXACT_CLASS_INCLUDE <androidplatform/threading/Semaphore.h>
#define SEMAPHORE_EXACT_CLASS_TYPE AndroidSemaphore
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_INCLUDE <androidplatform/storage/AndroidStorageDirectoryDescriptor.h>
#define STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_TYPE AndroidStorageDirectoryDescriptor
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_INCLUDE <androidplatform/storage/AndroidStorageFileDescriptor.h>
#define STORAGEFILEDESCRIPTOR_EXACT_CLASS_TYPE AndroidStorageFileDescriptor
#define THREAD_EXACT_CLASS_INCLUDE <androidplatform/threading/Thread.h>
#define THREAD_EXACT_CLASS_TYPE AndroidThread
#define WINDOW_EXACT_CLASS_INCLUDE <androidplatform/AndroidWindow.h>
#define WINDOW_EXACT_CLASS_TYPE AndroidWindow

#define NETWORKADAPTER_EXACT_CLASS_INCLUDE <androidplatform/network/AndroidNetworkAdapter.h>
#define NETWORKADAPTER_EXACT_CLASS_TYPE AndroidNetworkAdapter

#define UDPIPV4SOCKET_EXACT_CLASS_INCLUDE <androidplatform/network/ipv4/udp/AndroidUDPIPv4Socket.h>
#define UDPIPV4SOCKET_EXACT_CLASS_TYPE AndroidUDPIPv4Socket

#define TCPIPV4SOCKET_EXACT_CLASS_INCLUDE <androidplatform/network/ipv4/tcp/AndroidTCPIPv4Socket.h>
#define TCPIPV4SOCKET_EXACT_CLASS_TYPE AndroidTCPIPv4Socket
#define TCPIPV4CONNECTION_EXACT_CLASS_INCLUDE <androidplatform/network/ipv4/tcp/AndroidTCPIPv4Connection.h>
#define TCPIPV4CONNECTION_EXACT_CLASS_TYPE AndroidTCPIPv4Connection

#define RANDOMCONTEXT_EXACT_CLASS_INCLUDE <androidplatform/random/AndroidRandomContext.h>
#define RANDOMCONTEXT_EXACT_CLASS_TYPE AndroidRandomContext

#define SENSORMANAGER_EXACT_CLASS_INCLUDE <androidplatform/sensor/AndroidSensorManager.h>
#define SENSORMANAGER_EXACT_CLASS_TYPE AndroidSensorManager
#define ATTITUDESENSOR_EXACT_CLASS_INCLUDE <androidplatform/sensor/AndroidAttitudeSensor.h>
#define ATTITUDESENSOR_EXACT_CLASS_TYPE AndroidAttitudeSensor
#define ACCELEROMETERSENSOR_EXACT_CLASS_INCLUDE <androidplatform/sensor/AndroidAccelerometerSensor.h>
#define ACCELEROMETERSENSOR_EXACT_CLASS_TYPE AndroidAccelerometerSensor

#define CAMERAMANAGER_EXACT_CLASS_INCLUDE <androidplatform/camera/AndroidCameraManager.h>
#define CAMERAMANAGER_EXACT_CLASS_TYPE AndroidCameraManager
#define CAMERA_EXACT_CLASS_INCLUDE <androidplatform/camera/AndroidCamera.h>
#define CAMERA_EXACT_CLASS_TYPE AndroidCamera
#define CAMERAINFO_EXACT_CLASS_INCLUDE <androidplatform/camera/AndroidCameraInfo.h>
#define CAMERAINFO_EXACT_CLASS_TYPE AndroidCameraInfo
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_INCLUDE <androidplatform/camera/AndroidCameraTextureInputSource.h>
#define CAMERATEXTUREINPUTSOURCE_EXACT_CLASS_TYPE AndroidCameraTextureInputSource

#endif /* RHFW_PLATFORM_H_ */
