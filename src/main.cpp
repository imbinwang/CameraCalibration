#include "../include/CameraCalibrator.h"

int main(int argc,char *argv[])
{
	CameraCalibrator cameraCalib;
	std::string settingFileName;

	if(argc<2)
	{
		//default setting file
		settingFileName = "calibration_settings.yml";
	}
	else
	{
		settingFileName = std::string(argv[1]);
	}

	cameraCalib.run(settingFileName);

	return 0;
}