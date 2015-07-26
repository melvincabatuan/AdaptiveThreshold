#include "com_cabatuan_adaptivethreshold_MainActivity.h"
#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

#define  LOG_TAG    "AdaptiveThreshold"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


 
// src - input gray image
// dst - output binary image

void myAdaptiveThreshold(const cv::Mat& src, cv::Mat& dst, int threshold = 5) {

    int blockSize= 15; // size of the neighborhood
    dst = src.clone();

	int nl= src.rows;  
	int nc= src.cols;  
              
	// compute integral image
	cv::Mat integralImg;
	cv::integral(src, integralImg, CV_32S);

	// for each row
	int halfSize= blockSize/2;
    for (int j = halfSize; j < nl-halfSize-1; j++) { // blocksize is a regular n x n block

		  // get the address of row j
		  uchar* data= dst.ptr<uchar>(j);
		  int* idata1= integralImg.ptr<int>(j-halfSize);
		  int* idata2= integralImg.ptr<int>(j+halfSize+1);

          for (int i = halfSize; i < nc - halfSize - 1; i++) {
 
			  // compute mean
			  int mean = (idata2[i+halfSize+1] - idata2[i-halfSize] -
				        idata1[i+halfSize+1] + idata1[i-halfSize])/(blockSize * blockSize);

			  // apply adaptive threshold
			  if (data[i] < (mean - threshold))
				  data[i]= 0;
				  
			  else
				  data[i]=255;
         }                    
    }
}





int64 t;
Mat temp;

/*
 * Class:     com_cabatuan_adaptivethreshold_MainActivity
 * Method:    process
 * Signature: (Landroid/graphics/Bitmap;[B)V
 */
JNIEXPORT void JNICALL Java_com_cabatuan_adaptivethreshold_MainActivity_process
  (JNIEnv *pEnv, jobject clazz, jobject pTarget, jbyteArray pSource, jint threshold){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent; // Links to Bitmap content

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data... OK
   jbyte* source = (jbyte*)pEnv->GetPrimitiveArrayCritical(pSource, 0);
   if (source == NULL) abort();

   /// cv::Mat for YUV420sp source and output BGRA 
    Mat src(bitmapInfo.height, bitmapInfo.width, CV_8UC1, (unsigned char *)source);
    Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *)bitmapContent);
    

/***********************************************************************************************/
    /// Native Image Processing HERE... 
    if(temp.empty())
       temp = Mat(bitmapInfo.height, bitmapInfo.width, CV_8UC1);
      
      
      
    t = cv::getTickCount();   
    
    myAdaptiveThreshold(src, temp, threshold);  
    
    LOGI("myAdaptiveThreshold() took %0.2f ms.", 1000*(cv::getTickCount() - t)/(float)cv::getTickFrequency());
    
    
    
    
    cvtColor(temp, mbgra, CV_GRAY2BGRA);
 
/************************************************************************************************/ 
   
   /// Release Java byte buffer and unlock backing bitmap
   pEnv-> ReleasePrimitiveArrayCritical(pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}
