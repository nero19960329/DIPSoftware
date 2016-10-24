#include "Utils.h"

#include <future>

#include <QDebug>

#define EPSILON 1e-3

using namespace cv;
using namespace std;

QString Utils::getExtension(const String& str) {
	auto pos = str.find_last_of('.');
	QString res;
	repa(i, pos + 1, str.size()) {
		res.append(str[i]);
	}
	return res;
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

	return Vec3f(r, g, b);
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
		return Vec3f(h, s, l);
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

	return Vec3f(h, s, l);
}

Vec3f Utils::HSL2RGB(Vec3f hsl) {
	float h, s, l;
	h = hsl[0];
	s = hsl[1];
	l = hsl[2];

	if (s < 1e-3) {
		return Vec3f(l, l, l);
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

	return Vec3f(rgb[2], rgb[1], rgb[0]);
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
			res.at<Vec3f>(i, j) = Vec3f(1.0f, 1.0f, 1.0f);
		}
	}

	return res;
}

Mat Utils::changeImageMatLightness(const Mat& mat, float delta) {
	Mat res(mat.rows, mat.cols, CV_32FC3);

	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	rep(i, 4) rep(j, 2) {
		getPartialResult[i + i + j] = async(&Utils::changePartialImageMatLightness, mat, res, exp(-delta), x[i], x[i + 1], y[j], y[j + 1]);
	}
	
	rep(i, 8) {
		getPartialResult[i].get();
	}

	return res;
}

void Utils::changePartialImageMatLightness(const Mat& mat, Mat& res, float delta, int x0, int x1, int y0, int y1) {
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

Mat Utils::changeImageMatSaturation(const Mat& mat, float delta) {
	Mat res(mat.rows, mat.cols, CV_32FC3);
	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	rep(i, 4) rep(j, 2) {
		getPartialResult[i + i + j] = async(&Utils::changePartialImageMatSaturation, mat, res, delta, x[i], x[i + 1], y[j], y[j + 1]);
	}

	rep(i, 8) {
		getPartialResult[i].get();
	}

	return res;
}

void Utils::changePartialImageMatSaturation(const Mat& mat, Mat& res, float delta, int x0, int x1, int y0, int y1) {
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
			res.at<Vec3f>(i, j) = Vec3f(rgb[0] + (rgb[0] - L) * alpha, rgb[1] + (rgb[1] - L) * alpha, rgb[2] + (rgb[2] - L) * alpha);
		} else {
			alpha = delta;
			res.at<Vec3f>(i, j) = Vec3f(L + (rgb[0] - L) * (1 + alpha), L + (rgb[1] - L) * (1 + alpha), L + (rgb[2] - L) * (1 + alpha));
		}
	}
}

Mat Utils::changeImageMatHue(const Mat& mat, float delta) {
	Mat res(mat.rows, mat.cols, CV_32FC3);
	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	rep(i, 4) rep(j, 2) {
		getPartialResult[i + i + j] = async(&Utils::changePartialImageMatHue, mat, res, delta, x[i], x[i + 1], y[j], y[j + 1]);
	}

	rep(i, 8) {
		getPartialResult[i].get();
	}

	return res;
}

void Utils::changePartialImageMatHue(const Mat& mat, Mat& res, float delta, int x0, int x1, int y0, int y1) {
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