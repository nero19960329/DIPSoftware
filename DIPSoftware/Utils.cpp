#include "Utils.h"

#include <future>

#include <QDebug>

#define EPSILON 1e-3

using namespace cv;
using namespace std;

QString Utils::getExtension(const String& str) {
	size_t pos = str.find_last_of('.');
	QString res;
	for (int i = pos + 1; i < str.size(); ++i) {
		res.append(str[i]);
	}
	return res;
}

QImage Utils::mat2QImage(const Mat& mat) {
	if (mat.type() == CV_8UC1) {
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		image.setColorCount(256);
		for (int i = 0; i < 256; i++) {
			image.setColor(i, qRgb(i, i, i));
		}
		uchar *pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++) {
			uchar *pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	} else if (mat.type() == CV_8UC3) {
		const uchar *pSrc = (const uchar*) mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	} else if (mat.type() == CV_8UC4) {
		const uchar *pSrc = (const uchar*) mat.data;
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
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

	return Vec3b(round(r), round(g), round(b));
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

Vec3b Utils::HSL2RGB(Vec3f hsl) {
	float h, s, l;
	h = hsl[0];
	s = hsl[1];
	l = hsl[2];

	if (s < 1e-3) {
		uchar tmp = round(l * 255);
		return Vec3b(tmp, tmp, tmp);
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

	uchar r, g, b;
	r = round(rgb[2] * 255);
	g = round(rgb[1] * 255);
	b = round(rgb[0] * 255);

	return Vec3b(r, g, b);
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

	Mat res = Mat(newW, newH, CV_8UC3);
	for (int i = 0; i < res.rows; ++i) {
		for (int j = 0; j < res.cols; ++j) {
			float x, y;
			x = cosTheta * (i - cx - dx) - sinTheta * (j - cy - dy) + cx;
			y = sinTheta * (i - cx - dx) + cosTheta * (j - cy - dy) + cy;
			if (x >= -EPSILON && x < w + EPSILON && y >= -EPSILON && y < h + EPSILON) {
				res.at<Vec3b>(i, j) = biLinearInterpolation(mat, x, y);
			} else {
				res.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
			}
		}
	}

	return res;
}

Mat Utils::changeImageMatLightness(const Mat& mat, float delta) {
	Mat res(mat.rows, mat.cols, CV_8UC3);
	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 2; ++j) {
			getPartialResult[i + i + j] = async(&Utils::changePartialImageMatLightness, mat, res, delta, x[i], x[i + 1], y[j], y[j + 1]);
		}
	}
	
	for (int i = 0; i < 8; ++i) {
		getPartialResult[i].get();
	}

	return res;
}

void Utils::changePartialImageMatLightness(const Mat& mat, Mat& res, float delta, int x0, int x1, int y0, int y1) {
	for (int i = x0; i < x1; ++i) {
		for (int j = y0; j < y1; ++j) {
			Vec3b rgb = mat.at<Vec3b>(i, j);
			Vec3f hsl = RGB2HSL(rgb);
			hsl[2] *= 1.0 / (hsl[2] + exp(-delta) * (1 - hsl[2]));
			res.at<Vec3b>(i, j) = HSL2RGB(hsl);
		}
	}
}

Mat Utils::changeImageMatSaturation(const Mat& mat, float delta) {
	Mat res(mat.rows, mat.cols, CV_8UC3);
	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 2; ++j) {
			getPartialResult[i + i + j] = async(&Utils::changePartialImageMatSaturation, mat, res, delta, x[i], x[i + 1], y[j], y[j + 1]);
		}
	}

	for (int i = 0; i < 8; ++i) {
		getPartialResult[i].get();
	}

	return res;
}

void Utils::changePartialImageMatSaturation(const Mat& mat, Mat& res, float delta, int x0, int x1, int y0, int y1) {
	for (int i = x0; i < x1; ++i) {
		for (int j = y0; j < y1; ++j) {
			Vec3b rgb = mat.at<Vec3b>(i, j);
			Vec3f hsl = RGB2HSL(rgb);
			hsl[1] *= 1.0 / (hsl[1] + exp(-delta) * (1 - hsl[1]));
			res.at<Vec3b>(i, j) = HSL2RGB(hsl);
		}
	}
}

Mat Utils::changeImageMatHue(const Mat& mat, float delta) {
	Mat res(mat.rows, mat.cols, CV_8UC3);
	int x[5] = { 0, mat.rows / 4, mat.rows / 2, mat.rows * 3 / 4, mat.rows };
	int y[3] = { 0, mat.cols / 2, mat.cols };
	future<void> getPartialResult[8];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 2; ++j) {
			getPartialResult[i + i + j] = async(&Utils::changePartialImageMatHue, mat, res, delta, x[i], x[i + 1], y[j], y[j + 1]);
		}
	}

	for (int i = 0; i < 8; ++i) {
		getPartialResult[i].get();
	}

	return res;
}

void Utils::changePartialImageMatHue(const Mat& mat, Mat& res, float delta, int x0, int x1, int y0, int y1) {
	for (int i = x0; i < x1; ++i) {
		for (int j = y0; j < y1; ++j) {
			Vec3b rgb = mat.at<Vec3b>(i, j);
			Vec3f hsl = RGB2HSL(rgb);
			hsl[0] -= delta;
			if (hsl[0] < 0) {
				hsl[0] += 360.0;
			} else if (hsl[0] > 360.0) {
				hsl[0] -= 360.0;
			}
			res.at<Vec3b>(i, j) = HSL2RGB(hsl);
		}
	}
}