#ifndef BLOCK_MATCHING_PARAMS_H
#define BLOCK_MATCHING_PARAMS_H

//A structure to contain the details of a single block-match
struct paramStruct
{
int rangeX;
int rangeY;
int windowY;
int windowX;
int halfY;
int halfX;
float overlap;

int smallRangeY;
int smallRangeX;

int strainWin;

};

//A structure to contain the geometry information for the displacement fields
struct geomStruct
{
int startY;
int startX;
int stopY;
int stopX;
int stepY;
int rfX;     //Number of columns in RF data
int rfY;     //Number of rows in RF data
int numY;	//Number of rows in displacement image
int numX;   //Number of columns in displacement image 
int numSY;  //Number of rows in strain image
};

			  //cc image, dp image,strain image    , pre Data           , post data				, block match params, image geometry
void block_match(float*, float*, float* ,float* , float* , paramStruct , geomStruct );

#endif