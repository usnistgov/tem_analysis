

#include <iostream>
#include <string>
#include <stdio.h>
#include <libgen.h>

#include <QDebug>
#include <QDir>

#include "filter_registerImages.h"

#define MAX(x,y)  (((x)>(y))?(x):(y))

static FilterRegisterImages *CurrentModuleInstance = NULL;

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

/////////////////////////////////////////////////////////////////////

// General purpose functions

static void
parseFileName 
  (
  const QFileInfo fileInfo, 
  QString & dirName, 
  int & seqNum, 
  int & seqNumDigits, 
  QString &projTag, 
  QString &opTag, 
  QString &extension
  )
    {

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
getFilteredFileList
  (
  const QString & dirPath,
  const QStringList & fnFilters,
  QFileInfoList & imgFileList
  )
    {
    QDir dir;
    dir.setFilter (QDir::Files);
    dir.setSorting(QDir::Name);
    dir.setPath(dirPath);

    QStringList nameFilters;
    nameFilters << "*.bmp" << "*.png" << "*.tif" << "*.gif";

    dir.setNameFilters (fnFilters);

   imgFileList = dir.entryInfoList();

   // qDebug() << "Number of files in " << dirPath << ": " << imgFileList.size() ;

#if 0

    for (int i=0; i < imgFileList.size(); ++i)
        {
        QFileInfo fileInfo = imgFileList.at(i);
        qDebug() << "file name " << fileInfo.fileName();
        }
#endif

    }  // end of getFilteredImageFileList



static void
getImageFileList
  (
  const QString dirPath,
  QFileInfoList & fileList
  )
    {
    QStringList filters;
    filters << "*.bmp" << "*.png" << "*.tif" << "*.gif" <<
                        "*.BMP" << "*.PNG" << "*.TIF" << "*.GIF"  ;

    getFilteredFileList (dirPath, filters, fileList);
    } // end of getImageFileList



static void
getXformFileList
  (
  const QString dirPath,
  QFileInfoList & fileList
  )
    {
    QStringList filters;
    filters << "*.xform" << "*.XFORM";

    getFilteredFileList (dirPath, filters, fileList);
    } // end of getXformFileList







static void
makeOutFN 
  (
  const char *inImgFN, 
  const char *outDir, 
  const bool deriveSeqNumFromInputFN,
  const int altSeqNum,
  const char *outImgFNTag, 
  const char *outExt,
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
    sprintf (outFNFmt, "%%s/%%0%dd.%%s.%%s", seqNumLen);

    // printf ("seqNum = %d\n", seqNum);
    // printf ("seqNumLen = %d\n", seqNumLen);
    // printf ("out fmt= <%s>\n", outFNFmt);

    const char * outDirectory =  (outDir[0] == 0) ? "." : outDir;
        
    sprintf (outImgFN, outFNFmt, outDirectory, seqNum, outImgFNTag, outExt);

    // qDebug () << "outImgFN = " << outImgFN;

    }  // end of makeOutFN with C style args




static void
makeOutFN 
(  
const QString & fileName, 
const QString & outputDir,
const bool deriveSeqNumFromInputFN,
const int altSeqNum,
const QString & outImgFNTag, 
const QString & outExt, 
QString & outImgFN
)
{
    char outImgFN_c[10000];

    makeOutFN   (
                fileName.toStdString().c_str(),
                outputDir.toStdString().c_str(),
                deriveSeqNumFromInputFN,
                altSeqNum,
                outImgFNTag.toStdString().c_str(),
                outExt.toStdString().c_str(),
                outImgFN_c
                );

    outImgFN = outImgFN_c;
                
} // end of makeOutFN (with Qt object args)


// end of general purpose functions

/////////////////////////////////////////////////////////////////////




// functions specific to this plugin






// ..........................................
//
// the itk-based functions


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include <itkResampleImageFilter.h>
#include <itkRigid2DTransform.h>
#include <itkLinearInterpolateImageFunction.h>








typedef    float FloatPixelType;
typedef itk::Image< FloatPixelType, 2 >   FloatImageType;

typedef    unsigned int UIntPixelType;
typedef itk::Image< UIntPixelType, 2 >   UIntImageType;

typedef    unsigned short UShortPixelType;
typedef itk::Image< UShortPixelType, 2 >   UShortImageType;

typedef    unsigned char UCharPixelType;
typedef itk::Image< UCharPixelType, 2 >   UCharImageType;

typedef    itk::RGBPixel<unsigned char> UCharRGBPixelType;
typedef itk::Image< UCharRGBPixelType, 2 >   UCharRGBImageType;

typedef    itk::RGBAPixel<unsigned char> UCharRGBAPixelType;
typedef itk::Image< UCharRGBAPixelType, 2 >   UCharRGBAImageType;




typedef itk::ImageFileReader< FloatImageType >  FloatReaderType;
typedef itk::ImageFileReader< UCharImageType >  UCharReaderType;
typedef itk::ImageFileReader< UCharRGBImageType >  UCharRGBReaderType;
typedef itk::ImageFileReader< UCharRGBAImageType >  UCharRGBAReaderType;

typedef itk::ImageFileWriter< FloatImageType >  FloatWriterType;
typedef itk::ImageFileWriter< UCharImageType >  UCharWriterType;
typedef itk::ImageFileWriter< UCharRGBImageType >  UCharRGBWriterType;
typedef itk::ImageFileWriter< UCharRGBAImageType >  UCharRGBAWriterType;

typedef enum Pixel_T { UCHAR, UCHAR_RGB, UCHAR_RGBA, FLOAT, OTHER } Pixel_Type;




// several debug output functions

void printRegionInfo ( const char *label, 
                       const FloatImageType::RegionType & region )
    {
    const FloatImageType::SizeType & sz = region.GetSize ();
    const FloatImageType::IndexType & indx = region.GetIndex ();

    printf ("%s\n", label);
    printf ("        index: %d %d\n", indx[0], indx[1]);
    printf ("        size : %d %d\n", sz[0], sz[1]);

    } // end of printRegionInfo


void printImageInfo (const char *label, FloatImageType::Pointer &img)
    {

    const FloatImageType::RegionType & bufferedRegion = 
                                            img->GetBufferedRegion ();

    const FloatImageType::RegionType & largestRegion = 
                                            img->GetLargestPossibleRegion ();

    const FloatImageType::RegionType & requestedRegion = 
                                            img->GetRequestedRegion ();

    const FloatImageType::SpacingType & spacing = img->GetSpacing();

    const FloatImageType::PointType & origin = img->GetOrigin();

    const FloatImageType::DirectionType & dirMatrix = img->GetDirection();

    printf ("\n");
    printf ("Image: %s\n", label);
    printf ("   origin : %f %f\n", origin[0], origin[1]);
    printf ("   spacing: %f %f\n", spacing[0], spacing[1]);
    printf ("   direction: %f %f\n", dirMatrix[0][0], dirMatrix[0][1]);
    printf ("              %f %f\n", dirMatrix[1][0], dirMatrix[1][1]);
    printRegionInfo ("   buffered region:", bufferedRegion);
    printRegionInfo ("   largest possible region:", largestRegion);
    printRegionInfo ("   requested region:", requestedRegion);
    printf ("\n");

    } // end of printImageInfo




void printImageInfo (const char *label, UCharImageType::Pointer &img)
    {

    const UCharImageType::RegionType & bufferedRegion = 
                                            img->GetBufferedRegion ();

    const UCharImageType::RegionType & largestRegion = 
                                            img->GetLargestPossibleRegion ();

    const UCharImageType::RegionType & requestedRegion = 
                                            img->GetRequestedRegion ();

    const UCharImageType::SpacingType & spacing = img->GetSpacing();

    const UCharImageType::PointType & origin = img->GetOrigin();

    const UCharImageType::DirectionType & dirMatrix = img->GetDirection();

    printf ("\n");
    printf ("Image: %s\n", label);
    printf ("   origin : %f %f\n", origin[0], origin[1]);
    printf ("   spacing: %f %f\n", spacing[0], spacing[1]);
    printf ("   direction: %f %f\n", dirMatrix[0][0], dirMatrix[0][1]);
    printf ("              %f %f\n", dirMatrix[1][0], dirMatrix[1][1]);
    printRegionInfo ("   buffered region:", bufferedRegion);
    printRegionInfo ("   largest possible region:", largestRegion);
    printRegionInfo ("   requested region:", requestedRegion);
    printf ("\n");

    } // end of printImageInfo








static bool
getImageFileInfo (const char *inFN, Pixel_Type & pType, int & numDim)
    {
    typedef itk::ImageIOBase::IOComponentType ComponentType;
    typedef itk::ImageIOBase::IOPixelType PixelType;

    itk::ImageIOBase::Pointer imageIO =
        itk::ImageIOFactory::CreateImageIO(inFN, itk::ImageIOFactory::ReadMode);

    imageIO->SetFileName(inFN);
    imageIO->ReadImageInformation();
    const ComponentType componentType = imageIO->GetComponentType();

    const PixelType pixelType = imageIO->GetPixelType();

    numDim = imageIO->GetNumberOfDimensions();

    switch (componentType)
        {
        case itk::ImageIOBase::UCHAR:
            pType = UCHAR;
            break;

        case itk::ImageIOBase::FLOAT:
            pType = FLOAT;
            break;

        default:
            pType = OTHER;
        }

    if (pType == UCHAR)
        {
        switch (pixelType)
            {
            case itk::ImageIOBase::SCALAR:
                break;

            case itk::ImageIOBase::RGB:
                pType = UCHAR_RGB;
                break;

            case itk::ImageIOBase::RGBA:
                pType = UCHAR_RGBA;
                break;

            default:
                pType = OTHER;
            }
        }
    else if (pType == FLOAT)
        {
        if (pixelType != itk::ImageIOBase::SCALAR)
            {
            pType = OTHER;
            }
        }


    return true;
    }  // end of getImageFileInfo


static int
readImage (const char *inFN, FloatImageType::Pointer & inImg)
    {

    FloatReaderType::Pointer reader = FloatReaderType::New();
    reader->SetFileName( inFN );

    inImg = reader->GetOutput();

    inImg->Update();

    return 0;
    }  // end of readImage F

static int
readImage (const char *inFN, UCharImageType::Pointer & inImg)
    {

    UCharReaderType::Pointer reader = UCharReaderType::New();
    reader->SetFileName( inFN );

    inImg = reader->GetOutput();

    inImg->Update();

    return 0;
    }  // end of readImage UC


static int
readImage (const char *inFN, UCharRGBImageType::Pointer & inImg)
    {

    UCharRGBReaderType::Pointer reader = UCharRGBReaderType::New();
    reader->SetFileName( inFN );

    inImg = reader->GetOutput();

    inImg->Update();

    return 0;
    }  // end of readImage UC RGB


static int
readImage (const char *inFN, UCharRGBAImageType::Pointer & inImg)
    {

    UCharRGBAReaderType::Pointer reader = UCharRGBAReaderType::New();
    reader->SetFileName( inFN );

    inImg = reader->GetOutput();

    inImg->Update();

    return 0;
    }  // end of readImage UC RGBA


// ............................


static int
writeImage ( const char *outFN, FloatImageType::Pointer & outImg )
    {
    FloatWriterType::Pointer writer = FloatWriterType::New();
    writer->SetFileName( outFN );

    writer->SetInput( outImg );

    writer->Update();

    return 0;
    }  // end of writeImage



static int
writeImage ( const char *outFN, UCharImageType::Pointer & outImg )
    {
    UCharWriterType::Pointer writer = UCharWriterType::New();
    writer->SetFileName( outFN );

    writer->SetInput( outImg );

    writer->Update();

    return 0;
    }  // end of writeImage UC



static int
writeImage ( const char *outFN, UCharRGBImageType::Pointer & outImg )
    {
    UCharRGBWriterType::Pointer writer = UCharRGBWriterType::New();
    writer->SetFileName( outFN );

    writer->SetInput( outImg );

    writer->Update();

    return 0;
    }  // end of writeImage UC RGB




static int
writeImage ( const char *outFN, UCharRGBAImageType::Pointer & outImg )
    {
    UCharRGBAWriterType::Pointer writer = UCharRGBAWriterType::New();
    writer->SetFileName( outFN );

    writer->SetInput( outImg );

    writer->Update();

    return 0;
    }  // end of writeImage UC RGBA


// ..........................................

typedef itk::Rigid2DTransform < double > TransformType;

typedef itk::ResampleImageFilter<FloatImageType, FloatImageType> FResampT;
typedef itk::ResampleImageFilter<UCharImageType, UCharImageType> UResampT;
typedef itk::ResampleImageFilter<UCharRGBImageType, UCharRGBImageType> URGBResampT;
typedef itk::ResampleImageFilter<UCharRGBAImageType, UCharRGBAImageType> URGBAResampT;

typedef itk::LinearInterpolateImageFunction<FloatImageType, double> FInterpT;
typedef itk::LinearInterpolateImageFunction<UCharImageType, double> UInterpT;
typedef itk::LinearInterpolateImageFunction<UCharRGBImageType, double> URGBInterpT;
typedef itk::LinearInterpolateImageFunction<UCharRGBAImageType, double> URGBAInterpT;




static void
doResamp 
  (
  FResampT::Pointer & resamp, 
  TransformType::Pointer & xform, 
  const char *inImg, 
  const char *outImg
  )
    {

    FloatImageType::Pointer inPtr;
    FloatImageType::Pointer outPtr;

    readImage (inImg, inPtr);

    // printImageInfo ("In image", inPtr);

    inPtr->Update();
    resamp->SetOutputSpacing(inPtr->GetSpacing());
    resamp->SetOutputOrigin(inPtr->GetOrigin());
    resamp->SetOutputDirection(inPtr->GetDirection());
    resamp->SetSize(inPtr->GetLargestPossibleRegion().GetSize());


    resamp->SetTransform (xform);
    FInterpT::Pointer pInterpolator = FInterpT::New();
    resamp->SetInterpolator(pInterpolator);
    resamp->SetInput (inPtr);
    outPtr = resamp->GetOutput();

    outPtr->Update ();
    // printImageInfo ("Out image", outPtr);

    writeImage (outImg, outPtr);
    } // end of doResamp float

static void
doResamp 
  (
  UResampT::Pointer & resamp, 
  TransformType::Pointer & xform, 
  const char *inImg, 
  const char *outImg
  )
    {
    UCharImageType::Pointer inPtr;
    UCharImageType::Pointer outPtr;

    readImage (inImg, inPtr);

    inPtr->Update();
    resamp->SetOutputSpacing(inPtr->GetSpacing());
    resamp->SetOutputOrigin(inPtr->GetOrigin());
    resamp->SetOutputDirection(inPtr->GetDirection());
    resamp->SetSize(inPtr->GetLargestPossibleRegion().GetSize());

    resamp->SetTransform (xform);
    UInterpT::Pointer pInterpolator = UInterpT::New();
    resamp->SetInterpolator(pInterpolator);
    resamp->SetInput (inPtr);
    outPtr = resamp->GetOutput();

    writeImage (outImg, outPtr);
    } // end of doResamp UChar

static void
doResamp 
  (
  URGBResampT::Pointer & resamp, 
  TransformType::Pointer & xform, 
  const char *inImg, 
  const char *outImg
  )
    {
    UCharRGBImageType::Pointer inPtr;
    UCharRGBImageType::Pointer outPtr;

    readImage (inImg, inPtr);

    inPtr->Update();
    resamp->SetOutputSpacing(inPtr->GetSpacing());
    resamp->SetOutputOrigin(inPtr->GetOrigin());
    resamp->SetOutputDirection(inPtr->GetDirection());
    resamp->SetSize(inPtr->GetLargestPossibleRegion().GetSize());

    resamp->SetTransform (xform);
    URGBInterpT::Pointer pInterpolator = URGBInterpT::New();
    resamp->SetInterpolator(pInterpolator);
    resamp->SetInput (inPtr);
    outPtr = resamp->GetOutput();

    writeImage (outImg, outPtr);
    } // end of doResamp UChar RGB

static void
doResamp 
  (
  URGBAResampT::Pointer & resamp, 
  TransformType::Pointer & xform, 
  const char *inImg, 
  const char *outImg
  )
    {
    UCharRGBAImageType::Pointer inPtr;
    UCharRGBAImageType::Pointer outPtr;

    readImage (inImg, inPtr);

    inPtr->Update();
    resamp->SetOutputSpacing(inPtr->GetSpacing());
    resamp->SetOutputOrigin(inPtr->GetOrigin());
    resamp->SetOutputDirection(inPtr->GetDirection());
    resamp->SetSize(inPtr->GetLargestPossibleRegion().GetSize());

    resamp->SetTransform (xform);
    URGBAInterpT::Pointer pInterpolator = URGBAInterpT::New();
    resamp->SetInterpolator(pInterpolator);
    resamp->SetInput (inPtr);
    outPtr = resamp->GetOutput();

    outPtr->Update();

    writeImage (outImg, outPtr);
    } // end of doResamp UChar


static void
getRigidXformParams 
  (
  const char *inXformFN, 
  double translate[2], 
  double & rotate
  )
    {
    double xform[9];
    char xformID[100];
    char inXformStr[1000];

    FILE *xformFP = fopen (inXformFN, "r");
    if (xformFP == NULL)
        {
        // error
        writeLog ("ERROR (RegisterImages) : Unable to open transform file ");
        writeLog (inXformFN);
        writeLog (".\n");
        fclose (xformFP);
        return;
        }

    if (fgets ( inXformStr, sizeof (inXformStr), xformFP) == NULL)
        {
        // error
        writeLog ("ERROR (RegisterImages) : Unable to read transform file ");
        writeLog (inXformFN);
        writeLog (".\n");
        fclose (xformFP);
        return;
        }

    fclose (xformFP);

    // printf ("xformFN = %s\n", inXformFN);
    // printf ("xform str = %s\n", inXformStr);


    // assumes that the matrix is a rigid transform: 
    //      rotation followed by translation
    sscanf (inXformStr, "%s  %lf %lf %lf  %lf %lf %lf  %lf %lf %lf",
        xformID,
        xform+0,
        xform+1,
        xform+2,
        xform+3,
        xform+4,
        xform+5,
        xform+6,
        xform+7,
        xform+8);

    rotate = atan2(xform[3], xform[0]);
    translate[0] = xform[2];
    translate[1] = xform[5];


    }  // end of getRigidXformParams


static void
xformImage (const char * inImg, const char *inXformFN, const char *outImg)
    {
    Pixel_Type pType;
    int numDim;
    double translate[2], rotate;

    getRigidXformParams (inXformFN, translate, rotate);


    TransformType::Pointer xform = TransformType::New ();

    itk::Array<double> transformParams;
    transformParams.SetSize(3);
    transformParams[0] = rotate;
    transformParams[1] = translate[0];
    transformParams[2] = translate[1];

    xform->SetParameters (transformParams);

    if ( ! getImageFileInfo (inImg, pType,  numDim) )
        {
        // error
        writeLog ("ERROR (RegisterImages) : Unable to read image file ");
        writeLog (inImg);
        writeLog (".\n");
        return;
        }

    FResampT::Pointer fr = FResampT::New();
    UResampT::Pointer ur = UResampT::New();
    URGBResampT::Pointer urgbr = URGBResampT::New();
    URGBAResampT::Pointer urgbar = URGBAResampT::New();

    switch (pType)
        {
        case FLOAT:
            // printf ("FLOAT IMAGE!\n");
            doResamp (fr, xform, inImg, outImg);
            break;
            
        case UCHAR:
            // printf ("UCHAR IMAGE!\n");
            doResamp (ur, xform, inImg, outImg);
            break;
            
        case UCHAR_RGB:
            // printf ("UCHAR RGB IMAGE!\n");
            doResamp (urgbr, xform, inImg, outImg);
            break;

        case UCHAR_RGBA:
            // printf ("UCHAR RGBA IMAGE!\n");
            doResamp (urgbar, xform, inImg, outImg);
            break;

        default:
            // printf ("UNKNOWN IMAGE!\n");
            // error
            writeLog ("ERROR (RegisterImages) : image file ");
            writeLog (inImg);
            writeLog (" has unknown pixel type.\n");
            break;

        }


    return;
    } // end of xformImage


static void
xformImage (const QString & inImg, const QString & inXform, QString & outImg)
    {

    xformImage ( inImg.toStdString().c_str(),
                 inXform.toStdString().c_str(),
                 outImg.toStdString().c_str()     );

    return;

    }  // end of xformImage 


// ..........................................






static void
applyImageRegistration (
  const QFileInfoList &inputImgs,
  const QFileInfoList &inputXforms,
  const QString &outputDir
  )
    {

    QString inImgDir, inImgProjTag, inImgOpTag, inImgExt;
    QString inXDir, inXProjTag, inXOpTag, inXExt;
    QString outImgFN;
    int inImgN, inImgNDigits;
    int inXN, inXNDigits;



    writeLog ("Applying registration to " +
                    QString::number (inputImgs.size()) + " image files.\n");

    for (int i = 0; i < inputImgs.size(); i++)
        {
        parseFileName (inputImgs[i], inImgDir, inImgN, inImgNDigits,
                                    inImgProjTag, inImgOpTag, inImgExt);

        parseFileName (inputXforms[i], inXDir, inXN, inXNDigits,
                                    inXProjTag, inXOpTag, inXExt);

        if (inImgN != inXN)
            {
            // error
            // mismatch of sequence number
            writeLog ( "ERROR (RegisterImages) : "
                       "Sequence number mismatch for input files.\n");
            writeLog ( "      " + inputImgs[i].fileName() + "\n");
            writeLog ( "      " + inputXforms[i].fileName() + "\n");
            return;
            }

        bool deriveSeq = (inXNDigits >= 0) && (inImgNDigits >= 0);

qDebug() << "out tag = " << inImgProjTag+"."+inImgOpTag+".registered";

        makeOutFN (inputImgs[i].filePath(), outputDir,
                    deriveSeq, i, 
                    inImgProjTag+"."+inImgOpTag+".registered", 
                    inImgExt, outImgFN);


        xformImage (inputImgs[i].filePath(),
                        inputXforms[i].filePath(), outImgFN);

qDebug() << "out fn = " << outImgFN;

        writeLog (".");

        }

    writeLog ("\n");

    return;
    } // end of applyImageRegistration




static void
applyRegistration (
  const QString &inputDir,
  const QString &inputTransformDir,
  const QString &outputDir
  )
    {

    QFileInfoList imgFileList;
    QFileInfoList aposFileList;
    QFileInfoList xformFileList;


    getImageFileList (inputDir, imgFileList);
    getXformFileList (inputTransformDir, xformFileList);

    bool regDone = false;

    if (imgFileList.size() == xformFileList.size())
        {
        applyImageRegistration (imgFileList, xformFileList, outputDir);
        regDone = true;
        }
    else
        {
        // error
        writeLog ( "ERROR (RegisterImages) : "
                       "Number of image files (" +
                    QString::number(imgFileList.size()) +
                    ") is different than the number of transform files (" +
                    QString::number(xformFileList.size()) + ").\n"
                  );
        return;
        }

    if ( ! regDone )
        {
        // error
        writeLog ("ERROR (RegisterImages) : No input files.\n");
        }


    } // end of applyRegistration



// ..........................................
//

static void
writeRegistration 
  ( 
  const int altSeqNum, 
  const QString & regLine, 
  const QString & inFN, 
  const QString &outXformDir
  )
    {
    QString inDir, inProjTag, inOpTag, inExt, outXformFN;
    int inSeqNum, inDigits;

    parseFileName (inFN, inDir, inSeqNum, inDigits, inProjTag, inOpTag, inExt);

    int seqNum =  (inDigits < 0) ? altSeqNum : inSeqNum;

    makeOutFN (inFN, outXformDir, false, seqNum, 
          inProjTag + "." + inOpTag + ".registration", "xform", outXformFN);

    FILE *outFP = fopen (outXformFN.toStdString().c_str(), "w");
    if (outFP == NULL)
        {
        // error
        writeLog (
            "ERROR (RegisterImages) : Unable to open output transform file ");
        writeLog (outXformFN);
        writeLog (".\n");
        return;
        }

    fprintf (outFP, regLine.toStdString().c_str());

    fclose (outFP);

    return;
    }  // end of writeRegistration


static void
generateRegistrationTransformsIJ
  (
  const QString inputImageDir, 
  const QString outputTransformDir
  )
    {

    QFileInfoList inputFileList;
    getImageFileList (inputImageDir, inputFileList);

    if ( inputFileList.size() <= 0 )
        {
        // error
        writeLog ("ERROR (RegisterImages) : No input files.\n");
        return;
        }


    // construct an ij macro file

    char ijMacroFN[L_tmpnam];
    tmpnam (ijMacroFN);
printf ("Tempname #1: %s\n",ijMacroFN);

    FILE *fp = fopen (ijMacroFN, "w");


    fprintf (fp, "\n");
    fprintf (fp, "run(\"Image Sequence...\",\n");
    fprintf (fp, "    \"open=%s  sort\");\n", 
                inputFileList[0].absoluteFilePath().toStdString().c_str() );
    fprintf (fp, 
        "run(\"VerboseStackReg \", \"transformation=[Rigid Body]\");\n");
    fprintf (fp, "run(\"Quit\");\n");
    fclose (fp);

    QString cmd = "cd ~/external/ImageJ/ImageJ; java -jar ij.jar -macro ";
    cmd += ijMacroFN;


printf ("COMMAND = %s\n", cmd.toStdString().c_str());
char tmpCmd[1000];
sprintf (tmpCmd, "cat %s", ijMacroFN);
system (tmpCmd);
// remove (ijMacroFN);


    FILE *ijRegFP = popen (cmd.toStdString().c_str(), "r");

    if (ijRegFP == NULL)
        {
        writeLog ("ERROR (RegisterImages) : Unable to run command: <");
        writeLog (cmd);
        writeLog (">.\n");
        return;
        }

    char ijLine[1000];


    writeRegistration ( 0, "VSG_XFORM:9:  1 0 0  0 1 0  0 0 1\n", 
                           inputFileList[0].filePath(), outputTransformDir );


    QString ijRegToken =  "VSG_XFORM:9:";
    int i = 1;

    while (fgets (ijLine, sizeof(ijLine), ijRegFP) != NULL)
        {
        char token0[1000];
        if (sscanf (ijLine, "%s", token0) == 1)
            {
            if ( ijRegToken == token0 )
                {
                writeRegistration 
                 ( i, ijLine, inputFileList[i].filePath(), outputTransformDir );
                i++;
                }
            }

        }


    // get rid of the temporary macro file
    remove (ijMacroFN);


    return;
    } // end of generateRegistrationTransformsIJ


void FilterRegisterImages::execute
           (const QVector<QMap<QString, QVariant> >& parameters) 
{
    CurrentModuleInstance = this;  // this should be executable line


    // std::cout << "\n>>> BLOCK " << getName().toStdString() << std::endl;


    //
    QString inputImageDir;   
    QString outputTransformDir;
    QString outputImageDir;
   
    // get the parameters values
    // I'm going to try an alternative way of checking param names
    for (int i=0; i<parameters.size(); ++i)
    {
        // qDebug() << parameters[i]["name"].toString();

        QString paramName = parameters[i]["name"].toString();
        if (paramName == "inputImageDir")
        {
            inputImageDir = parameters[i]["value"].toString();
        }
        else if (paramName == "outputTransformDir")
        {
            outputTransformDir = parameters[i]["value"].toString();
        }
        else if (paramName == "outputImageDir")
        {
            outputImageDir = parameters[i]["value"].toString();
        }

        else
        {
            // error
           writeLog (
               "ERROR (RegisterImages) : Invalid parameter name <" +
               paramName+ ">.\n");
 
        }
      

    }  // end of loop over parameters



    // qDebug() << "EXECUTE PARAMS >>>";  
    // qDebug() << inputImageDir;
    // qDebug() << outputTransformDir;
    // qDebug() << outputImageDir;


    bool transformImages = ( outputImageDir != "");

    // QFileInfoList inputFileList;
    // getImageFileList (inputDir, inputFileList);

    generateRegistrationTransformsIJ (inputImageDir, outputTransformDir);

    if (transformImages)    
        {
        applyRegistration (inputImageDir, outputTransformDir, 
                                                        outputImageDir);
        }
    

}  // end of FilterRegisterImages::execute




