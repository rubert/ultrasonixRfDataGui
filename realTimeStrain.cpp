//This is a simple function that loads two files, performs a straight-forward block matching
//then exits.  It's to get an idea what the speed will be like.

//The input files are the same size, and the data in the two input images is stored with this numbering
//0	4	8
//1	5	9
//2	6	10
//3	7	11
//The input files have two numbers for headers which tell number of rows, and number of columns
//all numbers in binaries are ints

//The output displacement field entries are stored in C order
//0	1	2	3
//4	5	6	7
//8	9	10	11


//block matching computes 2-D SAD at each location with full search range
//uses parabolic fit to do sub-sample interpolation
//the block matching templates are not searched for outside of the post image
//Therefore Displacement field is necessarily smaller than input matrices
/*#include <iostream>
#include <fstream>
#include <cstdlib>
#include <climits>

//my own code
#include "block_matching_params.h"

//STL container code
#include <queue>

//openCV library headers
#include "cv.h"
#include "cxcore.h"

#include <ctime>


using namespace std;

//This is a structure which will keep track of the quality of a block match.  Index will give the index in
//the displacement image
struct seedStruct 
{
	float metric;	//Cross correlation value
	int index;		//index of displacement point
	int dpY;		//initial displacements
	int dpX;
	int region;    //Index telling which seed the point was grown from
	
};


//The priority queue is a point set as discussed by Chen
typedef priority_queue< seedStruct, vector<seedStruct>, less<seedStruct> >  p_queue;


bool operator < (const seedStruct& v1, const seedStruct& v2)
{
            return (v1.metric < v2.metric);
};

bool operator > (const seedStruct& v1, const seedStruct& v2)
{
            return (v1.metric > v2.metric);
} ;




void addNeighbors(bool *, seedStruct, p_queue * , int , int );






void block_match(float* ccMax, float* dpY, float* strainImage,float* floatPreArray, float* floatPostArray, paramStruct params, geomStruct geom)
{

	
	//Create priority queue for keeping track of quality of block matches
	//Putting in the less than operator in the class declaration
	//will cause the top member function to give the point
	//with the maximum quality.  Would need to use greater if I were using SSD/SAD
	
	
	p_queue* pointSet = new p_queue;   
	
	int rows = geom.rfY;
	int cols = geom.rfX;
	
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	/////////////////Parameters									////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	

	//Get block-matching parameters
	int winY = params.windowY;
	int	winX = params.windowX;

	int rangeY = params.rangeY;
	int	rangeX = params.rangeX;
	int halfY = params.halfY;
	int halfX = params.halfX;
	int stepY = geom.stepY;
	
	int smallRangeY = params.smallRangeY;
	int smallRangeX = params.smallRangeX;
	
	//bounds on X displacement
	int startX = geom.startX;
	int stopX = geom.stopX;
	int numX = geom.numX;
	
	//bounds on blocks in Y direction
	int startY = geom.startY;
	int numY = geom.numY;
	int stopY = geom.stopY;
	
	float  *dpX;

	dpX = new float[numX*numY];
	bool* processed = new bool[numX*numY];
	
	
	for(int ind = 0; ind < numX*numY; ind++)
		processed[ind] = false;
		
		
	
	//Seed parameters
	int numSeedsX = numX/40, numSeedsY = numY/20;
	int seedStepY = numY/numSeedsY, seedStepX = numX/numSeedsX ;
	
	int* regionCount = new int[numSeedsY*numSeedsX];
	int* seedImage = new int[numY*numX];

	for( int dummy = 0; dummy < numSeedsY*numSeedsX; dummy++)
		regionCount[dummy] = 0;
	
	int threshold = 50;
	
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	/////////////////First the tracking to obtain the seed points////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	////////////////											 ////////////////////////
	//     						  //
	//initialize openCV structures//
	//							  //
	/*
	Important to note that the iplImage data structure stores its image data in memory in C order, not fortran/matlab order
	in memory an image is stored like:
	
	0   1   2   3   4
	5   6   7   8   9
	10  11  12  13  14
	
	*/ /*
	float* cvImP;
	float* cvTpP;
	float* cvResP;
	
	int cvImSizeX =winX + 2*rangeX;
	int cvImSizeY = winY + 2*rangeY;
	
	int resHeight = 2*rangeY + 1;
	int resWidth = 2*rangeX + 1;
	
	seedStruct tempQuality;
	
	
	IplImage* cvIm = cvCreateImage(cvSize( cvImSizeX, cvImSizeY), IPL_DEPTH_32F, 1);
	IplImage* cvTp = cvCreateImage(cvSize(winX, winY ), IPL_DEPTH_32F, 1);
	IplImage* cvRes = cvCreateImage(cvSize(2*rangeX + 1, 2*rangeY + 1), IPL_DEPTH_32F, 1);

	cvImP = (float*)cvIm->imageData;
	cvTpP = (float*)cvTp->imageData;
	cvResP = (float*)cvRes->imageData;

	CvPoint maxLocTemp;

	
	int yRf, xRf, pointInd;
	double tempCCMax;
	float deltaY, deltaX, a, b, c;
	
	
	
	int xx, yy, dummyX, dummY, seedIdx;
	
	seedIdx = 0;
	
	for(int dummyX = 0; dummyX < numSeedsX; dummyX++){
		for(int dummyY = 0; dummyY < numSeedsY; dummyY++)
	{
			xx = dummyX*seedStepX;
			yy = dummyY*seedStepY;
			pointInd = yy + xx*numY;
			
		
			
			//Transfer data into CV Image classes
			for(int y = 0; y < winY ; y++ ) {
				for(int x = 0; x < winX ; x++) {
						yRf = startY + yy*stepY - halfY  + y;
						xRf = startX  + xx  - halfX + x;
						cvTpP[ x + y*winX] = floatPreArray[  yRf + xRf*rows];
					}
	
				}
			
			for(int y = 0; y < cvImSizeY ; y++ ) {
				for(int x = 0; x < cvImSizeX ; x++) {
							yRf = startY + yy*stepY - halfY - rangeY + y;
							xRf = startX  + xx - halfX - rangeX + x;
							cvImP[ x + y*cvImSizeX] =  floatPostArray[ yRf + xRf*rows];
						}
					}  
			
			
			//cross-correlation, find maximum
			cvMatchTemplate(cvIm, cvTp, cvRes, CV_TM_CCOEFF_NORMED);
			cvMinMaxLoc(cvRes, NULL, &tempCCMax, NULL, &maxLocTemp, NULL);
			
			
			//Now for the subsample fit in x
			if( maxLocTemp.x > 0 && maxLocTemp.x < resWidth){
			c = (float) tempCCMax;
			a = (cvResP[maxLocTemp.x-1 + maxLocTemp.y*resWidth] + cvResP[maxLocTemp.x +1 + maxLocTemp.y*resWidth]   )/2 - c;
			b = (-cvResP[maxLocTemp.x-1 + maxLocTemp.y*resWidth] + cvResP[maxLocTemp.x +1 + maxLocTemp.y*resWidth]   )/2;
			deltaX = -b/(2*a);
			}
			else
			deltaX = 0.0;
			
			//subsample fit in y
			if( maxLocTemp.y > 0 && maxLocTemp.y < resHeight){
			c = (float) tempCCMax;
			a = (cvResP[maxLocTemp.x + (maxLocTemp.y-1)*resWidth] + cvResP[maxLocTemp.x + (maxLocTemp.y+1)*resWidth]   )/2 - c;
			b = (-cvResP[maxLocTemp.x + (maxLocTemp.y-1)*resWidth] + cvResP[maxLocTemp.x + (maxLocTemp.y+1)*resWidth]   )/2;
			deltaY = -b/(2*a);
			}
			else
			deltaY = 0.0;
			
			
			//Add to point set, perform bounds checking
			dpY[yy + xx*numY] = (float)maxLocTemp.y  - rangeY + deltaY;
			dpX[yy + xx*numY] = (float)maxLocTemp.x - rangeX + deltaX;	
			ccMax[yy + xx*numY] = tempCCMax; 
			
			tempQuality.metric = tempCCMax;
			tempQuality.index = pointInd;
			tempQuality.dpY = maxLocTemp.y  - rangeY;
			tempQuality.dpX = maxLocTemp.x - rangeX;
			tempQuality.region = seedIdx;
				
			if(tempQuality.dpY > rangeY - smallRangeY)
				tempQuality.dpY = rangeY - smallRangeY;
				
			if(tempQuality.dpY < -(rangeY - smallRangeY) )
				tempQuality.dpY = -(rangeY - smallRangeY); 
			
				if(tempQuality.dpX >  rangeX - smallRangeX )
				tempQuality.dpX = rangeX - smallRangeX;
				
			if(tempQuality.dpX <  -(rangeX - smallRangeX) )
				tempQuality.dpX = -(rangeX - smallRangeX); 
			
			
			processed[yy + xx*numY] = true;
			addNeighbors(processed, tempQuality, pointSet, numY, numX); 
			
			seedImage[yy + xx*numY] = seedIdx;
			
			regionCount[seedIdx]++;
			
			seedIdx++;
		}
	}

	cvReleaseImage(&cvIm);
	cvReleaseImage(&cvTp);
	cvReleaseImage(&cvRes); 

	
	
	////////////////							 ///////////////////////////
	////////////////							 ///////////////////////////
	////////////////							 ///////////////////////////
	////////////////							 ///////////////////////////
	/////////////////Now for all the other points///////////////////////////
	////////////////							 ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////

	cvImSizeX = winX + 2*smallRangeX;
	cvImSizeY = winY + 2*smallRangeY;
	
	resHeight = 2*smallRangeY + 1;
	resWidth = 2*smallRangeX + 1;
	
	cvIm = cvCreateImage(cvSize( cvImSizeX, cvImSizeY), IPL_DEPTH_32F, 1);
	cvTp = cvCreateImage(cvSize(winX, winY ), IPL_DEPTH_32F, 1);
	cvRes = cvCreateImage(cvSize(resWidth, resHeight), IPL_DEPTH_32F, 1);

	cvImP = (float*)cvIm->imageData;
	cvTpP = (float*)cvTp->imageData;
	cvResP = (float*)cvRes->imageData;

	int iniDpY, iniDpX;

	while( pointSet->size() != 0)
	{
			//Get maximum quality point from point set
			tempQuality = pointSet->top();
			pointInd = tempQuality.index;
			xx = pointInd/numY;
			yy = pointInd -xx*numY;
			iniDpY = tempQuality.dpY;
			iniDpX = tempQuality.dpX;
	
			
			
			
			
			//remove point from point set
			pointSet->pop();
				
			//Transfer data into CV Image classes
			for(int y = 0; y < winY ; y++ ) {
				for(int x = 0; x < winX ; x++) {
						yRf = startY + yy*stepY - halfY  + y;
						xRf = startX  + xx  - halfX + x;
						cvTpP[ x + y*winX] = floatPreArray[  yRf + xRf*rows];
					}
	
				}
	
			
			for(int y = 0; y < cvImSizeY ; y++ ) {
				for(int x = 0; x < cvImSizeX ; x++) {
							yRf = startY + yy*stepY - halfY - smallRangeY + y + iniDpY;
							xRf = startX  + xx - halfX - smallRangeX + x + iniDpX;
							cvImP[ x + y*cvImSizeX] = floatPostArray[ yRf + xRf*rows];
						}
					}  
			
			//Calculat cross-correlation, find maximum
			cvMatchTemplate(cvIm, cvTp, cvRes, CV_TM_CCOEFF_NORMED);
			cvMinMaxLoc(cvRes, NULL, &tempCCMax, NULL, &maxLocTemp, NULL);
			
			
			//Now for the subsample fit in x
			if( maxLocTemp.x > 0 && maxLocTemp.x < resWidth){
			c = (float) tempCCMax;
			a = (cvResP[maxLocTemp.x-1 + maxLocTemp.y*resWidth] + cvResP[maxLocTemp.x +1 + maxLocTemp.y*resWidth]   )/2 - c;
			b = (-cvResP[maxLocTemp.x-1 + maxLocTemp.y*resWidth] + cvResP[maxLocTemp.x +1 + maxLocTemp.y*resWidth]   )/2;
			deltaX = -b/(2*a);
			}
			else
			deltaX = 0.0;
			
			//subsample fit in y
			if( maxLocTemp.y > 0 && maxLocTemp.y < resHeight){
			c = (float) tempCCMax;
			a = (cvResP[maxLocTemp.x + (maxLocTemp.y-1)*resWidth] + cvResP[maxLocTemp.x + (maxLocTemp.y+1)*resWidth]   )/2 - c;
			b = (-cvResP[maxLocTemp.x + (maxLocTemp.y-1)*resWidth] + cvResP[maxLocTemp.x + (maxLocTemp.y+1)*resWidth]   )/2;
			deltaY = -b/(2*a);
			}
			else
			deltaY = 0.0;
			
			
			
			dpY[yy + xx*numY] = (float)maxLocTemp.y  - smallRangeY + deltaY + iniDpY;
			dpX[yy + xx*numY] = (float)maxLocTemp.x - smallRangeX + deltaX + iniDpX;	
			ccMax[yy + xx*numY] = tempCCMax; 
			
			
			
			
			//Add to point set, perform bounds checking
			tempQuality.metric = tempCCMax;
			tempQuality.index = pointInd;
			tempQuality.dpY = maxLocTemp.y  - smallRangeY + iniDpY;
			tempQuality.dpX = maxLocTemp.x - smallRangeX + iniDpX;
			
			if(tempQuality.dpY > rangeY - smallRangeY)
				tempQuality.dpY = rangeY - smallRangeY;
				
			if(tempQuality.dpY < -(rangeY - smallRangeY) )
				tempQuality.dpY = -(rangeY - smallRangeY);
				
				if(tempQuality.dpX >  rangeX - smallRangeX )
				tempQuality.dpX = rangeX - smallRangeX;
				
			if(tempQuality.dpX <  -(rangeX - smallRangeX) )
				tempQuality.dpX = -(rangeX - smallRangeX); 
			
			

			addNeighbors( processed, tempQuality, pointSet, numY, numX); 
			
			seedImage[yy + xx*numY] = tempQuality.region;
			regionCount[tempQuality.region]++;
			
	}

		
	
	
	////////////////							 ///////////////////////////
	////////////////							 ///////////////////////////
	////////////////							 ///////////////////////////
	////////////////							 ///////////////////////////
	/////////////////Now for drop out correction ///////////////////////////
	////////////////							 ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	////////////////                             ///////////////////////////
	
	
	

	//Go through regions below threshold and set all those points processed variable to false
	
	for(int r = 0; r < numSeedsY*numSeedsX; r++ ){
	
		if(regionCount[r] < threshold ){
	
		for(int xx = 0; xx < numX; xx++){
			for(int yy = 0; yy < numY; yy++){
				if(seedImage[yy + xx*numY] == r)
				processed[yy + xx*numY] = false;
	
			} 
		} 
		
		}
	}		
	
	
	
	
	//Now repropagate displacements
	//Add the neighbors of points that were good to the point set
	for(xx = 0; xx < numX; xx++){
		for(yy = 0; yy < numY; yy++){
		
			if( processed[yy + xx*numY] ){
			
		
			
			tempQuality.metric = ccMax[yy + xx*numY];
			tempQuality.index = yy + xx*numY;
			tempQuality.dpY = (int) dpY[yy + xx*numY];
			tempQuality.dpX = (int) dpX[yy + xx*numY];
			
			if(tempQuality.dpY > rangeY - smallRangeY)
				tempQuality.dpY = rangeY - smallRangeY;
				
			if(tempQuality.dpY < -(rangeY - smallRangeY) )
				tempQuality.dpY = -(rangeY - smallRangeY);
				
			if(tempQuality.dpX >  rangeX - smallRangeX )
				tempQuality.dpX = rangeX - smallRangeX;
				
			if(tempQuality.dpX <  -(rangeX - smallRangeX) )
				tempQuality.dpX = -(rangeX - smallRangeX);
			
			addNeighbors( processed, tempQuality, pointSet, numY, numX); 
			
			}
		}
	}

	
	
	//Now perform tracking again on bad points
	
	while( pointSet->size() != 0)
	{
			//Get maximum quality point from point set
			tempQuality = pointSet->top();
			pointInd = tempQuality.index;
			xx = pointInd/numY;
			yy = pointInd -xx*numY;
			iniDpY = tempQuality.dpY;
			iniDpX = tempQuality.dpX;
	
			
			
			
			
			//remove point from point set
			pointSet->pop();
				
			//Transfer data into CV Image classes
			for(int y = 0; y < winY ; y++ ) {
				for(int x = 0; x < winX ; x++) {
						yRf = startY + yy*stepY - halfY  + y;
						xRf = startX  + xx  - halfX + x;
						cvTpP[ x + y*winX] = floatPreArray[  yRf + xRf*rows];
					}
	
				}
	
			
			for(int y = 0; y < cvImSizeY ; y++ ) {
				for(int x = 0; x < cvImSizeX ; x++) {
							yRf = startY + yy*stepY - halfY - smallRangeY + y + iniDpY;
							xRf = startX  + xx - halfX - smallRangeX + x + iniDpX;
							cvImP[ x + y*cvImSizeX] = floatPostArray[ yRf + xRf*rows];
						}
					}  
			
			//Calculat cross-correlation, find maximum
			cvMatchTemplate(cvIm, cvTp, cvRes, CV_TM_CCOEFF_NORMED);
			cvMinMaxLoc(cvRes, NULL, &tempCCMax, NULL, &maxLocTemp, NULL);
			
			
			//Now for the subsample fit in x
			if( maxLocTemp.x > 0 && maxLocTemp.x < resWidth){
			c = (float) tempCCMax;
			a = (cvResP[maxLocTemp.x-1 + maxLocTemp.y*resWidth] + cvResP[maxLocTemp.x +1 + maxLocTemp.y*resWidth]   )/2 - c;
			b = (-cvResP[maxLocTemp.x-1 + maxLocTemp.y*resWidth] + cvResP[maxLocTemp.x +1 + maxLocTemp.y*resWidth]   )/2;
			deltaX = -b/(2*a);
			}
			else
			deltaX = 0.0;
			
			//subsample fit in y
			if( maxLocTemp.y > 0 && maxLocTemp.y < resHeight){
			c = (float) tempCCMax;
			a = (cvResP[maxLocTemp.x + (maxLocTemp.y-1)*resWidth] + cvResP[maxLocTemp.x + (maxLocTemp.y+1)*resWidth]   )/2 - c;
			b = (-cvResP[maxLocTemp.x + (maxLocTemp.y-1)*resWidth] + cvResP[maxLocTemp.x + (maxLocTemp.y+1)*resWidth]   )/2;
			deltaY = -b/(2*a);
			}
			else
			deltaY = 0.0;
			
			
			
			dpY[yy + xx*numY] = (float)maxLocTemp.y  - smallRangeY + deltaY + iniDpY;
			dpX[yy + xx*numY] = (float)maxLocTemp.x - smallRangeX + deltaX + iniDpX;	
			ccMax[yy + xx*numY] = tempCCMax; 
			
			
			
			
			//Add to point set, perform bounds checking
			tempQuality.metric = tempCCMax;
			tempQuality.index = pointInd;
			tempQuality.dpY = maxLocTemp.y  - smallRangeY + iniDpY;
			tempQuality.dpX = maxLocTemp.x - smallRangeX + iniDpX;
			
			if(tempQuality.dpY > rangeY - smallRangeY)
				tempQuality.dpY = rangeY - smallRangeY;
				
			if(tempQuality.dpY < -(rangeY - smallRangeY) )
				tempQuality.dpY = -(rangeY - smallRangeY);
				
			if(tempQuality.dpX >  rangeX - smallRangeX )
				tempQuality.dpX = rangeX - smallRangeX;
				
			if(tempQuality.dpX <  -(rangeX - smallRangeX) )
				tempQuality.dpX = -(rangeX - smallRangeX); 
	

			addNeighbors( processed, tempQuality, pointSet, numY, numX);  
			
	} 

	
	

	
	cvReleaseImage(&cvIm);
	cvReleaseImage(&cvTp);
	cvReleaseImage(&cvRes); 
	
	



	

	////////////////							 /////////////////////
	////////////////							 /////////////////////
	////////////////							 /////////////////////
	////////////////							 /////////////////////
	/////////////////calculate a strain image	 /////////////////////
	////////////////							 /////////////////////
	////////////////                             /////////////////////
	////////////////                             /////////////////////
	////////////////                             /////////////////////
	////////////////                             /////////////////////
	////////////////                             /////////////////////
	

	int strainWin = params.strainWin;
	int halfWin = (strainWin-1)/2;

	
	int numSy = numY - strainWin;

	

	for(int sx = 0; sx < numX; sx++ )
	{
		for(int sy = 0; sy < numSy; sy++)
			{

				strainImage[sy + sx*numSy] = (1./(float)(strainWin*stepY))*(dpY[sy + sx*numY ] - dpY[(sy + strainWin ) +sx*numY ] );
	 		}
	}		

	/*for(int yy = 0; yy < numY; yy++){
		for(int xx = 0; xx < numX; xx++){
			dpY[yy + xx*numY] = yy + xx*numY;
		}
	} */
	
	
	//clear arrays
//	delete dpX;
/*
} 





void addNeighbors(bool *processed, seedStruct quality, p_queue *pointSet , int rows, int cols )
{

	int tempInd = quality.index;
	int xInd = quality.index/rows;
	int yInd = quality.index - xInd*rows;
	static int callNo = 0;
	
	
	
	
		
	
	//look for out of bounds indexes, then check to see if the point has already been processed
	//passing this criteria, add to point set, and mark as processed
	//
	
	//bottom
	if( yInd + 1 <= (rows - 1) ){
		
		if(!processed[tempInd + 1]  ){
			quality.index = tempInd + 1;
			processed[quality.index] = true;
			pointSet->push(quality);
		}
		
	}
		

	//top
	if( yInd - 1 >= 0  ){
			
			if(!processed[tempInd - 1]  ){
				quality.index = tempInd - 1;
				processed[quality.index] = true;
				pointSet->push(quality);
			}
			
		}

	//right
	if( xInd  + 1 <= (cols-1) ){
		
		if(!processed[tempInd + rows]  ){
				quality.index = tempInd + rows;
				processed[quality.index] = true;
				pointSet->push(quality);
			}
	}
	
	
	//left	
	if( xInd - 1 >= 0 ){
		if(!processed[tempInd - rows]  ){
				quality.index = tempInd -rows;
				processed[quality.index] = true;
				pointSet->push(quality);
			}
	}
		
		
	
	
	


		
}*/