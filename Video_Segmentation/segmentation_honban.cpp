#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include "CSVParser.h"




const int THRESHOLD_FOR_DISTANCE = 16* 16;
const double KERNEL_SIZE = 2 * 16 * 16;

class STPoint
{
public:
	int x;
	int y;
	int t;

	STPoint(int _x = 0, int _y = 0, int _t = 0) : x(_x), y(_y), t(_t) {}
};

template <typename tn>
static inline tn square(tn x) {
	return x * x;
}

int main()
{
	kn::CSVParser parser;
	std::vector<STPoint> temp;
	std::vector<std::vector<STPoint>> points;
	std::ifstream ifs1("video.txt");
	std::ifstream ifs2("UCF_trajectories_path.txt");
	std::ifstream ifs3("UCF_segmentation.txt");
	std::string video;
	std::string trajectories_path;
	std::string segmentation;

	while (getline(ifs1, video)){
		getline(ifs2, trajectories_path);
		getline(ifs3, segmentation);

		cv::VideoCapture cap = cv::VideoCapture(video);
		cv::VideoWriter wrt;
		wrt.open(segmentation, cap.get(CV_CAP_PROP_FOURCC), cap.get(CV_CAP_PROP_FPS), cv::Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT)));

		const int n_frames = static_cast<int>(cap.get(CV_CAP_PROP_FRAME_COUNT));

		points.assign(n_frames, temp);

		parser.SetDelimiter('\t');
		parser.Parse(trajectories_path);
		for (unsigned int i = 0; i < parser.RowSize(); ++i) {
			int base_t = parser.GetDataAsInt(i, 0);
			for (unsigned int j = 10; j < 42; j += 2) {
				STPoint p;
				p.x = static_cast<int>(round(parser.GetDataAsDouble(i, j)));
				p.y = static_cast<int>(round(parser.GetDataAsDouble(i, j + 1)));
				p.t = base_t + (j - 10) / 2;
				if (p.t < n_frames) points[p.t].push_back(p);
			}
		}

		for (int n = 0; cv::waitKey(1) == -1; ++n)
		{
			cv::Mat frame;
			cv::Mat probability;
			// 映像フレーム取得
			cap >> frame;
			if (frame.empty())
				break;

			probability = cv::Mat::zeros(frame.rows, frame.cols, CV_8UC1);
			for (int y = 0; y < frame.rows; ++y) {
				for (int x = 0; x < frame.cols; ++x) {
					double score = 0;
					for (int i = 0; i < points[n].size(); ++i) {
						int d = square(x - points[n][i].x) + square(y - points[n][i].y);
						if (d > THRESHOLD_FOR_DISTANCE) continue;
						score += exp(static_cast<double>(-d / KERNEL_SIZE));		//カーネル密度推定
					}
					probability.at<unsigned char>(y, x) = 255 - cv::saturate_cast<unsigned char>(score * 32);
				}
			}

			for (int y = 0; y < frame.rows; ++y) {
				for (int x = 0; x < frame.cols; ++x) {
					if (probability.at<unsigned char>(y, x) < 10){
						frame.at<cv::Vec3b>(y, x) = 0;
					}
				}
			}


			/*	for (int i = 0; i < points[n].size(); ++i) {
					cv::circle(frame, cv::Point(points[n][i].x, points[n][i].y), 2, cv::Scalar(0, 0, 255), -1);
					}
					*/

			//	cv::imshow("output", frame);
			//	cv::imshow("probability", probability);
			wrt << frame;

		}
	}
}
