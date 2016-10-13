#ifndef UTILS_H
#define UTILS_H

#include <QImage>

#include <opencv2/opencv.hpp>

class Utils {
public:
	static QString getExtension(const cv::String& str);

	static QImage mat2QImage(const cv::Mat& mat);
	static cv::Vec3b biLinearInterpolation(const cv::Mat& mat, float x, float y);
	static cv::Vec3f RGB2HSL(cv::Vec3b rgb);
	static cv::Vec3b HSL2RGB(cv::Vec3f hsl);
	static cv::Mat rotateImageMat(const cv::Mat& mat, float theta);
	static cv::Mat changeImageMatLightness(const cv::Mat& mat, float delta);
	static cv::Mat changeImageMatSaturation(const cv::Mat& mat, float delta);
	static cv::Mat changeImageMatHue(const cv::Mat& mat, float delta);

private:
	static void changePartialImageMatLightness(const cv::Mat& mat, cv::Mat& res, float delta, int x0, int x1, int y0, int y1);
	static void changePartialImageMatSaturation(const cv::Mat& mat, cv::Mat& res, float delta, int x0, int x1, int y0, int y1);
	static void changePartialImageMatHue(const cv::Mat& mat, cv::Mat& res, float delta, int x0, int x1, int y0, int y1);
};

#endif