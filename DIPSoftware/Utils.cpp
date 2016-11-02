#include "Utils.h"
#include "DebugUtils.h"

#include <future>
#include <numeric>

#include <QDebug>

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

void Utils::getSequence(const Vec3f &vec, float &max, float &mid, float &min) {
	if (vec[0] >= vec[1] && vec[0] >= vec[2]) {
		max = vec[0];
		mid = vec[1];
		min = vec[2];
	} else if (vec[0] >= vec[2] && vec[2] >= vec[1]) {
		max = vec[0];
		mid = vec[2];
		min = vec[1];
	} else if (vec[1] >= vec[0] && vec[0] >= vec[2]) {
		max = vec[1];
		mid = vec[0];
		min = vec[2];
	} else if (vec[1] >= vec[2] && vec[2] >= vec[0]) {
		max = vec[1];
		mid = vec[2];
		min = vec[0];
	} else if (vec[2] >= vec[0] && vec[0] >= vec[1]) {
		max = vec[2];
		mid = vec[0];
		min = vec[1];
	} else {
		max = vec[2];
		mid = vec[1];
		min = vec[0];
	}
}

Mat Utils::mat8U2Mat32F(const Mat& mat8U) {
	Mat mat32F(mat8U.rows, mat8U.cols, CV_32FC3);
	mat8U.convertTo(mat32F, CV_32FC3, 1.0f / 255);
	return mat32F;
}

Mat Utils::mat32F2Mat8U(const Mat& mat32F) {
	Mat mat8U(mat32F.rows, mat32F.cols, CV_8UC3);
	mat32F.convertTo(mat8U, CV_8UC3, 255, 0.5);
	return mat8U;
}

QImage Utils::mat2QImage(const Mat& mat) {
	if (mat.type() == CV_32FC3) {
		Mat mat8U = mat32F2Mat8U(mat);
		const uchar *pSrc = (const uchar*) mat8U.data;
		QImage image(pSrc, mat8U.cols, mat8U.rows, mat8U.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	} else {
		return QImage();
	}
}

Vec3f Utils::biLinearInterpolation(const Mat& mat, float x, float y) {
	int x1, x2, y1, y2;
	x1 = (int) x;
	y1 = (int) y;
	x2 = x1 + 1;
	y2 = y1 + 1;

	float r, g, b;

	if (x < mat.rows - 1 && y < mat.cols - 1) {
		r = mat.at<Vec3f>(x1, y1)[0] * (x2 - x) * (y2 - y) +
			mat.at<Vec3f>(x2, y1)[0] * (x - x1) * (y2 - y) +
			mat.at<Vec3f>(x1, y2)[0] * (x2 - x) * (y - y1) +
			mat.at<Vec3f>(x2, y2)[0] * (x - x1) * (y - y1);
		g = mat.at<Vec3f>(x1, y1)[1] * (x2 - x) * (y2 - y) +
			mat.at<Vec3f>(x2, y1)[1] * (x - x1) * (y2 - y) +
			mat.at<Vec3f>(x1, y2)[1] * (x2 - x) * (y - y1) +
			mat.at<Vec3f>(x2, y2)[1] * (x - x1) * (y - y1);
		b = mat.at<Vec3f>(x1, y1)[2] * (x2 - x) * (y2 - y) +
			mat.at<Vec3f>(x2, y1)[2] * (x - x1) * (y2 - y) +
			mat.at<Vec3f>(x1, y2)[2] * (x2 - x) * (y - y1) +
			mat.at<Vec3f>(x2, y2)[2] * (x - x1) * (y - y1);
	} else if (x == mat.rows - 1 && y < mat.cols - 1) {
		r = mat.at<Vec3f>(x1, y1)[0] * (y2 - y) + mat.at<Vec3f>(x1, y2)[0] * (y - y1);
		g = mat.at<Vec3f>(x1, y1)[1] * (y2 - y) + mat.at<Vec3f>(x1, y2)[1] * (y - y1);
		b = mat.at<Vec3f>(x1, y1)[2] * (y2 - y) + mat.at<Vec3f>(x1, y2)[2] * (y - y1);
	} else if (x < mat.rows - 1 && y == mat.cols - 1) {
		r = mat.at<Vec3f>(x1, y1)[0] * (x2 - x) + mat.at<Vec3f>(x2, y1)[0] * (x - x1);
		g = mat.at<Vec3f>(x1, y1)[1] * (x2 - x) + mat.at<Vec3f>(x2, y1)[1] * (x - x1);
		b = mat.at<Vec3f>(x1, y1)[2] * (x2 - x) + mat.at<Vec3f>(x2, y1)[2] * (x - x1);
	} else {
		r = mat.at<Vec3f>(x1, y1)[0];
		g = mat.at<Vec3f>(x1, y1)[1];
		b = mat.at<Vec3f>(x1, y1)[2];
	}

	return { r, g, b };
}

Vec3f Utils::RGB2HSL(Vec3f rgb) {
	float h, s, l;
	float r, g, b;
	r = rgb[2];
	g = rgb[1];
	b = rgb[0];

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

Vec3f Utils::HSL2RGB(Vec3f hsl) {
	float h, s, l;
	h = hsl[0];
	s = hsl[1];
	l = hsl[2];

	if (s < 1e-3) {
		return { l, l, l };
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

	return { rgb[2], rgb[1], rgb[0] };
}

Mat Utils::updateImageMat(const Mat& mat) {
	Mat res(mat);
	rep(i, res.rows) rep(j, res.cols) {
		Vec3f &rgb = res.at<Vec3f>(i, j);
		updateMinMax(rgb[0], 1.0f, 0.0f);
		updateMinMax(rgb[1], 1.0f, 0.0f);
		updateMinMax(rgb[2], 1.0f, 0.0f);
	}
	return res;
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

	Mat res(newW, newH, CV_32FC3);
	rep(i, res.rows) rep(j, res.cols) {
		float x, y;
		x = cosTheta * (i - cx - dx) - sinTheta * (j - cy - dy) + cx;
		y = sinTheta * (i - cx - dx) + cosTheta * (j - cy - dy) + cy;
		if (betw(x, -EPSILON, w + EPSILON) && betw(y, -EPSILON, h + EPSILON)) {
			res.at<Vec3f>(i, j) = biLinearInterpolation(mat, x, y);
		} else {
			res.at<Vec3f>(i, j) = { 1.0f, 1.0f, 1.0f };
		}
	}

	return res;
}

Mat Utils::changeImageMat(const Mat& mat, vector<float> deltas, changeFuncType changeFunc) {
	Mat res(mat.rows, mat.cols, CV_32FC3);

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
		Vec3f rgb = mat.at<Vec3f>(i, j);
		unsigned int grey = (rgb[0] + rgb[1] + rgb[2]) * 85.0 + 0.5;
		++res[grey];
	}
	return res;
}

array<int, 256> Utils::getHistogram1Channel(const Mat& mat, int channel) {
	array<int, 256> res;
	fill(res.begin(), res.end(), 0);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		++res[rgb[channel] * 255.0 + 0.5];
	}
	return res;
}

array<int, 256> Utils::getHistogram3Channel(const Mat& mat) {
	array<int, 256> res;
	fill(res.begin(), res.end(), 0);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		++res[rgb[0] * 255 + 0.5];
		++res[rgb[1] * 255 + 0.5];
		++res[rgb[2] * 255 + 0.5];
	}
	return res;
}

array<float, 256> Utils::getCDF(const array<int, 256>& hist, int pixels) {
	array<float, 256> res;
	array<int, 256> tmp;
	tmp[0] = hist[0] * 1.0 / pixels;
	repa(i, 1, 256) {
		tmp[i] = tmp[i - 1] + hist[i];
		res[i] = tmp[i] * 1.0 / pixels;
	}
	return res;
}

Mat Utils::linearConvert(const Mat& mat, const list<pair<float, float>>& vertices) {
	auto cvt = [=](float d){ return (int) (d * 255 + 0.5); };

	array<float, 256> map;
	auto it = vertices.begin();
	auto nextIt = vertices.begin();
	++nextIt;
	rep(i, 256) {
		if (i > cvt(nextIt->first)) {
			++it;
			++nextIt;
		}
		map[i] = ((nextIt->second - it->second) * i / 255 + (it->second * nextIt->first - it->first * nextIt->second)) / (nextIt->first - it->first);
	}

	Mat res(mat.rows, mat.cols, CV_32FC3);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		res.at<Vec3f>(i, j) = { map[cvt(rgb[0])], map[cvt(rgb[1])], map[cvt(rgb[2])] };
	}

	return res;
}

Mat Utils::histogramEqualization(const Mat& mat) {
	array<int, 256> hist = getHistogram3Channel(mat);
	array<float, 256> cdf = getCDF(hist, mat.rows * mat.cols * 3);

	Mat res(mat.rows, mat.cols, CV_32FC3);
	rep(i, mat.rows) rep(j, mat.cols) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		res.at<Vec3f>(i, j) = { cdf[rgb[0] * 255 + 0.5], cdf[rgb[1] * 255 + 0.5], cdf[rgb[2] * 255 + 0.5] };
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

	array<array<float, 256>, 3> map;
	rep(k, 3) {
		int tmpMin = 0, tmpMax = 0;
		rep(i, 256) {
			if (origCDF[k][i] <= patternCDF[k][tmpMin]) {
				map[k][i] = tmpMin * 1.0f / 255;
			} else {
				if (origCDF[k][i] > patternCDF[k][tmpMax]) {
					while (tmpMax < 256 && origCDF[k][i] > patternCDF[k][tmpMax]) ++tmpMax;
					tmpMin = tmpMax - 1;
				}

				if (origCDF[k][i] - patternCDF[k][tmpMin] > patternCDF[k][tmpMax] - origCDF[k][i]) {
					map[k][i] = tmpMax * 1.0f / 255;
				} else {
					map[k][i] = tmpMin * 1.0f / 255;
				}
			}
		}
	}

	Mat res(orig.rows, orig.cols, CV_32FC3);
	rep(i, orig.rows) rep(j, orig.cols) {
		Vec3f rgb = orig.at<Vec3f>(i, j);
		res.at<Vec3f>(i, j) = { map[0][rgb[0] * 255 + 0.5], map[1][rgb[1] * 255 + 0.5], map[2][rgb[2] * 255 + 0.5] };
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

	array<array<float, 256>, 3> map;
	array<array<int, 256>, 3> invMap;
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
					map[k][j] = i * 1.0f / 255;
				}
				tmpMin = invMap[k][i];
			}
		}
	}

	Mat res(orig.rows, orig.cols, CV_32FC3);
	rep(i, orig.rows) rep(j, orig.cols) {
		Vec3f rgb = orig.at<Vec3f>(i, j);
		res.at<Vec3f>(i, j) = { map[0][rgb[0] * 255 + 0.5], map[1][rgb[1] * 255 + 0.5], map[2][rgb[2] * 255 + 0.5] };
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

	Mat res(mat.rows - size, mat.cols - size, CV_32FC3);
	rep(k, 3) {
		rep(i, mat.rows - size) {
			array<int, 256> hist = getHistogram1Channel(mat(Rect(0, i, size, size)), k);
			int med = 0, mNum = hist[0];

			while (mNum < t) mNum += hist[++med];
			res.at<Vec3f>(i, 0)[k] = med * 1.0f / 255;

			repa(j, 1, mat.cols - size) {
				repa(m, i, i + size) {
					int tmp = mat.at<Vec3f>(m, j - 1)[k] * 255.0 + 0.5;
					--hist[tmp];
					if (tmp <= med) {
						--mNum;
					}
				}

				repa(m, i, i + size) {
					int tmp = mat.at<Vec3f>(m, j + size - 1)[k] * 255.0 + 0.5;
					++hist[tmp];
					if (tmp <= med) {
						++mNum;
					}
				}

				if (mNum <= t) {
					while (mNum < t) mNum += hist[++med];
					res.at<Vec3f>(i, j)[k] = med * 1.0f / 255;
				} else {
					while (mNum > t) mNum -= hist[med--];
					res.at<Vec3f>(i, j)[k] = med * 1.0f / 255;
				}
			}
		}
	}

	return res;
}

void Utils::changePartialImageMatLightness(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float delta = deltas[0];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		float maxValue, minValue;
		maxValue = max(rgb[0], max(rgb[1], rgb[2]));
		minValue = min(rgb[0], min(rgb[1], rgb[2]));
		float L = (maxValue + minValue) * 1.0 / 2;
		if (delta < 1) {
			float alpha = L * (1 - delta) / (L * (1 - delta) + delta);
			res.at<Vec3f>(i, j) = (1 - alpha) * rgb + Vec3f(alpha, alpha, alpha);
		} else {
			res.at<Vec3f>(i, j) = rgb / (L * (1 - delta) + delta);
		}
	}
}

void Utils::changePartialImageMatSaturation(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float delta = deltas[0];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		float maxValue, minValue;
		maxValue = max(rgb[0], max(rgb[1], rgb[2]));
		minValue = min(rgb[0], min(rgb[1], rgb[2]));
		float L, S;
		L = (maxValue + minValue) * 1.0 / 2;
		if (maxValue + minValue < 1) {
			S = (maxValue - minValue) * 1.0 / (maxValue + minValue);
		} else {
			S = (maxValue - minValue) * 1.0 / (2 - maxValue - minValue);
		}

		float alpha;
		if (delta > 0) {
			alpha = 1.0f / max(S, 1 - delta) - 1;
			res.at<Vec3f>(i, j) = { rgb[0] + (rgb[0] - L) * alpha, rgb[1] + (rgb[1] - L) * alpha, rgb[2] + (rgb[2] - L) * alpha };
		} else {
			alpha = delta;
			res.at<Vec3f>(i, j) = { L + (rgb[0] - L) * (1 + alpha), L + (rgb[1] - L) * (1 + alpha), L + (rgb[2] - L) * (1 + alpha) };
		}
	}
}

void Utils::changePartialImageMatHue(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float delta = deltas[0];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		Vec3f hsl = RGB2HSL(rgb);
		hsl[0] += delta;
		if (hsl[0] < 0) {
			hsl[0] += 360.0;
		} else if (hsl[0] > 360.0) {
			hsl[0] -= 360.0;
		}
		res.at<Vec3f>(i, j) = HSL2RGB(hsl);
	}
}

void Utils::changePartialImageMatGamma(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float gamma = deltas[0];
	float c = deltas[1];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		res.at<Vec3f>(i, j) = { pow(rgb[0], gamma) * c, pow(rgb[1], gamma) * c, pow(rgb[2], gamma) * c };
	}
}

void Utils::changePartialImageMatLog(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float a = deltas[0];
	float b = deltas[1];
	float c = deltas[2];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		Vec3f tmp;
		tmp[0] = a + log(rgb[0] + 1) / (b * log(c));
		tmp[1] = a + log(rgb[1] + 1) / (b * log(c));
		tmp[2] = a + log(rgb[2] + 1) / (b * log(c));
		res.at<Vec3f>(i, j) = tmp;
	}
}

void Utils::changePartialImageMatPow(const Mat& mat, Mat& res, vector<float> deltas, int x0, int x1, int y0, int y1) {
	float a = deltas[0];
	float b = deltas[1];
	float c = deltas[2];
	repa(i, x0, x1) repa(j, y0, y1) {
		Vec3f rgb = mat.at<Vec3f>(i, j);
		Vec3f tmp;
		tmp[0] = pow(b, c * (rgb[0] - a)) - 1;
		tmp[1] = pow(b, c * (rgb[1] - a)) - 1;
		tmp[2] = pow(b, c * (rgb[2] - a)) - 1;
		res.at<Vec3f>(i, j) = tmp;
	}
}