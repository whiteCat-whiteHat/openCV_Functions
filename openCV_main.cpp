#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <opencv2/core/core.hpp>	// cv::Mat
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cv::imread()
#include <opencv2/imgproc/imgproc.hpp>  // cv::Canny()


using namespace std;
using namespace cv;


void FourierTransformation()
{

	// The Fourier Transform will decompose an image into its sin and cosines components.

	Mat I = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	if( I.empty())
		return -1;

	Mat padded; //expand input image to optimal size
	int m = getOptimalDFTSize( I.rows );
	int n = getOptimalDFTSize( I.cols ); // on the border add zero values
	copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
	Mat complexI;
	merge(planes, 2, complexI); // Add to the expanded another plane with zeros

	dft(complexI, complexI); // this way the result may fit in the source matrix

	// compute the magnitude and switch to logarithmic scale
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	Mat magI = planes[0];

	magI += Scalar::all(1); // switch to logarithmic scale
	log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrants of Fourier image so that the origin is at the image center
	int cx = magI.cols/2;
	int cy = magI.rows/2;

	Mat q0(magI, Rect(0, 0, cx, cy)); // Top-Left - Create a ROI per quadrant
	Mat q1(magI, Rect(cx, 0, cx, cy)); // Top-Right
	Mat q2(magI, Rect(0, cy, cx, cy)); // Bottom-Left
	Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp; // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp); // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
 	// viewable image form (float between values 0 and 1).

	imshow("Input Image" , I ); // Show the result
	imshow("spectrum magnitude", magI);
	waitKey();
	return 0;
}

}

int Displaying_Big_End( Mat image, char* window_name, RNG rng )
{
	Size textsize = getTextSize("OpenCV forever!", CV_FONT_HERSHEY_COMPLEX, 3, 5, 0);
	Point org((window_width - textsize.width)/2, (window_height - textsize.height)/2);
	int lineType = 8;
	Mat image2;
	for( int i = 0; i < 255; i += 2 )
	{
		image2 = image - Scalar::all(i);
		putText( image2, "OpenCV forever!", org, CV_FONT_HERSHEY_COMPLEX, 3,
		Scalar(i, i, 255), 5, lineType );
		imshow( window_name, image2 );
		if( waitKey(DELAY) >= 0 )
		{ return -1; }
	}
	return 0;
}

int Displaying_Random_Text( Mat image, char* window_name, RNG rng )
{
	int lineType = 8;
	for ( int i = 1; i < NUMBER; i++ )
	{
		Point org;
		org.x = rng.uniform(x_1, x_2);
		org.y = rng.uniform(y_1, y_2);
		putText( image, "Testing text rendering", org, rng.uniform(0,8),
		rng.uniform(0,100)*0.05+0.1, randomColor(rng), rng.uniform(1, 10), lineType);
		imshow( window_name, image );
		if( waitKey(DELAY) >= 0 )
		{ return -1; }
	}
	return 0;
}

int Drawing_Random_Lines( Mat image, char* window_name, RNG rng )
{
	int lineType = 8;
	Point pt1, pt2;
	for( int i = 0; i < NUMBER; i++ )
	{
		pt1.x = rng.uniform( x_1, x_2 );
		pt1.y = rng.uniform( y_1, y_2 );
		pt2.x = rng.uniform( x_1, x_2 );
		pt2.y = rng.uniform( y_1, y_2 );
		line( image, pt1, pt2, randomColor(rng), rng.uniform(1, 10), 8 );
		imshow( window_name, image );
		if( waitKey( DELAY ) >= 0 )
		{ return -1; }
	}
	return 0;
}



void randomGeneratorAndText()
{
	RNG rng( 0xFFFFFFFF );
	
	/// Initialize a matrix filled with zeros
	Mat image = Mat::zeros( window_height, window_width, CV_8UC3 );

	/// Show it in a window during DELAY ms
	imshow( window_name, image );

	/// Now, let's draw some lines
	c = Drawing_Random_Lines(image, window_name, rng);
	if( c != 0 ) return 0;
	
	/// Go on drawing, this time nice rectangles
	c = Drawing_Random_Rectangles(image, window_name, rng);
	if( c != 0 ) return 0;
	
	/// Draw some ellipses
	c = Drawing_Random_Ellipses( image, window_name, rng );
	if( c != 0 ) return 0;
	
	/// Now some polylines
	c = Drawing_Random_Polylines( image, window_name, rng );
	if( c != 0 ) return 0;
	
	/// Draw filled polygons
	c = Drawing_Random_Filled_Polygons( image, window_name, rng );
	if( c != 0 ) return 0;
	
	/// Draw circles
	c = Drawing_Random_Circles( image, window_name, rng );
	if( c != 0 ) return 0;
	
	/// Display text in random positions
	c = Displaying_Random_Text( image, window_name, rng );
	if( c != 0 ) return 0;
	
	/// Displaying the big end!
	c = Displaying_Big_End( image, window_name, rng );
}


void contrastBrightnessChange()
{

	double alpha; /**< Simple contrast control */
	int beta; /**< Simple brightness control */

	/// Read image given by user
	Mat image = imread( argv[1] );
	Mat new_image = Mat::zeros( image.size(), image.type() );

	/// Initialize values
	std::cout<<" Basic Linear Transforms "<<std::endl;
	std::cout<<"-------------------------"<<std::endl;
	std::cout<<"* Enter the alpha value [1.0-3.0]: ";
	std::cin>>alpha;
	std::cout<<"* Enter the beta value [0-100]: "; 
	std::cin>>beta;

	/// Do the operation new_image(i,j) = alpha*image(i,j) + beta
	for( int y = 0; y < image.rows; y++ )
	{ 
		for( int x = 0; x < image.cols; x++ )
		{ 
			for( int c = 0; c < 3; c++ )
			{
				new_image.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( alpha*( image.at<Vec3b>(y,x)[c] ) + beta );
			}
		}
	}

	/// Create Windows
	namedWindow("Original Image", 1);
	namedWindow("New Image", 1);

	/// Show stuff
	imshow("Original Image", image);
	imshow("New Image", new_image);

	/// Wait until user press some key
	waitKey();

}

}

void blend2Pictures(string fileName)
{
	// Note: The explanation below belongs to the book Computer Vision: Algorithms and Applications by Richard Szeliski
	double alpha = 0.5; double beta; double input;
	Mat src1, src2, dst;
	/// Ask the user enter alpha
	std::cout<<" Simple Linear Blender "<<std::endl;
	std::cout<<"-----------------------"<<std::endl;
	std::cout<<"* Enter alpha [0-1]: ";
	std::cin>>input;
	/// We use the alpha provided by the user if it is between 0 and 1
	if( input >= 0.0 && input <= 1.0 )
	{ 
		alpha = input; 
	}
	/// Read image ( same size, same type )
	src1 = imread("../../images/LinuxLogo.jpg");
	src2 = imread("../../images/WindowsLogo.jpg");
	if( !src1.data ) { printf("Error loading src1 \n"); return -1; }
	if( !src2.data ) { printf("Error loading src2 \n"); return -1; }
	/// Create Windows
	namedWindow("Linear Blend", 1);
	beta = ( 1.0 - alpha );
	addWeighted( src1, alpha, src2, beta, 0.0, dst);
	imshow( "Linear Blend", dst );
	waitKey(0);

}


void Sharpen(const Mat& myImage, Mat& Result)
{
	CV_Assert(myImage.depth() == CV_8U); // accept only uchar images
	Result.create(myImage.size(), myImage.type());
	const int nChannels = myImage.channels();
	for(int j = 1; j < myImage.rows - 1; ++j)
	{
		const uchar* previous = myImage.ptr<uchar>(j - 1);
		const uchar* current = myImage.ptr<uchar>(j );
		const uchar* next = myImage.ptr<uchar>(j + 1);
		uchar* output = Result.ptr<uchar>(j);
		for(int i = nChannels; i < nChannels * (myImage.cols - 1); ++i)
		{
			*output++ = saturate_cast<uchar>(5 * current[i]
			-current[i - nChannels] - current[i + nChannels] - previous[i] - next[i]);
		}
	}
	Result.row(0).setTo(Scalar(0));
	Result.row(Result.rows - 1).setTo(Scalar(0));
	Result.col(0).setTo(Scalar(0));
	Result.col(Result.cols - 1).setTo(Scalar(0));

}

void modifyImage(string fileName)
{

	Mat image;
	image = imread(fileName, 1);

	Mat gray_image;
	cvtColor(image, gray_image, CV_BGR2GRAY);

	imwrite("", gray_image);

	namedWindow(fileName, CV_WINDOW_AUTOSIZE);
	namedWindow("Gray Image", CV_WINDOW_AUTOSIZE);

	imshow(fileName, image);
	imshow("Gray Image", gray_image);

	waitKey(0);


}


void cannyEdgeDetector(string fileName)
{
	Mat input;
	input = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	cout << "Detecting edges in input image" << endl;

	Mat edges;
	Canny(input, edges, 10, 100);
	

}

void showImage(string fileName)
{
	// CV_LOAD_IMAGE_UNCHANGED
	// CV_LOAD_IMAGE_GRAYSCALE
	// CV_LOAD_IMAGE_COLOR

	Mat image;
	image = imread(argv[1], 1);

	if(!image.data)
	{
		printf("No image data\n");
		return -1;
	}

	namedWindow("Showing Image...", WINDOW_AUTOSIZE);
	imshow("Showing Image, image);

	waitKey(0);

	return 0;

}



int main(int argc, char** argv )
{

	string fileName = argc;

	return 0;
}