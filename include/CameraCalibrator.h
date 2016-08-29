#ifndef _CameraCalibrator_H_
#define _CameraCalibrator_H_

#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class Settings
{
public:
	Settings() : goodInput(false) {}
	enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
	enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

	void write(FileStorage& fs) const                        //Write serialization for this class
	{
		fs << "{" << "BoardSize_Width"  << boardSize.width
			<< "BoardSize_Height" << boardSize.height
			<< "Square_Size"         << squareSize
			<< "Calibrate_Pattern" << patternToUse
			<< "Calibrate_NrOfFrameToUse" << nrFrames
			<< "Calibrate_FixAspectRatio" << aspectRatio
			<< "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
			<< "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

			<< "Write_DetectedFeaturePoints" << bwritePoints
			<< "Write_extrinsicParameters"   << bwriteExtrinsics
			<< "Write_outputFileName"  << outputFileName

			<< "Show_UndistortedImage" << showUndistorsed

			<< "Input_FlipAroundHorizontalAxis" << flipVertical
			<< "Input_Delay" << delay
			<< "Input" << input
			<< "}";
	}
	void read(const FileNode& node)                          //Read serialization for this class
	{
		node["BoardSize_Width" ] >> boardSize.width;
		node["BoardSize_Height"] >> boardSize.height;
		node["Calibrate_Pattern"] >> patternToUse;
		node["Square_Size"]  >> squareSize;
		node["Calibrate_NrOfFrameToUse"] >> nrFrames;
		node["Calibrate_FixAspectRatio"] >> aspectRatio;
		node["Write_DetectedFeaturePoints"] >> bwritePoints;
		node["Write_extrinsicParameters"] >> bwriteExtrinsics;
		node["Write_outputFileName"] >> outputFileName;
		node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
		node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
		node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
		node["Show_UndistortedImage"] >> showUndistorsed;
		node["Input"] >> input;
		node["Input_Delay"] >> delay;
		interprate();
	}
	void interprate()
	{
		goodInput = true;
		if (boardSize.width <= 0 || boardSize.height <= 0)
		{
			cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
			goodInput = false;
		}
		if (squareSize <= 10e-6)
		{
			cerr << "Invalid square size " << squareSize << endl;
			goodInput = false;
		}
		if (nrFrames <= 0)
		{
			cerr << "Invalid number of frames " << nrFrames << endl;
			goodInput = false;
		}

		if (input.empty())      // Check for valid input
			inputType = INVALID;
		else
		{
			if (input[0] >= '0' && input[0] <= '9')
			{
				stringstream ss(input);
				ss >> cameraID;
				inputType = CAMERA;
			}
			else
			{
				if (readStringList(input, imageList))
				{
					inputType = IMAGE_LIST;
					nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
				}
				else
					inputType = VIDEO_FILE;
			}
			if (inputType == CAMERA)
				inputCapture.open(cameraID);
			if (inputType == VIDEO_FILE)
				inputCapture.open(input);
			if (inputType != IMAGE_LIST && !inputCapture.isOpened())
				inputType = INVALID;
		}
		if (inputType == INVALID)
		{
			cerr << " Inexistent input: " << input;
			goodInput = false;
		}

		flag = 0;
		if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
		if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
		if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


		calibrationPattern = NOT_EXISTING;
		if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
		if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
		if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
		if (calibrationPattern == NOT_EXISTING)
		{
			cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
			goodInput = false;
		}
		atImageList = 0;

	}
	Mat nextImage()
	{
		Mat result;
		if( inputCapture.isOpened() )
		{
			Mat view0;
			inputCapture >> view0;
			view0.copyTo(result);
		}
		else if( atImageList < (int)imageList.size() )
			result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

		return result;
	}

	static bool readStringList( const string& filename, vector<string>& l )
	{
		l.clear();
		FileStorage fs(filename, FileStorage::READ);
		if( !fs.isOpened() )
			return false;
		FileNode n = fs.getFirstTopLevelNode();
		if( n.type() != FileNode::SEQ )
			return false;
		FileNodeIterator it = n.begin(), it_end = n.end();
		for( ; it != it_end; ++it )
			l.push_back((string)*it);
		return true;
	}
public:
	Size boardSize;            // The size of the board -> Number of items by width and height
	Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
	float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
	int nrFrames;              // The number of frames to use from the input for calibration
	float aspectRatio;         // The aspect ratio
	int delay;                 // In case of a video input
	bool bwritePoints;         //  Write detected feature points
	bool bwriteExtrinsics;     // Write extrinsic parameters
	bool calibZeroTangentDist; // Assume zero tangential distortion
	bool calibFixPrincipalPoint;// Fix the principal point at the center
	bool flipVertical;          // Flip the captured images around the horizontal axis
	string outputFileName;      // The name of the file where to write
	bool showUndistorsed;       // Show undistorted images after calibration
	string input;               // The input ->


	int cameraID;
	vector<string> imageList;
	int atImageList;
	VideoCapture inputCapture;
	InputType inputType;
	bool goodInput;
	int flag;

private:
	string patternToUse;
};

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
	if(node.empty())
		x = default_value;
	else
		x.read(node);
}

static void write(FileStorage& fs, const std::string&, const Settings& x)
{
	x.write(fs);
}

//============================================================================================================

class CameraCalibrator
{
public:
	CameraCalibrator(void);
	~CameraCalibrator(void);

	enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

	bool loadSettings(const string inputSettingsFile);
	bool saveSettings(const string outputSettingsFile);

	bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,
                           vector<vector<Point2f> > imagePoints );
	double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                         const vector<vector<Point2f> >& imagePoints,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const Mat& cameraMatrix , const Mat& distCoeffs,
                                         vector<float>& perViewErrors);
	void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                     Settings::Pattern patternType /*= Settings::CHESSBOARD*/);
	bool runCalibration( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                            vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                            vector<float>& reprojErrs,  double& totalAvgErr);
	void saveCameraParams( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                              const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                              const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                              double totalAvgErr );

	bool run(const string settingsFile);

public:
	Settings s;
};
#endif //_CameraCalibrator_H_
