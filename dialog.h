#ifndef DIALOG_H
#define DIALOG_H
//QT includes
#include <QDialog>
#include <QLCDNumber>
#include <QComboBox>
#include <Qimage>
#include <QPainter>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QTextEdit>

//openGL include
//#include "ch2.h"

//structures for keeping block matching info
//#include "block_matching_params.h"


//Porta includes
#include "ImagingModes.h"
#include "utx_opt.h"
#include "porta_params_def.h"
#include "porta.h"
#include "porta_std_includes.h"

#define CINE_SIZE     (64 * 1024 * 1024)   //Limit of 64 MB cine size
#ifndef SETTINGS_PATH
    #define SETTINGS_PATH   ("dat/")
#endif
#ifndef FIRMWARE_PATH
    #define FIRMWARE_PATH   ("fw/")
#endif
#ifndef LUT_PATH
    #define LUT_PATH        ("C:/luts/")
#endif
#ifndef LICENSE_PATH
    #define LICENSE_PATH    ("D:/")
#endif



 class QLabel;


 class Dialog : public QDialog
 {
     Q_OBJECT

 public:
     Dialog(QWidget *parent = 0);


	 //Need to reimplement close event to shutdown imaging when program closes
protected:
     void closeEvent(QCloseEvent *event);


signals:
	void acquireNextAngle(void);
	void volumeAcquisitionComplete(void);

 private slots:


	 
	 void collectRfVolume();
	 void acquireAngleInRfVolume();
	 void writeRfVolume();

	 void changeFocusDepth(int);
	 void changeImageDepth(int);
	 void changeStaticAngle(int);

	 void startBModeTimer();
	 void stopBModeTimer();
	 
	 void startStrainTimer();
	 void stopStrainTimer();

	 void updateStrainImage();
	 void updateBModeImage();

	 


 private:
  
     porta* m_porta;
	 
	 QTextEdit *statusBox;

	 QLabel *depthLabel;
	 QLabel *focusLabel;
     QComboBox *imageDepthBox;
	 QComboBox *focusDepthBox;

	 QLabel *fNameLabel;
	 QLineEdit *fileNameBox;
	 QString fNameString;

	 QPushButton *bModeOffButton;
	 QPushButton *bModeOnButton;
	 QImage *bModeImage;
	 QPainter *bModePainter;
	 QLabel *bModeDisplay;
	 

	// NeHeChapter2 *scanConvert;
	 QImage *strainImage;
	 QLabel *strainDisplay;
	 QPushButton *strainOffButton;
	 QPushButton *strainOnButton;
	 QPushButton *rfInPlaneButton;
	//Volume Controls
	int halfAngles;
	int numAngles;
	double degPerAngle;
	double degPerStep;
	int steps;
	int fpa;
	int fov;
	int centralAngle;
	int numLines;
	int currentAngleInVolume, minTime, szFrm;

	QLabel *angleLabel;
	QComboBox *angleBox;

	//Strain/B-mode image
	int displayW; 
	int displayH;
	//For holding the B-mode image
	unsigned char* buffer;
	QTimer *bModeTimer;
	unsigned char* rfBuffer;

	//For producing strain image
//	paramStruct blockMatchParams;
//	geomStruct geom;
	float *strainImagePtr, *preBufferFloat, *postBufferFloat, *dpYPtr, *ccImage;
	bool saveRFImage;
	QTimer *strainTimer;	
 
 };

 #endif