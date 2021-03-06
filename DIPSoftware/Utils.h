#pragma once

#include <QImage>

#include <opencv2/opencv.hpp>

#include <array>
#include <cstdarg>
#include <functional>
#include <string>
#include <vector>

#define COLOR_RED		"\x1B[31m"
#define COLOR_GREEN		"\x1B[32m"
#define COLOR_YELLOW	"\x1B[33m"
#define COLOR_BLUE		"\x1B[34m"
#define COLOR_MAGENTA	"\x1B[35m"
#define COLOR_CYAN		"\x1B[36m"
#define COLOR_RESET		"\x1B[0m"

#define betw(x, y, z) ((x) >= (y) && (x) < (z))
#define rep(x, y) for (decltype(y) x{0}; x < (y); ++x)
#define repa(x, y, z) for (decltype(z) x{y}; x < (z); ++x)
#define repd(x, y, z) for (decltype(z) x{y}; x >= (z); --x)

#define touc(x) (uchar) round(x)

#define QSL(x) QStringLiteral(x)

template<typename T>
inline void updateMax(T& value, const T& max) {
	if (value < max) {
		value = max;
	}
}

template<typename T>
inline void updateMin(T& value, const T& min) {
	if (value > min) {
		value = min;
	}
}

template<typename T>
inline void updateMinMax(T& value, const T& min, const T& max) {
	if (value > min) {
		value = min;
	} else if (value < max) {
		value = max;
	}
}

template<typename T>
inline T sqr(const T &value) {
	return value * value;
}

template<typename T, int cn>
cv::Vec<T, cn> abs(const cv::Vec<T, cn> &v) {
	cv::Vec<T, cn> res(v);
	rep(i, cn) {
		res[i] = abs(res[i]);
	}
	return res;
}

class Utils {
public:
	using changeFuncType = std::function<void(const cv::Mat &, cv::Mat &, std::vector<float>)>;

	static std::string int2ANSIColor(int k);
	static void c_printf(const char *color, const char *format, ...);
	static void c_fprintf(const char *color, FILE *fp, const char *format, ...);

	static QString getExtension(const cv::String& str);

	static QImage mat2QImage(const cv::Mat& mat);
	static cv::Vec3b biLinearInterpolation(const cv::Mat& mat, float x, float y);
	static cv::Vec3f RGB2HSL(cv::Vec3b rgb);
	static cv::Vec3b HSL2RGB(cv::Vec3f hsl);

	static cv::Mat rotateImageMat(const cv::Mat& mat, float theta);
	static cv::Mat changeImageMat(const cv::Mat& mat, std::vector<float> delta, changeFuncType changeFunc);

	static std::array<int, 256> getHistogram(const cv::Mat& mat);
	static std::array<int, 256> getHistogram1Channel(const cv::Mat& mat, int channel);
	static std::array<int, 256> getHistogram3Channel(const cv::Mat& mat);
	static std::array<float, 256> getCDF(const std::array<int, 256>& hist, int pixels);

	static cv::Mat linearConvert(const cv::Mat& mat, const std::list<std::pair<float, float>> &vertices);
	static cv::Mat histogramEqualization(const cv::Mat& mat);
	static cv::Mat histogramSpecificationSML(const cv::Mat& orig, const cv::Mat& pattern);
	static cv::Mat histogramSpecificationGML(const cv::Mat& orig, const cv::Mat& pattern);

	static cv::Mat medianFilterImageMat(const cv::Mat& mat, int size);
	static cv::Mat gaussianFilterImageMat(const cv::Mat& mat, int size, float sigma);
	static cv::Mat sharpenImageMat(const cv::Mat& mat, int type);

	static void changePartialImageMatLightness(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas);
	static void changePartialImageMatSaturation(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas);
	static void changePartialImageMatHue(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas);
	static void changePartialImageMatGamma(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas);
	static void changePartialImageMatLog(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas);
	static void changePartialImageMatPow(const cv::Mat& mat, cv::Mat& res, std::vector<float> deltas);

	static cv::Mat freqFiltering(const cv::Mat &mat, const cv::Mat &filter);
	static cv::Mat lowPassFiltering(const cv::Mat &res, const cv::Mat &filter);
	static cv::Mat highPassFiltering(const cv::Mat &res, const cv::Mat &filter);

	static cv::Mat idealLowPassFilter(int rows, int cols, float D0);
	static cv::Mat butterWorthLowPassFilter(int rows, int cols, float D0, int n);
	static cv::Mat gaussLowPassFilter(int rows, int cols, float D0);
	static cv::Mat trapezoidLowPassFilter(int rows, int cols, float D0, float D_);
	static cv::Mat expLowPassFilter(int rows, int cols, float D0, int n);

	static cv::Mat idealHighPassFilter(int rows, int cols, float D0);
	static cv::Mat butterWorthHighPassFilter(int rows, int cols, float D0, int n);
	static cv::Mat gaussHighPassFilter(int rows, int cols, float D0);
	static cv::Mat laplaceHighPassFilter(int rows, int cols);

private:
	static std::vector<float> getGaussianKernel1D(int size, float sigma);

	static cv::Mat getRobertFilterImageMat(const cv::Mat& mat);
	static cv::Mat getPrewittFilterImageMat(const cv::Mat& mat);
	static cv::Mat getSobelFilterImageMat(const cv::Mat& mat);
	static cv::Mat getLaplaceFilterImageMat(const cv::Mat& mat);

	static void shiftDFT(cv::Mat &fImg);
};