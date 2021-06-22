#ifndef SIGN_DETECTOR_HPP
#define SIGN_DETECTOR_HPP

#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/utility.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <map>

struct typeSign {
	typeSign(int _id, std::string _n) :typeId(_id), name(_n) {}
	int gettypeId() const { return typeId; }
	std::string getName() const { return name; }
private:
	int typeId;
	std::string name;
};

//#define _PI 3.14159
struct settings
{
	int NumberGate = 1;
	int NumberQueues = 1;
	int MaxNumberQueues = 5;
	int MinPeopleQueues = 5;
	int CamFPS = 30;
	int NotMovingShiftPIX = 10;
	int64 MinutesDelay = 1;
	bool FindRibbonMaze = false;
	int minPeopleToFindRibbons = 2;
	bool ShowImageDebug = false;
	//
	int MinBoarderLight = 30;
	int MaxBoarderLight = 300;
	int MinSquareRibbon = 50;//old 50
	int MaxSquareRibbon = 300;//old 500
	int MinLengthRibbon = 25;//old 10
	int MinSummAngle = 1;
	int LimitRibbons = 2;
	int CntRightStructRibbons = 200;
	//
	int MinPartHeightFrame = 5;
	int ElongatedRibbonRatioLengthToHeight = 100;
	int AnglSeparateHorizontalAndVerticalLines = 5;
};

class SignDetector {
	settings m_settings;
	cv::Mat image, src_gray;
	cv::VideoCapture vc;

public:
	SignDetector();
	~SignDetector();
	void Run(std::string path);
	std::vector< cv::Rect> Run(cv::Mat& image, bool IsShowWindows, bool IsRecogSigns, std::map<cv::Rect, typeSign>& _mapSigns);
private:
	bool CircleDetection(const cv::Mat& src_gray, const cv::Mat& src_display, int cannyThreshold, int accumulatorThreshold, std::string name);
	cv::Mat filterByMask(settings& m_settings, cv::Mat1b& mask);
	void drawSquares(cv::Mat& _image, const std::vector<std::vector<cv::Point> >& squares, std::vector<cv::Vec4i>& _lines, std::string _color);
	cv::Mat drawSquaresBoth(const cv::Mat& image, const std::vector<std::vector<cv::Point> >& sqs, std::vector<cv::Vec4i>& _lines, std::string color);
	std::vector<std::vector<cv::Point>> getColorZones(settings& m_settings, cv::Scalar lowLev, cv::Scalar highLev, cv::Mat& image, cv::Mat hsv, std::string nameColor, bool IsShowDebugWindows, bool IsShowCircles, bool IsShowResult);
	std::vector<std::vector<cv::Point>> getColorZonesRed(settings& m_settings, cv::Scalar lowLev, cv::Scalar highLev, cv::Mat& image, cv::Mat& hsv, std::string nameColor, bool IsShowDebugWindows, bool IsShowCircles, bool IsShowResult);
	void IntersectionColorZones(cv::Mat& img, std::vector<std::vector<cv::Point>>& z1, std::vector<std::vector<cv::Point>>& z2, std::string nameBlob, cv::Scalar color);
	std::vector<cv::Rect> IntersectionColorZones(cv::Mat& img, std::vector<std::vector<cv::Point>>& z1, std::vector<std::vector<cv::Point>>& z2, std::string nameBlob, cv::Scalar color, bool IsShow);
	void RecogSigns(cv::Mat image, std::vector<cv::Rect> zones1, std::map<cv::Rect, typeSign>& mapSigns);
};
	
#endif //SIGN_DETECTOR_HPP
