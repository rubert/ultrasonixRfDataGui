#include <QtGui>
#include <QHBoxLayout>
#include "dialog.h"
#include <windows.h> // for Sleep
#include <iostream>
#include <random>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>

 Dialog::Dialog(QWidget *parent)
     : QDialog(parent)
 {
	 
	 displayW = 480;
	 displayH = 360;
	 statusBox = new QTextEdit;
	 statusBox->setReadOnly(true);
	//Initialize Porta
	  m_porta = new porta;
	  centralAngle =37.5;  //To home the motor go here:  The probes.xml file contains the motor FOV info
	 
	  numLines = 128;

	  int  sel, code;
      char str[1024];
	  if(m_porta->init(CINE_SIZE, FIRMWARE_PATH, SETTINGS_PATH, LICENSE_PATH, LUT_PATH, 3, 3, 0, 0, 64) )
	    statusBox->setText("Porta successfully Initialized");

	  code = m_porta->getProbeID(0);
      m_porta->selectProbe(code);
	  m_porta->activateProbeConnector(0);
      m_porta->findMasterPreset(str, 1024, code);
      m_porta->loadPreset(str);
	  m_porta->initImagingMode((imagingMode)RfMode); 
	  m_porta->setParam(prmRfMode, 1);
	  m_porta->setParam(prmRfDecim, 0);
	  m_porta->setParam(prmMotorStatus, 0);
	  m_porta->setParam(prmBLineDensity, numLines);  // try 128 and 256, may need 128 to achieve sufficient frame rate
	  m_porta->setParam(prmBTxFreq, 4500000);  //Units are hz
	  m_porta->setParam(prmBImageDepth, 100);  //Units are mm
	  m_porta->setParam(prmBFocusDepth, 70000);  //units are microns
	  m_porta->setParam("b-pulse repeat", 1);
	
	  
	 //turn on the imaging so I can get the parameters I need


	//initialize linear array data collection
	rfInPlaneButton = new QPushButton(tr("Collect RF in plane") );
		  
	 //Volume Acquisition Controls initialization
     QPushButton *rfVolumeButton = new QPushButton(tr("Collect RF data Volume") );
	 
	 
	 ///////////File Naming
	 fileNameBox = new QLineEdit;
	 fNameLabel = new QLabel;
	 fNameLabel->setText("RF data file Name: " );
	 fNameString = "D:";
	 fNameString += '/';
	 fNameString += "defaultName.rf";
	 fileNameBox->insert(fNameString);

	 //////							   //////
	 //////							   //////
	 //////B-mode image initialization //////
	 //////							   //////
	 //////							   //////
     bModeOnButton = new QPushButton(tr("Start B-mode Imaging") );
     bModeOffButton = new QPushButton(tr("Stop B-mode Imaging") );
	 bModeDisplay = new QLabel;
	 bModeTimer = new QTimer;
	 volumeRefreshTimer = new QTimer;
	 strainTimer = new QTimer;
	 bModeImage = new QImage(displayW,displayH, QImage::Format_Indexed8);
	 bModeImage->setNumColors(256);
	 //Assign an RGB value to the 256 indexes in the image color table
	 for ( int i = 0; i < 256; i++ ) 
	 {
	bModeImage->setColor( i, qRgb( i, i, i )) ;
	}
	bModeImage->fill(100);
	QPixmap bModePix = QPixmap::fromImage(*bModeImage);
	bModeDisplay->setPixmap(bModePix); 

	int szFrm = (displayW * displayH * (8 / 8));   //(16 bit ints/8 bits per char)*10 frames per angle
	buffer = new unsigned char[szFrm];
	rfBuffer = NULL; //Initialize RF buffer to NULL

	//////							 //////
	//////							 //////
	/////							 /////
	/////Strain Image initialization /////
	/////							 /////
	//////							//////
	//////							//////
	/*int w = 200;//m_porta->getParam(prmRfNumLines);
    int h = 200;//m_porta->getParam(prmRfNumSamples);

	//Adjust block matching parameters
	blockMatchParams.windowY = 81;
	blockMatchParams.windowX = 7;
	blockMatchParams.overlap = .75;
	blockMatchParams.halfY = (81 - 1)/2;
	blockMatchParams.halfX = (7 - 1)/2;
	blockMatchParams.rangeY = 45;
	blockMatchParams.rangeX = 7;
	blockMatchParams.smallRangeY = 3;
	blockMatchParams.smallRangeX = 1;
	blockMatchParams.strainWin = 13;


	//Work out displacement image geometry info
	//bounds on X displacement
	geom.startX = blockMatchParams.halfX + blockMatchParams.rangeX;
	geom.stopX = (w - 1) - blockMatchParams.halfX - blockMatchParams.rangeX;
	geom.numX = geom.stopX - geom.startX + 1;
	
	//bounds on blocks in Y direction
	geom.stepY =  ( (float)1 - blockMatchParams.overlap )*blockMatchParams.windowY;

	geom.startY = blockMatchParams.halfY + blockMatchParams.rangeY;
	int maxY = (h - 1) - blockMatchParams.rangeY - blockMatchParams.halfY;
	geom.numY = (maxY - geom.startY)/geom.stepY;
	geom.stopY = geom.startY + (geom.numY-1)*geom.stepY;

	geom.numSY = geom.numY - blockMatchParams.strainWin;

	//Initialize array to hold strain image
	strainImagePtr = new float[geom.numSY*geom.numX];

	//scanConvert = new NeHeChapter2;
	strainOnButton = new QPushButton(tr("Start Strain Imaging") );
	strainOffButton = new QPushButton(tr("Stop Strain Imaging") );
	strainDisplay = new QLabel;
	strainImage = new QImage(geom.numX,geom.numSY, QImage::Format_Indexed8);
	strainImage->setNumColors(256);
	 //Assign an RGB value to the 256 indexes in the image color table
	 for ( int i = 0; i < 256; i++ ) 
	 {
	strainImage->setColor( i, qRgb( i, i, i )) ;
	}
	strainImage->fill(100);
	QPixmap strainPix = QPixmap::fromImage(*strainImage);
	strainDisplay->setPixmap(strainPix); */ 
	
	/////											/////
	/////											/////
	/////Image/Focus Depth and angle Initialization /////
	/////											/////
	/////											/////
	/////											/////
    depthLabel = new QLabel;
    depthLabel->setText("Imaging Depth (mm)");
	QComboBox *imageDepthBox = new QComboBox;
	imageDepthBox->insertItem(0,"20");
	imageDepthBox->insertItem(1,"30");
	imageDepthBox->insertItem(2,"40");
	imageDepthBox->insertItem(3,"50");
	imageDepthBox->insertItem(4,"60");
	imageDepthBox->insertItem(5,"70");
	imageDepthBox->insertItem(6,"80");
	imageDepthBox->insertItem(7,"90");
	imageDepthBox->insertItem(8,"100");
	imageDepthBox->insertItem(9, "110");
	imageDepthBox->insertItem(10,"120");
	imageDepthBox->setCurrentIndex(8);

	//Focus Depth initialization
	focusLabel = new QLabel;
	focusLabel->setText("Focal Depth (mm)");
	QComboBox *focusDepthBox = new QComboBox;
	focusDepthBox ->insertItem(0,"30");
	focusDepthBox ->insertItem(1,"40");
	focusDepthBox ->insertItem(2,"50");
	focusDepthBox ->insertItem(3,"60");
	focusDepthBox ->insertItem(4,"70");
	focusDepthBox ->insertItem(5,"80");
	focusDepthBox ->insertItem(6,"90");
	focusDepthBox->setCurrentIndex(4);


	//Go to angle box
	angleLabel = new QLabel;
	angleLabel->setText("Wobbler Position (Degrees)");
	QComboBox *angleBox = new QComboBox;
	angleBox ->insertItem(0,"-10");
	angleBox ->insertItem(1,"-5");
	angleBox ->insertItem(2,"0");
	angleBox ->insertItem(3,"5");
	angleBox ->insertItem(4,"10");
	angleBox->setCurrentIndex(2);

	/////							/////
	/////							/////
	///// Signals/Slots  and layout	/////
	/////							/////
	/////							/////

    connect(rfVolumeButton, SIGNAL(clicked() ), this, SLOT(collectRfVolume() ) ) ;
	connect(volumeRefreshTimer, SIGNAL( timeout() ), this, SLOT( acquireAngleInRfVolume() ) );
	connect(this, SIGNAL( volumeAcquisitionComplete() ), this, SLOT( writeRfVolume() ) );
	connect(this, SIGNAL( updateImageWhileVolumeAcquisition() ), this, SLOT( repaint() ) );


	connect(bModeOnButton, SIGNAL(clicked() ), this, SLOT(stopStrainTimer() ) );
	connect(bModeOnButton, SIGNAL(clicked() ), this, SLOT(startBModeTimer() ) );
	connect(bModeOffButton, SIGNAL(clicked() ), this, SLOT(stopBModeTimer() ) );
	connect(bModeTimer, SIGNAL(timeout()), this, SLOT(updateBModeImage() ) ) ;
	
	connect(imageDepthBox, SIGNAL(currentIndexChanged(int) ), this, SLOT( changeImageDepth(int) ) );
	connect(focusDepthBox, SIGNAL(currentIndexChanged(int) ), this, SLOT( changeFocusDepth(int) ) );
	connect(angleBox, SIGNAL(currentIndexChanged(int) ), this, SLOT(changeStaticAngle(int) ) );

/*	connect(strainOnButton, SIGNAL(clicked() ), this, SLOT(stopBModeTimer() ) ); //stop B-mode imaging
	connect(strainOnButton, SIGNAL(clicked() ), this, SLOT(startStrainTimer() ) ); //begin strain imaging
	connect(strainOffButton, SIGNAL(clicked() ), this, SLOT(stopStrainTimer() ) );
	connect(strainTimer, SIGNAL(timeout()), this, SLOT(updateStrainImage() ) ); */
	
	 /////Arrange controls in Horizontal Grid/////
     QGridLayout *controlsLayout = new QGridLayout;

	 controlsLayout->addWidget(depthLabel, 0, 0);
     controlsLayout->addWidget(imageDepthBox, 1,0);

	 controlsLayout->addWidget(focusLabel, 0, 1);
	 controlsLayout->addWidget(focusDepthBox, 1, 1);

     controlsLayout->addWidget(angleLabel, 0, 2);
	 controlsLayout->addWidget(angleBox, 1, 2);

	 controlsLayout->addWidget(fNameLabel, 0, 3);
	 controlsLayout->addWidget(fileNameBox, 1, 3);

	 controlsLayout->addWidget(rfInPlaneButton, 0, 4);
	 controlsLayout->addWidget(rfVolumeButton, 1, 4);

	 controlsLayout->addWidget(bModeOnButton, 0, 5);
	 
	 controlsLayout->addWidget(bModeOffButton,1, 5);

	// controlsLayout->addWidget(strainOnButton, 0, 6);

//	 controlsLayout->addWidget(strainOffButton, 1, 6);



	 
	 //Arrange the B-mode and Strain imaging in a horizontal layout
	 QHBoxLayout *imagingLayout = new QHBoxLayout;
	 imagingLayout->addWidget(bModeDisplay);
	// imagingLayout->addWidget(strainDisplay);


	 //Put the imaging controls underneath the images
	 QVBoxLayout *mainLayout = new QVBoxLayout;
	 mainLayout->addLayout(imagingLayout);
	 mainLayout->addLayout(controlsLayout);
	 mainLayout->setSpacing(30);

	 mainLayout->addWidget(statusBox, Qt::AlignCenter);
     setLayout(mainLayout);
//	 setLayout(controlsLayout);

	statusBox->resize(150, 150);
	statusBox->setMaximumHeight(150);
    statusBox->setMinimumHeight(150);
    statusBox->setMaximumWidth(150);
    statusBox->setMinimumWidth(150);
 }

 


 void Dialog::collectRfVolume()
 {
		
		//Initialize imaging
		m_porta->stopImage();
		m_porta->initImagingMode((imagingMode)RfMode); 
		m_porta->setParam(prmRfMode, 2);  //2 gives B-mode and rf images
		
		//variables related to angles stepped
		numAngles = 25;   //must be odd
		halfAngles = 12;   //(numAngles - 1)/2
		fpa = 20;

		
		
		
		/////////// get probe info to find out degrees per step
		probeInfo nfo;
		m_porta->getProbeInfo(nfo);
		degPerStep = (double)nfo.motorFov / (double)nfo.motorSteps;
		degPerStep = degPerStep/1000.0;
		steps = 12; //there is a minimum number of steps. I believe it is 8
		degPerAngle = (double)steps*degPerStep;
		

        //Get size of Image, initialize buffer for image storage
		if(rfBuffer != NULL)
			delete rfBuffer;

        int w = m_porta->getParam(prmRfNumLines);
        int h = m_porta->getParam(prmRfNumSamples);
        szFrm = (w * h * (16 / 8));   //(16 bit ints per sample/8 bits per char)*samples*lines
		try{
		rfBuffer = new unsigned char[szFrm*fpa*numAngles];  
		}
		catch( std::bad_alloc&)
		{
			statusBox->setText("Unable to allocate memory for saving RF data");
			return;
		}   
		//center the motor then move it ten steps off center
		 m_porta->goToPosition(centralAngle);
		
		//make the sweep symmetric, the central angle will look better than the others
		 for(int i = 0; i < halfAngles; i ++ )
			 m_porta->stepMotor(0, steps);

		

		//Acquire a manual volume	
		//Figure out sleep time necessary to get the requisite frames per angle
		minTime = 50;
		while(m_porta->getFrameCount(1) < fpa){
				m_porta->runImage();
				::Sleep(minTime);  //1 second of data
				m_porta->stopImage();
				minTime *=2;	
				}

				currentAngleInVolume = -1;
				statusBox->setText("Rf data Acquisition commenced \n" );

				volumeRefreshTimer->start(400);
				
 }

 
 void Dialog::acquireAngleInRfVolume(){
				
				volumeRefreshTimer->stop();
				Sleep(200); 
				currentAngleInVolume+=1;
				int slpTime = minTime;
				m_porta->runImage();
				::Sleep(slpTime); 
				m_porta->stopImage();

				while(m_porta->getFrameCount(1) < fpa){
				m_porta->runImage();
				::Sleep(500);  //500 ms of data?
				m_porta->stopImage();
				}
			
					
			// remember +4 when getting data from the cine b/c of the frame header (counter)
   			int counter = 0;
			for (int j=0; j<displayH; j++)  //compress the data
				{
				for(int i= 0; i<displayW; i++) 
					{
					bModeImage->setPixel(i,j,*(m_porta->getFrameAddress(0, 0 ) + 4 +counter) );
					counter++;
					}
				}
		
	
			QPixmap pix = QPixmap::fromImage(*bModeImage);
			bModeDisplay->setPixmap(pix);
			bModeDisplay->update();
			emit updateImageWhileVolumeAcquisition(); 
			
			for(int i = 0;i < fpa; i++)
			{
				memcpy(rfBuffer +i*szFrm + currentAngleInVolume*fpa*szFrm, m_porta->getFrameAddress(1, i ) + 4, szFrm);
			}

			m_porta -> initImagingMode((imagingMode)RfMode);  //resets the cine buffer
			m_porta->setParam(prmRfMode, 2);  //2 gives B-mode and rf images
		
		m_porta->stepMotor(1, steps);
		
		if(currentAngleInVolume < numAngles-1)
			volumeRefreshTimer->start(400);
		else
			emit volumeAcquisitionComplete();

 }
 
 void Dialog::writeRfVolume()   
 {
	  // write to file
		fNameString = fileNameBox->text();
		FILE* fp = fopen(fNameString.toLocal8Bit().data(), "wb");
        if (!fp)
        {
            if (rfBuffer)
            {
                delete[] rfBuffer;
            }
            return;
        } 

		int w = m_porta->getParam(prmRfNumLines);
        int h = m_porta->getParam(prmRfNumSamples);
		
		fwrite(&w,sizeof(int), 1, fp);
		fwrite(&h, sizeof(int), 1, fp);
		fwrite(&fpa, sizeof(int), 1, fp);
		fwrite(&numAngles, sizeof(int), 1, fp);
		fwrite(&degPerAngle, sizeof(double), 1, fp);
        fwrite(rfBuffer, szFrm*fpa*numAngles, 1, fp);
        fclose(fp); 

        if (rfBuffer)
        {
            delete[] rfBuffer;
        }
		statusBox->append("Volume collection Successful");

 }


 void Dialog::changeFocusDepth(int index)
 {

	 
	 switch( index) 
	 {
		case 0:
		 m_porta->setParam(prmBFocusDepth, 30000);  //units are microns
		 break;
		case 1:
		m_porta->setParam(prmBFocusDepth, 40000);
		break;
		case 2:
		m_porta->setParam(prmBFocusDepth, 50000);
		break;
		case 3:
		m_porta->setParam(prmBFocusDepth, 60000);
		break;
		case 4:
		m_porta->setParam(prmBFocusDepth, 70000);
		break;
		case 5:
		m_porta->setParam(prmBFocusDepth, 80000);
		break;
		case 6:
		m_porta->setParam(prmBFocusDepth, 90000);
		break;
		

	 } 
 }

 void Dialog::changeImageDepth(int index)
 {
	

switch( index) 
	 {
		case 0:
		 m_porta->setParam(prmBImageDepth, 20);  //units are mm
		 break;
		case 1:
		m_porta->setParam(prmBImageDepth, 30);
		break;
		case 2:
		m_porta->setParam(prmBImageDepth, 40);
		break;
		case 3:
		m_porta->setParam(prmBImageDepth, 50);
		break;
		case 4:
		m_porta->setParam(prmBImageDepth, 60);
		break;
		case 5:
		m_porta->setParam(prmBImageDepth, 70);
		break;
		case 6:
		m_porta->setParam(prmBImageDepth, 80);
		break;
		case 7:
		m_porta->setParam(prmBImageDepth, 90);
		break;
		case 8:
		m_porta->setParam(prmBImageDepth, 100);
		break;
		case 9:
		m_porta->setParam(prmBImageDepth, 110);
		break;
		case 10:
		m_porta->setParam(prmBImageDepth, 120);

		

	 }
 

	
 
 }

 void Dialog::changeStaticAngle(int index)
 {

	
	 switch( index) 
	 {
		case 0:
		m_porta->goToPosition(centralAngle - 10);
		 break;
		case 1:
		m_porta->goToPosition(centralAngle - 5);
		break;
		case 2:
		m_porta->goToPosition(centralAngle);
		break;
		case 3:
		m_porta->goToPosition(centralAngle + 5);
		break;
		case 4:
		m_porta->goToPosition(centralAngle + 10);
		break;
		

	 } 
 }
 
 void Dialog::startBModeTimer()
 {

	 	m_porta->stopImage();
		if( m_porta->initImagingMode((imagingMode)BMode)  )
			statusBox->setText("B-mode imaging activated");
		m_porta->setDisplayDimensions(0,displayW, displayH);

                
		
		 //center the motor
		 m_porta->goToPosition(centralAngle);
    	 m_porta->runImage();
	
		bModeTimer->start(75);  
 }


 void Dialog::startStrainTimer()
 {
	//	m_porta->goToPosition(centralAngle);
	//	strainTimer->start(1000);
	//	 saveRFImage = true;
		 
 }

 

 void Dialog::stopBModeTimer()
 {
	 bModeTimer->stop();
	 m_porta->stopImage();
 }
 
 void Dialog::stopStrainTimer()
 {

	// strainTimer->stop();
	// m_porta->stopImage();
 }
 

 void Dialog::updateBModeImage()
 {

	int hhhh = 0;
    // remember +4 when getting data from the cine b/c of the frame header (counter)
	m_porta->getBwImage(0, buffer, 0);    //0 for B-mode, pointer to Buffer, 0 to not apply colormap       
			    
              
	int counter = 0;
	for (int j=0; j<displayH; j++)  //compress the data
	{
		for(int i= 0; i<displayW; i++) 
			{
			bModeImage->setPixel(i,j,*(buffer+counter) );
			
			counter++;
			}
		}

		QPixmap pix = QPixmap::fromImage(*bModeImage);
		bModeDisplay->setPixmap(pix);


 }




 void Dialog::closeEvent(QCloseEvent *event)
 {
		m_porta->stopImage();
		if(m_porta != NULL)
		 delete m_porta;

 }






void Dialog::updateStrainImage()
{
	
/*	m_porta->stopImage();
	m_porta->initImagingMode((imagingMode)RfMode); 
	m_porta->setParam(prmRfMode, 1);

	//Change the size of the buffer for holding the strain image
	if(!strainImagePtr)
		delete strainImagePtr;

	if(!dpYPtr)
		delete strainImagePtr;

	
	if(!ccImage)
		delete ccImage;

	
	int w = m_porta->getParam(prmRfNumLines);
    int h = m_porta->getParam(prmRfNumSamples);

	//Work out displacement image geometry info
	//bounds on X displacement
	geom.startX = blockMatchParams.halfX + blockMatchParams.rangeX;
	geom.stopX = (w - 1) - blockMatchParams.halfX - blockMatchParams.rangeX;
	geom.numX = geom.stopX - geom.startX + 1;
	
	//bounds on blocks in Y direction
	geom.stepY =  ( (float)1 - blockMatchParams.overlap )*blockMatchParams.windowY;

	geom.startY = blockMatchParams.halfY + blockMatchParams.rangeY;
	int maxY = (h - 1) - blockMatchParams.rangeY - blockMatchParams.halfY;
	geom.numY = (maxY - geom.startY)/geom.stepY;
	geom.stopY = geom.startY + (geom.numY-1)*geom.stepY;

	geom.numSY = geom.numY - blockMatchParams.strainWin;

	//Initialize array to hold strain image
	strainImagePtr = new float[geom.numSY*geom.numX];
	dpYPtr = new float[geom.numY*geom.numX];
	ccImage = new float[geom.numY*geom.numX];

	//Get size of Image, initialize buffer for image storage
	short int *rfBuffer1, *rfBuffer2;
       
    int szFrm = (w * h);   //(16 bit ints per sample/8 bits per char)*samples*lines
		rfBuffer1 = new short int[szFrm];
		rfBuffer2 = new short int[szFrm];
		preBufferFloat = new float[szFrm];
		postBufferFloat = new float[szFrm];

		geom.rfY = h;
		geom.rfX = w;
	
		

		

		
		//Acquire a pre and post image	
		int slpTime = 50;
		while(m_porta->getFrameCount(1) < 2)
				{m_porta->runImage();
				::Sleep(slpTime);  //1 second of data
				m_porta->stopImage();
				slpTime *=2;	
				}
		slpTime = 50;
				
							
		memcpy(rfBuffer1, m_porta->getFrameAddress(1, 0 ) + 4, szFrm*sizeof(short int));  
		memcpy(rfBuffer2, m_porta->getFrameAddress(1, 1 ) + 4, szFrm*sizeof(short int));  
		
		QString num2str;

	
	//Cast pre and post image into floats
		for(int yy = 0; yy < h; yy++){
			for(int xx = 0; xx < w; xx++){
				preBufferFloat[yy + xx*h] = (float) rfBuffer1[yy + xx*h]; 
			}
		}

		
			

		for(int yy = 0; yy < h; yy++){
			for(int xx = 0; xx < w; xx++){
				postBufferFloat[yy + xx*h] = (float) rfBuffer2[yy + xx*h]; 
			}
		}
		




		//pass RF data to block matching algorithm
		//get pointer to strain image back
		block_match(ccImage, dpYPtr, strainImagePtr, preBufferFloat, postBufferFloat, blockMatchParams, geom );
		
		statusBox->append( "Number of rows in strain image is: " );
		statusBox->append(num2str.number(geom.numX) );
		
		//Assign an RGB value to the floating point strain image
		//conversion to 0-255 index image
		float tempStrainF;
		int tempStrain;
		float strainMax = .01; 
		int counter = 0;

			
		delete strainImage;
		strainImage = new QImage(geom.numX,geom.numSY, QImage::Format_Indexed8);
		strainImage->setNumColors(256);
		//Assign an RGB value to the 256 indexes in the image color table
		for ( int i = 0; i < 256; i++ ) 
			strainImage->setColor( i, qRgb( i, i, i )) ;


		for (int yy=0; yy<geom.numY; yy++)  //compress the data
		{
			for(int xx= 0; xx< geom.numX; xx++) 
				{
				//from float to int
				tempStrainF = fabs(strainImagePtr[yy + xx*geom.numSY] );
				if(tempStrainF > strainMax)
					tempStrainF = strainMax;
				tempStrain = (int)(tempStrainF/strainMax *255);
				strainImage->setPixel(xx,yy, tempStrain );
				counter++;
			}
		} 

		

		QPixmap strainPix = QPixmap::fromImage( (strainImage->scaled(displayW, displayH, Qt::IgnoreAspectRatio) ) );
		strainDisplay->setPixmap(strainPix);  
 
		
		if(saveRFImage){
			saveRFImage = false;

			std::ofstream outFile;
			outFile.open("strain", std::ios::binary);
			
			outFile.write( (char*) &geom.numSY, sizeof(int) );
			outFile.write( (char*) &geom.numX, sizeof(int) );
			outFile.write( (char*) strainImagePtr, sizeof(float)*geom.numSY*geom.numX);

			outFile.close();
		} 
		
		 
*/
}