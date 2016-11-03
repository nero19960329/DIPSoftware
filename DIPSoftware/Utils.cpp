#include "Utils.h"
#include "DebugUtils.h"

#include <future>
#include <numeric>

#include <QDebug>

#define PI 3.141592653589793
#define EPSILON 1e-3

using namespace cv;
using namespace std;

string Utils::int2ANSIColor(int k) {
	ostringstream oss;
	oss << "\x1B[3" << (k + 2) << "m";
	return oss.str();
}

void Utils::c_printf(const char *color, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	printf(color);
	vprintf(format, ap);
	printf(COLOR_RESET);
	va_end(ap);
}

void Utils::c_fprintf(const char *color, FILE *fp, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	fprintf(fp, color);
	vfprintf(fp, format, ap);
	fprintf(fp, COLOR_RESET);
	va_end(ap);
}

QString Utils::getExtension(const String& str) {
	auto pos = str.find_last_of('.');
	QString res;
	repa(i, pos + 1, str.size()) {
		res.append(str[i]);
	}
	return res;
}

QImage Utils::mat2QImage(const Mat& mat) {
	if (mat.type() == CV_8UC3) {
		const uchar *pSrc = (const uchar*) mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	} else {
		return QImage();
	}
}

Vec3b Utils::biLinearInterpolation(const Mat& mat, float x, float y) {
	int x1, x2, y1, y2;
	x1 = (int) x;
	y1 = (int) y;
	x2 = x1 + 1;
	y2 = y1 + 1;

	float r, g, b;

	if (x < mat.rows - 1 && y < mat.cols - 1) {
		r = mat.at<Vec3b>(x1, y1)[0] * (x2 - x) * (y2 - y) +
			mat.at<Vec3b>(x2, y1)[0] * (x - x1) * (y2 - y) +
			mat.at<Vec3b>(x1, y2)[0] * (x2 - x) * (y - y1) +
			mat.at<Vec3b>(x2, y2)[0] * (x - x1) * (y - y1);
		g = mat.at<Vec3b>(x1, y1)[1] * (x2 - x) * (y2 - y) +
			mat.at<Vec3b>(x2, y1)[1] * (x - x1) * (y2 - y) +
			mat.at<Vec3b>(x1, y2)[1] * (x2 - x) * (y - y1) +
			mat.at<Vec3b>(x2, y2)[1] * (x - x1) * (y - y1);
		b = mat.at<Vec3b>(x1, y1)[2] * (x2 - x) * (y2 - y) +
			mat.at<Vec3b>(x2, y1)[2] * (x - x1) * (y2 - y) +
			mat.at<Vec3b>(x1, y2)[2] * (x2 - x) * (y - y1) +
			mat.at<Vec3b>(x2, y2)[2] * (x - x1) * (y - y1);
	} else if (x == mat.rows - 1 && y < mat.cols - 1) {
		r = mat.at<Vec3b>(x1, y1)[0] * (y2 - y) + mat.at<Vec3b>(x1, y2)[0] * (y - y1);
		g = mat.at<Vec3b>(x1, y1)[1] * (y2 - y) + mat.at<Vec3b>(x1, y2)[1] * (y - y1);
		b = mat.at<Vec3b>(x1, y1)[2] * (y2 - y) + mat.at<Vec3b>(x1, y2)[2] * (y - y1);
	} else if (x < mat.rows - 1 && y == mat.cols - 1) {
		r = mat.at<Vec3b>(x1, y1)[0] * (x2 - x) + mat.at<Vec3b>(x2, y1)[0] * (x - x1);
		g = mat.at<Vec3b>(x1, y1)[1] * (x2 - x) + mat.at<Vec3b>(x2, y1)[1] * (x - x1);
		b = mat.at<Vec3b>(x1, y1)[2] * (x2 - x) + mat.at<Vec3b>(x2, y1)[2] * (x - x1);
	} else {
		r = mat.at<Vec3b>(x1, y1)[0];
		g = mat.at<Vec3b>(x1, y1)[1];
		b = mat.at<Vec3b>(x1, y1)[2];
	}

	return { touc(r), touc(g), touc(b) };
}

Vec3f Utils::RGB2HSL(Vec3b rgb) {
	float h, s, l;
	float r, g, b;
	r = rgb[2] * 1.0 / 255;
	g = rgb[1] * 1.0 / 255;
	b = rgb[0] * 1.0 / 255;

	float maxValue, minValue;
	minValue = min(r, min(g, b));

	if (r == g && r == b) {
		h = 0.0;
		s = 0.0;
		l = r;
		return { h, s, l };
	} else if (r >= g && r >= b) {
		maxValue = r;
		h = 60 * (g - b) / (maxValue - minValue);
		if (g < b) {
			h += 360.0;
		}
	} else if (g >= r && g >= b) {
		maxValue = g;
		h = 60 * (b - r) / (maxValue - minValue) + 120.0;
	} else {
		maxValue = b;
		h = 60 * (r - g) / (maxValue - minValue) + 240.0;
	}

	l = (maxValue + minValue) / 2;
	if (l < 0.5) {
		s = (maxValue - minValue) / (maxValue + minValue);
	} else {
		s = (maxValue - minValue) / (2.0 - maxValue - minValue);
	}

	return { h, s, l };
}

Vec3b Utils::HSL2RGB(Vec3f hsl) {
	float h, s, l;
	h = hsl[0];
	s = hsl[1];
	l = hsl[2];

	if (s < 1e-3) {
		return { touc(l), touc(l), touc(l) };
	}

	float rgb[3];
	float p, q;
	if (l < 0.5) {
		q = l * (1 + s);
	} else {
		q = l + s - (l * s);
	}
	p = 2 * l - q;

	rgb[1] = h / 360.0;
	rgb[0] = rgb[1] + 1.0 / 3;
	rgb[2] = rgb[1] - 1.0 / 3;

	for (int i = 0; i < 3; ++i) {
		if (rgb[i] < 0) {
			rgb[i] += 1.0;
		} else if (rgb[i] >= 1.0) {
			rgb[i] -= 1.0;
		}

		if (rgb[i] < 1.0 / 6) {
			rgb[i] = p + ((q - p) * 6 * rgb[i]);
		} else if (rgb[i] < 0.5) {
			rgb[i] = q;
		} else if (rgb[i] < 2.0 / 3) {
			rgb[i] = p + ((q - p) * 6 * (2.0 / 3 - rgb[i]));
		} else {
			rgb[i] = p;
		}
	}

	return { touc(rgb[2] * 255), touc(rgb[1] * 255), touc(rgb[0] * 255) };
}

Mat Utils::rotateImageMat(const Mat& mat, float theta) {
	int newW, newH;
	int w, h;
	float cx, cy, dx, dy;
	float cosTheta = cos(theta), sinTheta = sin(theta);

	w = mat.rows;
	h = mat.cols;
	newW = ceil(w * abs(cosTheta) + h * abs(sinTheta) - EPSILON);
	newH = ceil(w * abs(sinTheta) + h * abs(cosTheta) - EPSILON);
	cx = (w - 1) * 1.0 / 2;
	cy = (h - 1) * 1.0 / 2;
	dx = (newW - w) * 1.0 / 2;
	dy = (newH - h) * 1.0 / 2;

	Mat res(newW, newH, CV_8UC3);
	rep(i, res.rows) rep(j, res.cols) {
		float x, y;
		x = cosTheta * (i - cx - dx) - sinTheta * (j - cy - dy) + cx;
		y = sinTheta * (i - cx - dx) + cosTheta * (j - cy - dy) + cy;
		if (betw(x, -EPSILON, w + EPSILON) && betw(y, -EPSILON, h + EPSILON)) {
			res.at<Vec3b>(i, j) = biLinearInterpolation(mat, x, y);
		} else {
			res.at<Vec3b>(i, j) = { 255, 255, 255 };
		}
	}

	return res;
}

Mat Utils::changeImageMat(const Mat& mat, vector<float> deltas, changeFuncType changeFunc) {
	Mat res(mat.rows, mat.cols, CV_8UC3);

	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	rep(i, 4) rep(j, 2) {
		getPartialResult[i + i + j] = async(changeFunc, mat, res, deltas, x[i], x[i + 1], y[j], y[j + 1]);
	}

	rep(i, 8) {
		getPartialResult[i].get();
	}

	return res;
}

array<int, 256> Utils::getHistogram(const Mat& mat) {
	array<int, 256> res;
	fill(res.begin(), res.end(), 0);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		uchar grey = touc((rgb[0] + rgb[1] + rgb[2]) * 1.0 / 3);
		++res[grey];
	}
	return res;
}

array<int, 256> Utils::getHistogram1Channel(const Mat& mat, int channel) {
	array<int, 256> res;
	fill(res.begin(), res.end(), 0);
	rep(i, mat.rows) rep(j, mat.cols) {
		++res[mat.at<Vec3b>(i, j)[channel]];
	}
	return res;
}

array<int, 256> Utils::getHistogram3Channel(const Mat& mat) {
	array<int, 256> res;
	fill(res.begin(), res.end(), 0);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		++res[rgb[0]];
		++res[rgb[1]];
		++res[rgb[2]];
	}
	return res;
}

array<float, 256> Utils::getCDF(const array<int, 256>& hist, int pixels) {
	array<float, 256> res;
	array<int, 256> tmp;
	tmp[0] = hist[0];
	repa(i, 1, 256) {
		tmp[i] = tmp[i - 1] + hist[i];
		res[i] = tmp[i] * 1.0 / pixels;
	}
	return res;
}

Mat Utils::linearConvert(const Mat& mat, const list<pair<float, float>>& vertices) {
	auto cvt = [=](float d){ return (int) (d * 255 + 0.5); };

	array<uchar, 256> map;
	auto it = vertices.begin();
	auto nextIt = vertices.begin();
	++nextIt;
	rep(i, 256) {
		if (i > nextIt->first) {
			++it;
			++nextIt;
		}
		map[i] = touc(((nextIt->second - it->second) * i + (it->second * nextIt->first - it->first * nextIt->second)) * 1.0 / (nextIt->first - it->first));
	}

	Mat res(mat.rows, mat.cols, CV_8UC3);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		res.at<Vec3b>(i, j) = { map[rgb[0]], map[rgb[1]], map[rgb[2]] };
	}

	return res;
}

Mat Utils::histogramEqualization(const Mat& mat) {
	array<int, 256> hist = getHistogram3Channel(mat);
	array<float, 256> cdf = getCDF(hist, mat.rows * mat.cols * 3);

	Mat res(mat.rows, mat.cols, CV_8UC3);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		res.at<Vec3b>(i, j) = { touc(cdf[rgb[0]] * 255), touc(cdf[rgb[1]] * 255), touc(cdf[rgb[2]] * 255) };
	}

	return res;
}

Mat Utils::histogramSpecificationSML(const Mat& orig, const Mat& pattern) {
	array<array<int, 256>, 3> origHist, patternHist;
	array<array<float, 256>, 3> origCDF, patternCDF;

	rep(i, 3) {
		origHist[i] = getHistogram1Channel(orig, i);
		patternHist[i] = getHistogram1Channel(pattern, i);
		origCDF[i] = getCDF(origHist[i], orig.rows * orig.cols);
		patternCDF[i] = getCDF(patternHist[i], pattern.rows * pattern.cols);
	}

	array<array<uchar, 256>, 3> map;
	rep(k, 3) {
		int tmpMin = 0, tmpMax = 0;
		rep(i, 256) {
			if (origCDF[k][i] <= patternCDF[k][tmpMin]) {
				map[k][i] = tmpMin;
			} else {
				if (origCDF[k][i] > patternCDF[k][tmpMax]) {
					while (tmpMax < 256 && origCDF[k][i] > patternCDF[k][tmpMax]) ++tmpMax;
					tmpMin = tmpMax - 1;
				}

				if (origCDF[k][i] - patternCDF[k][tmpMin] > patternCDF[k][tmpMax] - origCDF[k][i]) {
					map[k][i] = tmpMax;
				} else {
					map[k][i] = tmpMin;
				}
			}
		}
	}

	Mat res(orig.rows, orig.cols, CV_8UC3);
	rep(i, orig.rows) rep(j, orig.cols) {
		Vec3b rgb = orig.at<Vec3b>(i, j);
		res.at<Vec3b>(i, j) = { map[0][rgb[0]], map[1][rgb[1]], map[2][rgb[2]] };
	}

	return res;
}

Mat Utils::histogramSpecificationGML(const Mat& orig, const Mat& pattern) {
	array<array<int, 256>, 3> origHist, patternHist;
	array<array<float, 256>, 3> origCDF, patternCDF;

	rep(i, 3) {
		origHist[i] = getHistogram1Channel(orig, i);
		patternHist[i] = getHistogram1Channel(pattern, i);
		origCDF[i] = getCDF(origHist[i], orig.rows * orig.cols);
		patternCDF[i] = getCDF(patternHist[i], pattern.rows * pattern.cols);
	}

	array<array<uchar, 256>, 3> map;
	array<array<uchar, 256>, 3> invMap;
	rep(k, 3) {
		int tmpMin = 0, tmpMax = 0;
		rep(i, 256) {
			if (patternCDF[k][i] <= origCDF[k][tmpMin]) {
				invMap[k][i] = tmpMin;
			} else {
				if (patternCDF[k][i] > origCDF[k][tmpMax]) {
					while (tmpMax < 256 && patternCDF[k][i] > origCDF[k][tmpMax]) ++tmpMax;
					tmpMin = tmpMax - 1;
				}

				if (patternCDF[k][i] - origCDF[k][tmpMin] > origCDF[k][tmpMax] - patternCDF[k][i]) {
					invMap[k][i] = tmpMax;
				} else {
					invMap[k][i] = tmpMin;
				}
			}
		}

		tmpMin = -1;
		rep(i, 256) {
			if (patternHist[k][i]) {
				repa(j, tmpMin + 1, invMap[k][i] + 1) {
					map[k][j] = i;
				}
				tmpMin = invMap[k][i];
			}
		}
	}

	Mat res(orig.rows, orig.cols, CV_8UC3);
	rep(i, orig.rows) rep(j, orig.cols) {
		Vec3b rgb = orig.at<Vec3b>(i, j);
		res.at<Vec3b>(i, j) = { map[0][rgb[0]], map[1][rgb[1]], map[2][rgb[2]] };
	}

	return res;
}

Mat Utils::medianFilterImageMat(const Mat& mat, int size) {
	if (size < 3) {
		return mat;
	} else if (!(size % 2)) {
		--size;
	}

	int t = (size * size - 1) / 2;

	Mat res(mat.rows - size, mat.cols - size, CV_8UC3);
	rep(k, 3) {
		rep(i, mat.rows - size) {
			array<int, 256> hist = getHistogram1Channel(mat(Rect(0, i, size, size)), k);
			int med = 0, mNum = hist[0];

			while (mNum < t) mNum += hist[++med];
			res.at<Vec3b>(i, 0)[k] = med;

			repa(j, 1, mat.cols - size) {
				repa(m, i, i + size) {
					int tmp;
					tmp = mat.at<Vec3b>(m, j - 1)[k];
					--hist[tmp];
					if (tmp <= med) {
						--mNum;
					}

					tmp = mat.at<Vec3b>(m, j + size - 1)[k];
					++hist[tmp];
					if (tmp <= med) {
						++mNum;
					}
				}

				if (mNum <= t) {
					while (mNum < t) mNum += hist[++med];
					res.at<Vec3b>(i, j)[k] = med;
				} else {
					while (mNum > t) mNum -= hist[med--];
					res.at<Vec3b>(i, j)[k] = med;
				}
			}
		}
	}

	return res;
}

vector<float> Utils::getGaussianKernel1D(int size, float sigma) {
	vector<float> res;

	int mid = (size - 1) / 2;
	rep(i, size) {
		float tmp = exp(-sqr(i - mid) / (2 * sqr(sigma))) / (sqrt(2 * PI) * sigma);
		res.push_back(tmp);
	}

	float sum = accumulate(res.begin(), res.end(), 0.0);
	for (auto &elem : res) {
		elem /= sum;
	}

	return res;
}

Mat Utils::gaussianFilterImageMat(const Mat& mat, int size, float sigma) {
	vector<float> kernel1D = getGaussianKernel1D(size, sigma);

	Mat tmpMat(mat.rows - size, mat.cols, CV_32FC3);
	Mat res(mat.rows - size, mat.cols - size, CV_8UC3);
	rep(i, mat.rows - size) rep(j, mat.cols) {
		Vec3f tmp{ 0.0, 0.0, 0.0 };
		rep(k, size) {
			Vec3b rgb = mat.at<Vec3b>(i + k, j);
			tmp[0] += kernel1D[k] * rgb[0];
			tmp[1] += kernel1D[k] * rgb[1];
			tmp[2] += kernel1D[k] * rgb[2];
		}
		tmpMat.at<Vec3f>(i, j) = tmp;
	}

	rep(i, mat.rows - size) rep(j, mat.cols - size) {
		Vec3f tmp{ 0.0, 0.0, 0.0 };
		rep(k, size) {
			Vec3f rgb = tmpMat.at<Vec3f>(i, j + k);
			tmp[0] += kernel1D[k] * rgb[0];
			tmp[1] += kernel1D[k] * rgb[1];
			tmp[2] += kernel1D[k] * rgb[2];
		}
		res.at<Vec3b>(i, j) = { touc(tmp[0]), touc(tmp[1]), touc(tmp[2]) };
	}

	return res;
}

Mat Utils::getRobertFilterImageMat(const Mat& mat) {
	Mat res = Mat::zeros(mat.rows, mat.cols, CV_8UC3);

	repa(i, 1, mat.rows - 1) repa(j, 1, mat.cols - 1) {
		Mat crop = mat(Rect(j, i, 2, 2));
		Vec3i tmp;
		rep(k, 3) {
			tmp[k] = abs(crop.at<Vec3b>(0, 0)[k] - crop.at<Vec3b>(1, 1)[k]) +
				abs(crop.at<Vec3b>(0, 1)[k] - crop.at<Vec3b>(1, 0)[k]);
			updateMinMax(tmp[k], 255, 0);
		}
		res.at<Vec3b>(i, j) = { (uchar) tmp[0], (uchar) tmp[1], (uchar) tmp[2] };
	}

	return res;
}

Mat Utils::getPrewittFilterImageMat(const Mat& mat) {
	Mat res = Mat::zeros(mat.rows, mat.cols, CV_8UC3);

	repa(i, 1, mat.rows - 1) repa(j, 1, mat.cols - 1) {
		Mat crop = mat(Rect(j - 1, i - 1, 3, 3));
		Vec3i tmp;
		rep(k, 3) {
			tmp[k] = abs(crop.at<Vec3b>(0, 2)[k] + crop.at<Vec3b>(1, 2)[k] + crop.at<Vec3b>(2, 2)[k] - crop.at<Vec3b>(0, 0)[k] - crop.at<Vec3b>(1, 0)[k] - crop.at<Vec3b>(2, 0)[k]) +
				abs(crop.at<Vec3b>(2, 0)[k] + crop.at<Vec3b>(2, 1)[k] + crop.at<Vec3b>(2, 2)[k] - crop.at<Vec3b>(0, 0)[k] - crop.at<Vec3b>(0, 1)[k] - crop.at<Vec3b>(0, 2)[k]);
			updateMinMax(tmp[k], 255, 0);
		}
		res.at<Vec3b>(i, j) = { (uchar) tmp[0], (uchar) tmp[1], (uchar) tmp[2] };
	}

	return res;
}

Mat Utils::getSobelFilterImageMat(const Mat& mat) {
	Mat res = Mat::zeros(mat.rows, mat.cols, CV_8UC3);

	repa(i, 1, mat.rows - 1) repa(j, 1, mat.cols - 1) {
		Mat crop = mat(Rect(j - 1, i - 1, 3, 3));
		Vec3i tmp;
		rep(k, 3) {
			tmp[k] = abs(crop.at<Vec3b>(0, 2)[k] + 2 * crop.at<Vec3b>(1, 2)[k] + crop.at<Vec3b>(2, 2)[k] - crop.at<Vec3b>(0, 0)[k] - 2 * crop.at<Vec3b>(1, 0)[k] - crop.at<Vec3b>(2, 0)[k]) +
				abs(crop.at<Vec3b>(2, 0)[k] + 2 * crop.at<Vec3b>(2, 1)[k] + crop.at<Vec3b>(2, 2)[k] - crop.at<Vec3b>(0, 0)[k] - 2 * crop.at<Vec3b>(0, 1)[k] - crop.at<Vec3b>(0, 2)[k]);
			updateMinMax(tmp[k], 255, 0);
		}
		res.at<Vec3b>(i, j) = { (uchar) tmp[0], (uchar) tmp[1], (uchar) tmp[2] };
	}

	return res;
}

Mat Utils::getLaplaceFilterImageMat(const Mat& mat) {
	Mat res = Mat::zeros(mat.rows, mat.cols, CV_8UC3);

	repa(i, 1, mat.rows - 1) repa(j, 1, mat.cols - 1) {
		Mat crop = mat(Rect(j - 1, i - 1, 3, 3));
		Vec3i tmp;
		rep(k, 3) {
			tmp[k] = abs(crop.at<Vec3b>(1, 1)[k] * 8 - crop.at<Vec3b>(0, 0)[k] - crop.at<Vec3b>(0, 1)[k] - crop.at<Vec3b>(0, 2)[k] - crop.at<Vec3b>(1, 0)[k] - crop.at<Vec3b>(1, 2)[k] - crop.at<Vec3b>(2, 0)[k] - crop.at<Vec3b>(2, 1)[k] - crop.at<Vec3b>(2, 2)[k]);
			updateMinMax(tmp[k], 255, 0);
		}
		res.at<Vec3b>(i, j) = { (uchar) tmp[0], (uchar) tmp[1], (uchar) tmp[2] };
	}

	return res;
}

Mat Utils::sharpenImageMat(const Mat& mat, int type) {
	Mat res(mat.rows, mat.cols, CV_8UC3);
	Mat grad;

	float t = 0.1;

	if (type == 0) {
		t = 0.2;
		grad = getRobertFilterImageMat(mat);
	} else if (type == 1) {
		grad = getPrewittFilterImageMat(mat);
	} else if (type == 2) {
		grad = getSobelFilterImageMat(mat);
	} else if (type == 3) {
		t = 0.2;
		grad = getLaplaceFilterImageMat(mat);
	} else {
		return res;
	}

	rep(i, res.rows) rep(j, res.cols) {
		Vec3i tmp;
		rep(k, 3) {
			tmp[k] = round(mat.at<Vec3b>(i, j)[k] + t * grad.at<Vec3b>(i, j)[k]);
			updateMinMax(tmp[k], 255, 0);
		}
		res.at<Vec3b>(i, j) = { (uchar) tmp[0], (uchar) tmp[1], (uchar) tmp[2] };
	}

	return res;
}

void Utils::changePartialImageMatLightness(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float delta = deltas[0];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		float maxValue, minValue;
		maxValue = max(rgb[0], max(rgb[1], rgb[2]));
		minValue = min(rgb[0], min(rgb[1], rgb[2]));
		float L = (maxValue + minValue) * 1.0 / 510;
		if (delta < 1) {
			float alpha = L * (1 - delta) / (L * (1 - delta) + delta);
			res.at<Vec3b>(i, j) = { touc((1 - alpha) * rgb[0] + alpha * 255), touc((1 - alpha) * rgb[1] + alpha * 255), touc((1 - alpha) * rgb[2] + alpha * 255) };
		} else {
			res.at<Vec3b>(i, j) = { touc(rgb[0] / (L * (1 - delta) + delta)), touc(rgb[1] / (L * (1 - delta) + delta)), touc(rgb[2] / (L * (1 - delta) + delta)) };
		}
	}
}

void Utils::changePartialImageMatSaturation(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float delta = deltas[0];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		float maxValue, minValue;
		maxValue = max(rgb[0], max(rgb[1], rgb[2]));
		minValue = min(rgb[0], min(rgb[1], rgb[2]));
		float L, S;
		L = (maxValue + minValue) * 1.0 / 510;
		if (maxValue + minValue < 255) {
			S = (maxValue - minValue) * 1.0 / (maxValue + minValue);
		} else {
			S = (maxValue - minValue) * 1.0 / (510 - maxValue - minValue);
		}

		float alpha;
		if (delta > 0) {
			alpha = 1.0f / max(S, 1 - delta) - 1;
			res.at<Vec3b>(i, j) = { touc(rgb[0] + (rgb[0] - L * 255) * alpha), touc(rgb[1] + (rgb[1] - L * 255) * alpha), touc(rgb[2] + (rgb[2] - L * 255) * alpha) };
		} else {
			alpha = delta;
			res.at<Vec3b>(i, j) = { touc(L * 255 + (rgb[0] - L * 255) * (1 + alpha)), touc(L * 255 + (rgb[1] - L * 255) * (1 + alpha)), touc(L * 255+ (rgb[2] - L * 255) * (1 + alpha)) };
		}
	}
}

void Utils::changePartialImageMatHue(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float delta = deltas[0];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		Vec3f hsl = RGB2HSL(rgb);
		hsl[0] += delta;
		if (hsl[0] < 0) {
			hsl[0] += 360.0;
		} else if (hsl[0] > 360.0) {
			hsl[0] -= 360.0;
		}
		res.at<Vec3b>(i, j) = HSL2RGB(hsl);
	}
}

void Utils::changePartialImageMatGamma(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float gamma = deltas[0];
	float c = deltas[1];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		int tmpB, tmpG, tmpR;
		tmpB = round(pow(rgb[0] * 1.0 / 255, gamma) * c * 255);
		tmpG = round(pow(rgb[1] * 1.0 / 255, gamma) * c * 255);
		tmpR = round(pow(rgb[2] * 1.0 / 255, gamma) * c * 255);
		updateMinMax(tmpB, 255, 0);
		updateMinMax(tmpG, 255, 0);
		updateMinMax(tmpR, 255, 0);
		res.at<Vec3b>(i, j) = { (uchar) tmpB, (uchar) tmpG, (uchar) tmpR };
	}
}

void Utils::changePartialImageMatLog(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float a = deltas[0];
	float b = deltas[1];
	float c = deltas[2];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		int tmpB, tmpG, tmpR;
		tmpB = round((a + log(rgb[0] * 1.0 / 255 + 1) / (b * log(c))) * 255);
		tmpG = round((a + log(rgb[1] * 1.0 / 255 + 1) / (b * log(c))) * 255);
		tmpR = round((a + log(rgb[2] * 1.0 / 255 + 1) / (b * log(c))) * 255);
		updateMinMax(tmpB, 255, 0);
		updateMinMax(tmpG, 255, 0);
		updateMinMax(tmpR, 255, 0);
		res.at<Vec3b>(i, j) = { (uchar) tmpB, (uchar) tmpG, (uchar) tmpR };
	}
}

void Utils::changePartialImageMatPow(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float a = deltas[0];
	float b = deltas[1];
	float c = deltas[2];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3b rgb = mat.at<Vec3b>(i, j);
		int tmpB, tmpG, tmpR;
		tmpB = round((pow(b, c * (rgb[0] * 1.0 / 255 - a)) - 1) * 255);
		tmpG = round((pow(b, c * (rgb[1] * 1.0 / 255 - a)) - 1) * 255);
		tmpR = round((pow(b, c * (rgb[2] * 1.0 / 255 - a)) - 1) * 255);
		updateMinMax(tmpB, 255, 0);
		updateMinMax(tmpG, 255, 0);
		updateMinMax(tmpR, 255, 0);
		res.at<Vec3b>(i, j) = { (uchar) tmpB, (uchar) tmpG, (uchar) tmpR };
	}
}