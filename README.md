# Camera Calibration #

### Introduction ###

This code repository aims at camera calibration by planar pattern. 

### Requirements ###

1. OS Platform: Windows
2. [OpenCV](http://opencv.org/): The version should be below 3.0, because the 2.X APIs are different from 3.X somehow. 
3. Before you can run the code project, you should configure the correct pathes of includes, libs and dlls for aforementioned third dependencies.

### Installation ###

1. Clone the CameraCalibration repository

	```
	git clone https://github.com/imbinwang/CameraCalibration.git
	```

2. We'll call the directory that you cloned CameraCalibration into CameraCalibration_ROOT. You should modify the configuration file `CameraCalibration_ROOT/data/calibration_settings.yml` and `CameraCalibration_ROOT/data/calibration_images_list.yml`. Please see the files for more information. 

3. After you prepare the images captured by the camera, call `CameraCalibration.exe YOUR_CALIBRATION_SETTINGS_PATH` to get the camera parameters in the output text file.


