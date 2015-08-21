#include "retro_filter.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "ctime"
using namespace std;
using namespace cv;

inline void alphaBlend(const Mat& src, Mat& dst, const Mat& alpha)
{
    Mat w, d, s, dw, sw;
    alpha.convertTo(w, CV_32S);
    src.convertTo(s, CV_32S);
    dst.convertTo(d, CV_32S);

    //multiply(s, w, sw);
   // multiply(d, -w, dw);
	multiply(s-d,w,sw);
    d = (d*255 + sw )/255.0;
    d.convertTo(dst, CV_8U);
}

RetroFilter::RetroFilter(const Parameters& params) : rng_(time(0))
{
    params_ = params;

    resize(params_.fuzzyBorder, params_.fuzzyBorder, params_.frameSize);

    if (params_.scratches.rows < params_.frameSize.height ||
        params_.scratches.cols < params_.frameSize.width)
    {
        resize(params_.scratches, params_.scratches, params_.frameSize);
    }

    hsvScale_ = 1;
    hsvOffset_ = 20;
}

void RetroFilter::applyToVideo(const Mat& frame, Mat& retroFrame)
{
    int col, row;
    Mat luminance;

    cvtColor(frame, luminance, CV_BGR2GRAY);

    // Add scratches
    Scalar meanColor = mean(luminance.row(luminance.rows / 2));


    Mat scratchColor(params_.frameSize, CV_8UC1, meanColor * 2.0);

    int x = rng_.uniform(0, params_.scratches.cols - luminance.cols);
    int y = rng_.uniform(0, params_.scratches.rows - luminance.rows);

	for (row = 0; row < luminance.rows; row ++)
    {
		for (col = 0; col < luminance.cols; col ++)
        {
		
            luminance.at<uchar>(row, col) = params_.scratches.at<uchar>(row + y, col + x) ? scratchColor.at<uchar>(row, col) : luminance.at<uchar>(row, col);
        }
    }

    // Add fuzzy border
    Mat borderColor(params_.frameSize, CV_32FC1, meanColor * 1.5 );

    alphaBlend(borderColor, luminance, params_.fuzzyBorder);

    // Apply sepia-effect
    retroFrame.create(luminance.size(), CV_8UC3);

	vector<Mat> channels;
	split(retroFrame, channels);
	channels[0] = 19;
	channels[1] = 78;
	channels[2] = luminance * hsvScale_ + hsvOffset_;
	merge(channels, retroFrame);
	cvtColor( retroFrame, retroFrame,CV_HSV2BGR);
}
