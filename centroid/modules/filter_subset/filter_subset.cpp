#include <iostream>
#include <string>
#include <stdio.h>
#include <libgen.h>

#include <QDebug>
#include <QDir>

#if 1
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCropImageFilter.h"
#endif

#include "filter_subset.h"
#include "filter_support.h"

// 
// When we define metadata for a plugin, we know upfront the name of
// variables we use. 
// 
// In this particular plugin, we have filter_invert.json file:
//
// {
//    "name" : "filter.invert",
//    "version" : "0.0.1",
//    "type" : "imaging",
//    "parameters" : 
//    [ 
//       {
//          "name" : "input",
//          "type" : "file",
//          "form" : "Input image",
//          "value" : ""
//       },
//       {
//          "name" : "output",
//          "type" : "file",
//          "form" : "Output image",
//          "value" : ""
//       },
//       {
//          "name" : "threshold",
//          "type" : "float",
//          "form" : "Threshold",
//          "value" : "0.5"
//       }
//    ],
//    "dependancies" : []
// }
//
// Description of the filter_invert.json:
//    "name": defines unique name of the module,
//    "version": defines version of the module (can be used for checking
//          dependancies on other modules if we use cross modules),
//    "type": defines type of module (imaging, particles, textual, etc.)
//    "parameters": define all out customised parameters we could later
//          refer to when writing execution code, etc. 
//    "dependancies": describe names and version of the modules that this
//          module depends on.
//
// There could be many parameters use by a module. The structure will have
// though unique fields:
//    "name": defines name of the parameter,
//    "type": defines type of the parameter,
//    "form": defines string that will be displayed when UI is constructed
//            from filter_invert.json metadata,
//    "value": contains predefined value. We can assume safely to keep
//             all values always as QString 
//


//
// The CurrentModuleInstance and static writeLog function
// allow static methods in this file to write log messages
//
FilterSubset* CurrentModuleInstance = NULL;

static void writeLog ( const QString & message )
{
    if (CurrentModuleInstance != NULL)
    {
        CurrentModuleInstance->writeLog(message);
    }
    else
    {
        qDebug() << message;
    }
}



static void
removeImgs 
  (
  const QString &inputDir, 
  QFileInfoList & fileList, 
  const bool rmDups,
  const bool rmSingles,
  const bool rmNonBWs
  )
    {

    if ( ! (rmDups || rmSingles || rmNonBWs) )
        {
        return;
        }

    bool * isUnique = new bool [ fileList.size() ];
    for (int i = 0; i < fileList.size(); i++)
        {
        isUnique[i] = false;
        }
    isUnique[0] = true;

    if (rmDups || rmSingles)
        {
        for (int i = 1; i < fileList.size(); i++)
            {
            isUnique[i] = ! FilterSupport::filesAreSame (
                                          fileList[i-1].filePath().toStdString().c_str(), 
                                          fileList[i].filePath().toStdString().c_str() 
                                         );
            }
        }

    // Loop through images in reverse order so that as we remove
    // images from the list, we don't change the indexing relative
    // to the isUnique array until we are past it.
    for (int i = fileList.size()-1; i >= 0; i--)
        {

        // check to see if current image requires deletion.

        bool removeMe = false;

        if (rmDups)
            {
            if ( ! isUnique[i] )
                {
                removeMe = true;
                }
            }

        if (rmSingles)
            {
            if (i != (fileList.size()-1) )
                {
                if ( isUnique[i] && isUnique[i+1] )
                    {
                    removeMe = true;
                    }
                }

            }


        // we check if an image is not gray only if we haven't removed it 
        // for other reasons.
        if ( ( ! removeMe ) && rmNonBWs )
            {
            if ( ! FilterSupport::imgIsGray ( fileList[i].filePath().toStdString().c_str() ) )
                {
                removeMe = true;
                }
            }


        if (removeMe)
            {
            fileList.removeAt (i);
            }
        }


    delete [] isUnique;


    }  // end of removeImgs


#if 0

static void
cropImgs 
  (
  const QString &inputDir, 
  QFileInfoList & cropFileList, 
  const QString & outputDir, 
  const bool deriveSeqNumFromInputFN,
  const QString & outProjTag, 
  const unsigned int cropRect[2][2]
  )
{

    qDebug () << "Cropping " << cropFileList.size() << " images.";

    for (int i=0; i < cropFileList.size(); i++)
    {
        QFileInfo fileInfo = cropFileList.at(i);
        qDebug() << "CROP " << fileInfo.filePath();
    }



}  // end of cropImgs


#else


#if 1
typedef    unsigned char    PixelType;
typedef itk::Image< PixelType,  2 >   ImageType;
typedef itk::ImageFileWriter< ImageType >  WriterType;
typedef itk::ImageFileReader< ImageType >  ReaderType;

static int
readImage (const char *inFN, ImageType::Pointer & inImg)
    {

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inFN );
    inImg = reader->GetOutput();
    inImg->Update();

    return 0;
    }  // end of readImage



static int
writeImage
  (
  const char *outFN,
  ImageType::Pointer & outImg
  )
    {

    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outFN );
    writer->SetInput( outImg );
    writer->Update();

    return 0;
    }  // end of writeImage





static int 
cropImg (
  const unsigned int cropRect[2][2],
  ImageType::Pointer & inImg,
  ImageType::Pointer & outImg
  )
    {

    inImg->Update ();

    const ImageType::SizeType & inSize =
                        inImg->GetLargestPossibleRegion ().GetSize();

    const ImageType::IndexType & inIndex =
                        inImg->GetLargestPossibleRegion ().GetIndex();

    ImageType::SizeType upperCropSize;
    ImageType::SizeType lowerCropSize;

    // printf ("inSize = %d %d\n", inSize[0], inSize[1]);

    lowerCropSize[0] = cropRect[0][0];
    lowerCropSize[1] = cropRect[0][1];

    if ( (cropRect[1][0] == 0) && (cropRect[1][1] == 0) )
        {
        upperCropSize[0] = 0;
        upperCropSize[1] = 0;
        }
    else
        {
        upperCropSize[0] = (inSize[0] - cropRect[1][0]) - 1;
        upperCropSize[1] = (inSize[1] - cropRect[1][1]) - 1;
        }

    // printf ("lowerCropSize = %d %d\n", lowerCropSize[0], lowerCropSize[1]);
    // printf ("upperCropSize = %d %d\n", upperCropSize[0], upperCropSize[1]);

    typedef itk::CropImageFilter <ImageType, ImageType>
                                                    CropImageFilterType;

    CropImageFilterType::Pointer cropFilter = CropImageFilterType::New();

    cropFilter->SetInput (inImg);
    cropFilter->SetLowerBoundaryCropSize (lowerCropSize);
    cropFilter->SetUpperBoundaryCropSize (upperCropSize);
    cropFilter->Update();

    outImg = cropFilter->GetOutput();
    outImg->Update();

    return 0;
    }  // end of cropImg
#endif


static void
cropImg 
  (
  const unsigned int cropRect[2][2], 
  const char *inImgFN, 
  const char *outDir, 
  const bool deriveSeqNumFromInputFN,
  const int  altSeqNum,
  const char *outImgFNTag
  )
    {
    char outImgFN[1000];

    // qDebug () << "NEW CROP: inImgFN = " << inImgFN;

    FilterSupport::makeOutFN (inImgFN, outDir, deriveSeqNumFromInputFN,
                            altSeqNum, outImgFNTag, "png", outImgFN);

    // qDebug () << "In cropImg: outImgFN = " << outImgFN;

#if 1
    // read inImgFN into inImg
    ImageType::Pointer inImg;
    readImage (inImgFN, inImg);

    FilterSupport::setNominalImageCoordSys ( inImg );

    // crop image
    ImageType::Pointer outImg;
    cropImg (cropRect, inImg, outImg);

    FilterSupport::setNominalImageCoordSys ( outImg );

    // write outImg as outImgFN
    writeImage (outImgFN, outImg);
#endif

    } // end of cropImg




static void
cropImgs 
  (
  const QString &inputDir, 
  QFileInfoList & cropFileList, 
  const QString & outputDir, 
  const bool deriveSeqNumFromInputFN,
  const QString & outProjTag, 
  const unsigned int cropRect[2][2]
  )
{

    // qDebug () << "Cropping " << cropFileList.size() << " images.";


    writeLog (QString ("\nSubset: Begin...\n\n"));
    writeLog ("Subset: Processing images in "  + inputDir + "\n");

    for (int i=0; i < cropFileList.size(); i++)
    {

        writeLog ("Subset:      "  + cropFileList[i].fileName() + "\n");

        cropImg (cropRect, 
                 cropFileList[i].filePath().toStdString().c_str(), 
                 outputDir.toStdString().c_str(), 
                 deriveSeqNumFromInputFN, 
                 i,
                 outProjTag.toStdString().c_str()
                 );

    }
    writeLog (QString ("Subset: Done.\n\n"));



}  // end of cropImgs





#endif



static void
subsetImages 
  (
  const QString &inputDir, 
  const QFileInfoList & inputFileList, 
  const QString & outputDir, 
  const QString & projTag, 
  const bool rmDups,
  const bool rmSingles,
  const bool rmNonBWs,
  const unsigned int cropRect[2][2]
  )
    {

    QFileInfoList cropFileList = inputFileList;

    if (rmDups || rmSingles || rmNonBWs)
        {
        removeImgs (inputDir, cropFileList, rmDups, rmSingles, rmNonBWs);
        }


    if (cropFileList.size() > 0)
        {
        QString fileName;
        QString dirName; 
        int seqNum; 
        int seqNumDigits; 
        QString outProjTag; 
        QString opTag; 
        QString extension; 

        // we parse the first file name to see if it conforms
        // to our naming scheme.  This is signaled by whether
        // seqNumDigits > 0.
        FilterSupport::parseFileName (cropFileList[0].filePath(),
                                      dirName, seqNum, seqNumDigits,
                                      outProjTag, opTag, extension);

        outProjTag = projTag;
        if (projTag == "")
            {
            outProjTag = "p"; // JGH - temporary fix
            }
        outProjTag += ".subset";

        bool deriveSeqNumFromInputFN = seqNumDigits > 0;

        cropImgs (dirName, cropFileList, outputDir, 
                    deriveSeqNumFromInputFN, outProjTag, cropRect);
        }
    else
        {
        // error msg?
        }
    

    }  // end of subsetImages




void FilterSubset::execute(const QVector<QMap<QString, QVariant> >& parameters) 
{
   CurrentModuleInstance = this;  // this should be first line in execute

   qDebug() << "Entering module " + getMetaData()->getName();

   QString projectShortTag = getProject()->getShortTag();
   if (projectShortTag == "")
   {
        projectShortTag = "p";
   }
   // qDebug() << "Project short tag is |" + projectShortTag + "|";





   //
    QString inputDir;   
    QString outputDir;
    bool rmDups = false;
    bool rmSingles = false;
    bool rmNonBWs = false;
    unsigned int cropRect[2][2];
   
    // get the parameters values
    // I'm going to try an alternative way of checking param names
    for (int i=0; i<parameters.size(); ++i)
    {
        // qDebug() << parameters[i]["name"].toString();

        QString paramName = parameters[i]["name"].toString();
        if (paramName == "input")
        {
            inputDir = parameters[i]["value"].toString();
        }
        else if (paramName == "output")
        {
            outputDir = parameters[i]["value"].toString();
        }
        else if (paramName == "rmDups")
        {
            rmDups = parameters[i]["value"].toString() == "1";
        }
        else if (paramName == "rmSingles")
        {
            rmSingles = parameters[i]["value"].toString() == "1";
        }
        else if (paramName == "rmNonBWs")
        {
            rmNonBWs = parameters[i]["value"].toString() == "1";
        }
        else if (paramName == "cropLowerX")
        {
            cropRect[0][0] = parameters[i]["value"].toInt();
        }
        else if (paramName == "cropLowerY")
        {
            cropRect[0][1] = parameters[i]["value"].toInt();
        }
        else if (paramName == "cropUpperX")
        {
            cropRect[1][0] = parameters[i]["value"].toInt();
        }
        else if (paramName == "cropUpperY")
        {
            cropRect[1][1] = parameters[i]["value"].toInt();
        }
        else
        {
            // error
        }
      

    }  // end of loop over parameters


    writeParameters (parameters, outputDir);




    for (int i = 0; i < 2; i++)
        {
        if (cropRect[0][i] > cropRect[1][i])
            {
            unsigned int tmp;
            tmp = cropRect[0][i];
            cropRect[0][i] = cropRect[1][i];
            cropRect[1][i] = tmp;
            }
        }
            



   // qDebug() << "EXECUTE PARAMS >>>";  
   // qDebug() << inputDir;
   // qDebug() << outputDir;
   // qDebug() << rmDups;
   // qDebug() << rmSingles;
   // qDebug() << rmNonBWs;


    if ( ! FilterSupport::isDirectory (inputDir) )
        {
        writeLog ("Subset: Input folder does not exist: " + inputDir);
        return;
        }
    else if ( ! FilterSupport::mkDirectory (outputDir) )
        {
        writeLog ("Subset: Error accessing or creating output folder: " + outputDir);
        return;
        }


   QFileInfoList inputFileList;
   FilterSupport::getImageFileList (inputDir, inputFileList);
    for (int i=0; i < inputFileList.size(); ++i)
    {
        QFileInfo fileInfo = inputFileList.at(i);
        qDebug() << "FILE NAME " << fileInfo.fileName();
    }


   // qDebug() << "<<< EXECUTE PARAMS\n";  

    

   // processing code below
   //


    QString outProjTag = projectShortTag;

    subsetImages (inputDir, inputFileList, outputDir, outProjTag, 
                            rmDups, rmSingles, rmNonBWs, cropRect);


   qDebug() << "Exiting module " + getMetaData()->getName();
}


