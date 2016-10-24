#ifndef UTILS_H
#define UTILS_H

#include <QImage>

#include <opencv2/opencv.hpp>

#define betw(x, y, z) ((x) > (y) && (x) < (z))
#define rep(x, y) for (decltype(y) x{0}; x < (y); ++x)
#define repa(x, y, z) for (decltype(z) x{y}; x < (z); ++x)
#define repd(x, y, z) for (decltype(z) x{y}; x >= (z); --x)

class Utils {
public:
	static QString getExtension(const cv::String& str);

	static cv::Mat mat8U2Mat32F(const cv::Mat& mat8U);
	static cv::Mat mat32F2Mat8U(const cv::Mat& mat32F);
	static QImage mat2QImage(const cv::Mat& mat);
	static cv::Vec3f biLinearInterpolation(const cv::Mat& mat, float x, float y);
	static cv::Vec3f RGB2HSL(cv::Vec3f rgb);
	static cv::Vec3f HSL2RGB(cv::Vec3f hsl);
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