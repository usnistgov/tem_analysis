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




static FilterSubset *CurrentModuleInstance = NULL;

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




static void
parseFileName 
  (
  QString fileName, 
  QString & dirName, 
  int & seqNum, 
  int & seqNumDigits, 
  QString &projTag, 
  QString &opTag, 
  QString &extension
  )
    {

    // not very efficient, but easy to code
    QFileInfo fileInfo = QFileInfo (fileName);

    dirName = fileInfo.filePath();
    QString baseName = fileInfo.fileName();

    seqNum = -1;
    seqNumDigits = -1;

    QStringList nameParts = baseName.split(".");

    if (nameParts.size() > 0)
        {
        QString seqNumStr = nameParts[0];
        bool isInt;
        seqNum = seqNumStr.toInt (&isInt);
        seqNumDigits = isInt ? seqNumStr.length() : -1;
        }


    extension = (nameParts.size() > 1) ?  nameParts[nameParts.size()-1] : "";

    projTag = (nameParts.size() > 2) ?  nameParts[1] : "";

    opTag = "";

    for (int i = 2; i < nameParts.size()-1; i++)
        {
        opTag += nameParts[i];
        if (i != nameParts.size()-2)
            {
            opTag += ".";
            }
        }

    } // end of parseFileName




static void
getImageFileList (const QString dirPath, QFileInfoList & imgFileList)
{
    QDir dir;
    dir.setFilter (QDir::Files);
    dir.setSorting(QDir::Name);
    dir.setPath(dirPath);

    QStringList nameFilters;
    nameFilters << "*.bmp" << "*.png" << "*.tif" << "*.gif";

    dir.setNameFilters (nameFilters);

   imgFileList = dir.entryInfoList();
   // qDebug() << "Number of files in " << dirPath << ": " << imgFileList.size() ;

    for (int i=0; i < imgFileList.size(); ++i)
    {
        QFileInfo fileInfo = imgFileList.at(i);
        // qDebug() << "file name " << fileInfo.fileName();
    }


}  // end of getImageFileList



#if 0

static bool
imgIsGray (const char *imgFN)
    {
    printf ("imgIsGray ( %s )\n", imgFN);
    return true;
    } // end of imgIsGray

#else

static bool
imgIsGray (const char *imgFN)
    {
    // printf ("imgIsGray ( %s )\n", imgFN);

    typedef itk::RGBPixel <float>         RGBPixelType;
    typedef itk::Image <RGBPixelType, 2>  RGBImageType;
    typedef itk::ImageFileReader < RGBImageType >  RGBReaderType;

    RGBReaderType::Pointer reader = RGBReaderType::New ();
    reader->SetFileName (imgFN);
    reader->Update ();

    RGBImageType::Pointer img = reader->GetOutput ();

    const RGBImageType::SizeType & sz =
                        img->GetLargestPossibleRegion().GetSize ();

    for (int i = 0; i < sz[0]; i++)
        {
        for (int j = 0; j < sz[1]; j++)
            {
            RGBImageType::IndexType pixelIndex;
            pixelIndex[0] = i;
            pixelIndex[1] = j;

            RGBPixelType pix = img->GetPixel (pixelIndex);

            if ( (pix[0] != pix[1]) ||
                 (pix[0] != pix[2]) ||
                 (pix[1] != pix[2])       )
                {
                return false;
                }
            }
        }


    return true;

    }  // end of imgIsGray
#endif







static bool
filesAreSame (const char *fnA, const char *fnB)
    {
    // printf ("filesAreSame ( %s , %s )\n", fnA, fnB );

    unsigned char *a, *aa, *b, *bb;
    unsigned long lenA, lenB;
    FILE *fpA, *fpB;
    bool rtn;


    fpA = fopen (fnA, "r");
    fpB = fopen (fnB, "r");

    if ((fpA == NULL) || (fpB == NULL))
        {
        fprintf (stderr, "Can't open file %s or %s\n", fnA, fnB);
        exit (-1);
        }

    fseek (fpA, 0, SEEK_END);
    lenA = ftell (fpA);
    fseek (fpA, 0, SEEK_SET);

    fseek (fpB, 0, SEEK_END);
    lenB = ftell (fpB);
    fseek (fpB, 0, SEEK_SET);


    if (lenA != lenB)
        {
        fclose (fpA);
        fclose (fpB);
        return 0;
        }

    aa = a = (unsigned char *) malloc (lenA*sizeof(unsigned char));
    bb = b = (unsigned char *) malloc (lenB*sizeof(unsigned char));

    if (fread (a, sizeof(unsigned char), lenA, fpA) != lenA)
        {
        fprintf (stderr, "bad read of A %s\n", fnA);
        exit (-1);
        }

    if (fread (b, sizeof(unsigned char), lenB, fpB) != lenB)
        {
        fprintf (stderr, "bad read of B %s\n", fnB);
        exit (-1);
        }


    fclose (fpA);
    fclose (fpB);

    rtn = true;

    for (unsigned int i = 0; i < lenA; i++)
        {
        if ((*(a++)) != (*(b++)))
            {
            // printf ("mismatch at %d\n", i);
            rtn = false;
            break;
            }
        }

    free (aa);
    free (bb);

    return rtn;

    } // end of filesAreSame








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
            isUnique[i] = ! filesAreSame (
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
            if ( ! imgIsGray ( fileList[i].filePath().toStdString().c_str() ) )
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


static void
makeOutFN 
  (
  const char *inImgFN, 
  const char *outDir, 
  const bool deriveSeqNumFromInputFN,
  const int altSeqNum,
  const char *outImgFNTag, 
  char *outImgFN
  )
    {
    static int lastSeqNum = 1;
    int seqNum;
    // char inName[1000];
    char *inImgFNdup, *inName;

    inImgFNdup = strdup (inImgFN);

    inName = basename (inImgFNdup);
    // strcpy (inName, basename (inImgFN));


    for (char * c = inName; *c != 0; c++)
        {
        if (*c == '.')
            {
            *c = 0;
            break;
            }
        }

    int seqNumLen;

    if (sscanf (inName, "%d", &seqNum) != 1)
        {
        seqNum = altSeqNum;
        seqNumLen = 6;
        }
    else
        {
        seqNumLen = strlen (inName);
        }


    char outFNFmt[100];
    sprintf (outFNFmt, "%%s/%%0%dd.%%s.png", seqNumLen);

    // printf ("seqNum = %d\n", seqNum);
    // printf ("seqNumLen = %d\n", seqNumLen);
    // printf ("out fmt= <%s>\n", outFNFmt);

    const char * outDirectory =  (outDir[0] == 0) ? "." : outDir;
        
    sprintf (outImgFN, outFNFmt, outDirectory, seqNum, outImgFNTag);

    // qDebug () << "outImgFN = " << outImgFN;

    }  // makeOutFN



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

    makeOutFN (inImgFN, outDir, deriveSeqNumFromInputFN, altSeqNum, outImgFNTag, outImgFN);

    // qDebug () << "In cropImg: outImgFN = " << outImgFN;

#if 1
    // read inImgFN into inImg
    ImageType::Pointer inImg;
    readImage (inImgFN, inImg);

    // crop image
    ImageType::Pointer outImg;
    cropImg (cropRect, inImg, outImg);

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

    for (int i=0; i < cropFileList.size(); i++)
    {

        writeLog ("Subset: processing image "  + cropFileList[i].filePath() + "\n");
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
  const QString & outProjTag, 
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
        parseFileName (cropFileList[0].filePath(), 
                        dirName, seqNum, seqNumDigits, 
                        outProjTag, opTag, extension);

        if (outProjTag == "")
            {
            outProjTag = "XXX"; // JGH - temporary fix
            }
        outProjTag += ".cropped";

        bool deriveSeqNumFromInputFN = seqNumDigits > 0;

        cropImgs (inputDir, cropFileList, outputDir, 
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

   std::cout << "\n>>> BLOCK " << getName().toStdString() << std::endl;


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

   QFileInfoList inputFileList;
   getImageFileList (inputDir, inputFileList);
    for (int i=0; i < inputFileList.size(); ++i)
    {
        QFileInfo fileInfo = inputFileList.at(i);
        // qDebug() << "FILE NAME " << fileInfo.fileName();
    }


   // qDebug() << "<<< EXECUTE PARAMS\n";  

    

   // processing code below
   //


    QString outProjTag = "TEM1";

    subsetImages (inputDir, inputFileList, outputDir, outProjTag, 
                            rmDups, rmSingles, rmNonBWs, cropRect);





}


