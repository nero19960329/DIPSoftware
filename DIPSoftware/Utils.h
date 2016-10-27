#ifndef UTILS_H
#define UTILS_H

#include <QImage>

#include <opencv2/opencv.hpp>

#include <array>
#include <functional>
#include <vector>

#define betw(x, y, z) ((x) > (y) && (x) < (z))
#define rep(x, y) for (decltype(y) x{0}; x < (y); ++x)
#define repa(x, y, z) for (decltype(z) x{y}; x < (z); ++x)
#define repd(x, y, z) for (decltype(z) x{y}; x >= (z); --x)

template<typename T>
inline void updateMax(T& value, const T& max) {
	if (value < max) {
		value = max;
	}
}

class Utils {
public:
	using changeFuncType = std::function<void(const cv::Mat &, cv::Mat &, std::vector<float>, int, int, int, int)>;

	static QString getExtension(const cv::String& str);
	static void getSequence(const cv::Vec3f& vec, float &max, float &mid, float &min);

	static cv::Mat mat8U2Mat32F(const cv::Mat& mat8U);
	static cv::Mat mat32F2Mat8U(const cv::Mat& mat32F);
	static QImage mat2QImage(const cv::Mat& mat);
	static cv::Vec3f biLinearInterpolation(const cv::Mat& mat, float x, float y);
	static cv::Vec3f RGB2HSL(cv::Vec3f rgb);
	static cv::Vec3f HSL2RGB(cv::Vec3f hsl);
	static cv::Mat rotateImageMat(const cv::Mat& mat, float theta);
	static cv::Mat changeImageMat(const cv::Mat& mat, std::vector<float> delta, changeFuncType changeFunc);

	static std::array<int, 256> getHistogram(const cv::Mat& mat);
	static cv::Mat histogramEqualization(const cv::Mat& mat);
	static cv::Mat histogramSpecificationSML(const cv::Mat& orig, const cv::Mat& pattern);

	static void changePartialImageMatLightness(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas, int x0, int x1, int y0, int y1);
	static void changePartialImageMatSaturation(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas, int x0, int x1, int y0, int y1);
	static void changePartialImageMatHue(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas, int x0, int x1, int y0, int y1);
	static void changePartialImageMatGamma(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas, int x0, int x1, int y0, int y1);
	static void changePartialImageMatLog(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas, int x0, int x1, int y0, int y1);
	static void changePartialImageMatPow(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas, int x0, int x1, int y0, int y1);
};

#endif