
/*
Need to parameterize:
    template: radius, minVal, exponent
    
    modByStdDev radius for sd nbhd calc
    thresholds for stretch after modByStdDev
*/


#include <iostream>
#include <string>
#include <stdio.h>
#include <libgen.h>

#include <QtCore>
#include <QtGlobal>
#include <QDebug>
#include <QDir>

#include "filter_atomPositions.h"
#include "filter_support.h"

/////////////////////////////////////////////////////////////////////




//
// The CurrentModuleInstance and static writeLog function
// allow static methods in this file to write log messages
//
static FilterAtomPositions *CurrentModuleInstance = NULL;

static void
writeLog (const QString & s)
    {

    if (CurrentModuleInstance != NULL)
        {
        CurrentModuleInstance->writeLog (s);
        }
    else
        {
        qDebug () << s;
        }

    }  // end of local writeLog



// functions specific to this plugin


// ..........................................
//
// the itk-based functions

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkMinimumMaximumImageCalculator.h"
// #include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"


typedef    float FloatPixelType;
typedef itk::Image< FloatPixelType, 2 >   FloatImageType;

typedef    unsigned int UIntPixelType;
typedef itk::Image< UIntPixelType, 2 >   UIntImageType;

typedef    unsigned short UShortPixelType;
typedef itk::Image< UShortPixelType, 2 >   UShortImageType;

typedef    unsigned char UCharPixelType;
typedef itk::Image< UCharPixelType, 2 >   UCharImageType;

typedef itk::ImageFileReader< FloatImageType >  FloatReaderType;


static int
readImage (const char *inFN, FloatImageType::Pointer & inImg)
    {

    FloatReaderType::Pointer reader = FloatReaderType::New();
    reader->SetFileName( inFN );

    inImg = reader->GetOutput();

    inImg->Update();

    return 0;
    }  // end of readImage



static int
makeProportionThresholdedImage
  (
  FloatImageType::Pointer & inImg,
  float proportionThreshold,
  UCharImageType::Pointer & thresholdedImg,
  float minmax[2]
  )
    {

    typedef itk::MinimumMaximumImageCalculator <FloatImageType>
            ImgCalcFilterType;

    ImgCalcFilterType::Pointer imageCalculatorFilter = ImgCalcFilterType::New ();
    imageCalculatorFilter->SetImage (inImg);
    imageCalculatorFilter->Compute ();

    double minPix = imageCalculatorFilter->GetMinimum ();
    double maxPix = imageCalculatorFilter->GetMaximum ();
    minmax[0] = minPix;
    minmax[1] = maxPix;

    // typedef itk::ThresholdImageFilter <FloatImageType> ThresholdImageFilterType;
    typedef itk::BinaryThresholdImageFilter <FloatImageType, UCharImageType>
                                                BinaryThresholdImageFilterType;

    BinaryThresholdImageFilterType::Pointer thresholdFilter
                            = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(inImg);
    thresholdFilter->SetLowerThreshold (
                            minPix + proportionThreshold*(maxPix-minPix));
    thresholdFilter->SetUpperThreshold (maxPix + (maxPix-minPix));
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInsideValue(255);


    thresholdedImg = thresholdFilter->GetOutput();
    thresholdedImg->Update ();

    return 0;
    }  // end of makeProportionThresholdedImage


typedef struct CC_Info
    {
    double x;
    double y;
    int nPix;
    double maxWeight;
    double totWeight;
    bool exclude;
    } CCInfo;


static int
writeConnectedComponentCentroids (
  const char *centroidListFN,
  FloatImageType::Pointer & inImg,
  int minNPix,
  float weightThreshold,
  UShortImageType::Pointer & connCompImg
  )
    {

    typedef itk::MinimumMaximumImageCalculator <UShortImageType>
            ImgCalcFilterType;

    ImgCalcFilterType::Pointer imageCalculatorFilter = ImgCalcFilterType::New ();
    imageCalculatorFilter->SetImage (connCompImg);
    imageCalculatorFilter->Compute ();

    unsigned short minPix = imageCalculatorFilter->GetMinimum ();
    unsigned short maxPix = imageCalculatorFilter->GetMaximum ();

    CCInfo * ccInfo = new CCInfo[maxPix+1];

    for (int i = 0; i <= maxPix; i++)
        {
        ccInfo[i].x = 0;
        ccInfo[i].y = 0;
        ccInfo[i].nPix = 0;
        ccInfo[i].maxWeight = 0;
        ccInfo[i].totWeight = 0;
        ccInfo[i].exclude = false;
        }

    UShortImageType::IndexType pixelIndex;

    const UShortImageType::RegionType & largestRegion =
                             connCompImg->GetLargestPossibleRegion ();
    const UShortImageType::SizeType & sz = largestRegion.GetSize ();

    for (int i = 0; i < sz[0]; i++)
        {
        for (int j = 0; j < sz[1]; j++)
            {
            pixelIndex[0] = i;
            pixelIndex[1] = j;
            unsigned short ccIndex = connCompImg->GetPixel (pixelIndex);
            if (ccIndex > 0)
                {
                double weight = inImg->GetPixel (pixelIndex);
                ccInfo[ccIndex].x += weight * i;
                ccInfo[ccIndex].y += weight * j;
                ccInfo[ccIndex].nPix++;
                ccInfo[ccIndex].totWeight += weight;
                ccInfo[ccIndex].maxWeight = 
                                 qMax (ccInfo[ccIndex].maxWeight, weight);
                }
            } // end of loop over j
        } // end of loop over i


    for (int i = 1; i <= maxPix; i++)
        {
        ccInfo[i].exclude =  (ccInfo[i].totWeight <= 0) ||
                             (ccInfo[i].nPix < minNPix) ||
                             (ccInfo[i].maxWeight < weightThreshold) ;
        }


    FILE *fp = fopen (centroidListFN, "w");
    if (fp == NULL)
        {
        // fprintf (stderr, "Unable to open file %s.\n", centroidListFN);
        qWarning () << "Unable to open file " + QString(centroidListFN) + ".";
        exit (-1);
        }

    for (int i = 1; i <= maxPix; i++)
        {
        if ( ! ccInfo[i].exclude )
            {
            ccInfo[i].x /= ccInfo[i].totWeight;
            ccInfo[i].y /= ccInfo[i].totWeight;
            fprintf (fp, "%5d    %5d       %11.5f  %11.5f      %.8g  %.8g\n",
                i, ccInfo[i].nPix, ccInfo[i].x, ccInfo[i].y,
                ccInfo[i].maxWeight, ccInfo[i].totWeight/ccInfo[i].nPix );
            }
        }

    fclose (fp);


    return 0;
    }  // end of writeConnectedComponentCentroids


static int makeConnectedComponentImage
  (
  UCharImageType::Pointer & thresholdedImg,
  UShortImageType::Pointer & connCompImg
  )
    {
    typedef itk::ConnectedComponentImageFilter 
                    <UCharImageType, UShortImageType >
                        ConnectedComponentImageFilterType;

    ConnectedComponentImageFilterType::Pointer connected =
                                  ConnectedComponentImageFilterType::New ();

    connected->SetInput(thresholdedImg);
    connected->Update();

    connCompImg = connected->GetOutput ();
    connCompImg->Update();

    return 0;
    } // end of makeConnectedComponentImage


static void 
atomPositions 
( 
const int minClusterSize,
const double threshold,
const char * inImgFN,
const char * outPosFN 
)
    {

    // printf ("Entered atom positions : %f %s %s\n", threshold, inImgFN, outPosFN);

    FloatImageType::Pointer inImg;
    UCharImageType::Pointer thresholdedImg;
    UShortImageType::Pointer connCompImg;
    float minmax[2];

    readImage (inImgFN, inImg);

    makeProportionThresholdedImage (inImg, threshold, thresholdedImg, minmax);

    makeConnectedComponentImage (thresholdedImg, connCompImg);


    double weightThreshold = 
                minmax[0] + weightThreshold * (minmax[1] - minmax[0]);

    // disable weight thresholding
    weightThreshold = minmax[0];

    writeConnectedComponentCentroids (outPosFN, inImg,
                            minClusterSize, weightThreshold, connCompImg);

    return;
    }  // end of atomPositions (C version)



static void
writeLayerFile (const char *outLayerFN, const char * outCroppedFN)
    {
    // printf ("writeLayerFile %s %s\n", outLayerFN, outCroppedFN);
    FILE *fp = fopen (outLayerFN, "w");
    if (fp == NULL)
        {
        // error JGH
        return;
        }

    fprintf (fp, "%s\n", outCroppedFN);
    fclose (fp);

    return;
    }   // end of writeLayerFile

// ..........................................
//
// Here are the functions that connect up the plugin Qt framework to
// the itk-based functions

static void
atomPositions
  (
  const int minClusterSize,
  const double threshold,
  const QFileInfoList & inputFileList, 
  const QString & outputDir,
  const QString & projectShortTag 
  )
    {


    if (inputFileList.size() <= 0)
        {
        // error ?
        return;
        }


    QString fileName;
    QString dirName; 
    int seqNum; 
    int seqNumDigits; 
    QString outProjTag; 
    QString outCroppedProjTag; 
    QString opTag; 
    QString extension; 
    QString outExtension; 
    QString outLayerExtension; 

    // we parse the first file name to see if it conforms
    // to our naming scheme.  This is signaled by whether
    // seqNumDigits > 0.
    FilterSupport::parseFileName ( inputFileList[0],
                                   dirName, seqNum, seqNumDigits,
                                   outProjTag, opTag, extension);

    if (outProjTag == "")
        {
        outProjTag = projectShortTag;
        }
    outCroppedProjTag = outProjTag+".cropped";
    outProjTag += ".atomPos";

    outExtension = "apos";
    outLayerExtension = "layer";

    bool deriveSeqNumFromInputFN = seqNumDigits > 0;


    writeLog ("\nAtomPositions: Begin...\n");
    qDebug () << "AtomPositions for " << inputFileList.size() << " files.";
    
    QString outputCroppedDir = outputDir + "/../subset";
    // qDebug() << "outputCroppedDir = " + outputCroppedDir;
    QDir qd = QDir (outputCroppedDir);
    outputCroppedDir = qd.canonicalPath ();
    // qDebug() << "outputCroppedDir canonical = " + outputCroppedDir;

    for (int i=0; i < inputFileList.size(); i++)
        {
        QString outFN;

        FilterSupport::makeOutFN ( inputFileList[i].filePath(), outputDir,
                                   deriveSeqNumFromInputFN, i,
                                   outProjTag, outExtension, outFN );


#if 0
        QString outLayerFN;
        QString outCroppedFN;
        ///////////////
        //  this part and the related stuff above is sort of a hack
        makeOutFN ( inputFileList[i].filePath(), outputDir,
                    deriveSeqNumFromInputFN, i,
                    outProjTag, outLayerExtension, outLayerFN );

        makeOutFN ( inputFileList[i].filePath(), outputCroppedDir,
                    deriveSeqNumFromInputFN, i,
                    outCroppedProjTag, "png", outCroppedFN );

        // qDebug () << "outCroppedFN = " + outCroppedFN;
        writeLayerFile (outLayerFN.toStdString().c_str(), 
                 outCroppedFN.toStdString().c_str());
        //  end of hack
        ///////////////
#endif

        writeLog ("AtomPositions: processing " + 
                    inputFileList[i].filePath() + "\n");

        atomPositions ( minClusterSize, threshold,
                        inputFileList[i].filePath().toStdString().c_str(), 
                        outFN.toStdString().c_str()     );

        qDebug () << "  " << i << "  processing " << 
                                      inputFileList[i].filePath();
        }

    writeLog ("AtomPositions: Done.\n\n");
    qDebug () << "AtomPositions done.";

    return;

    }  // end of atomPositions




void FilterAtomPositions::execute
           (const QVector<QMap<QString, QVariant> >& parameters) 
{
    CurrentModuleInstance = this;  // this should be first line in execute

    qDebug () << "/nEntering module " + getMetaData()->getName();

    // std::cout << "\n>>> BLOCK " << getMetaData()->getName().toStdString() << std::endl;


    
    QString projectShortTag = getProject()->getShortTag();
    if (projectShortTag == "")
    {
        projectShortTag = "p";
    }
    // qDebug() << "Project short tag is |" + projectShortTag + "|";



    //
    QString inputDir;   
    QString outputDir;
    double threshold = 0.0;
    int minClusterSize = 0;
   
    // get the parameters values
    // I'm going to try an alternative way of checking param names
    for (int i=0; i<parameters.size(); ++i)
    {
        // qDebug() << parameters[i]["name"].toString();

        QString paramName = parameters[i]["name"].toString();
        if (paramName == "inputDir")
        {
            inputDir = parameters[i]["value"].toString();
        }
        else if (paramName == "outputDir")
        {
            outputDir = parameters[i]["value"].toString();
        }

        else if (paramName == "percentThreshold")
        {
// qDebug () << "about to get threshold";
            threshold = parameters[i]["value"].toFloat() / 100.0;
// qDebug () << "got threshold " << threshold;
        }
        else if (paramName == "minClusterSize")
        {
            minClusterSize = parameters[i]["value"].toInt();
        }

        else
        {
            // error
        }
      

    }  // end of loop over parameters


    writeParameters (parameters, outputDir);


    // threshold = 0.05;


    qDebug() << "EXECUTE PARAMS >>>";  
    qDebug() << "    input folder  " + inputDir;
    qDebug() << "    output folder " + outputDir;



    if ( ! FilterSupport::isDirectory (inputDir) )
        {
        writeLog ("AtomPositions: Input folder does not exist: " + inputDir);
        return;
        }
    else if ( ! FilterSupport::mkDirectory (outputDir) )
        {
        writeLog (
            "AtomPositions: Error accessing or creating output folder: " + 
            outputDir );
        return;
        }


    QFileInfoList inputFileList;
    FilterSupport::getImageFileList (inputDir, inputFileList);

    for (int i=0; i < inputFileList.size(); ++i)
    {
        QFileInfo fileInfo = inputFileList.at(i);
        // qDebug() << "FILE NAME " << fileInfo.fileName();
    }

    qDebug () << "Begin atom position calculation....\n";

    // processing code below
    //
    atomPositions (minClusterSize, threshold, 
                        inputFileList, outputDir, projectShortTag);

    qDebug () << "Done with atom position calculation.\n";
    qDebug () << "Exiting module " + getMetaData()->getName() + "\n";

}  // end of FilterAtomPositions::execute




