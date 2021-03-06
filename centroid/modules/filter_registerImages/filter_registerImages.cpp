

#include <iostream>
#include <string>
#include <stdio.h>
#include <libgen.h>

#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryFile>
#include <QProcess>

#include "filter_registerImages.h"
#include "filter_support.h"


//
// The CurrentModuleInstance and static writeLog function
// allow static methods in this file to write log messages
//
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

typedef enum Pixel_T {
    PixelType_UCHAR,
    PixelType_UCHAR_RGB,
    PixelType_UCHAR_RGBA,
    PixelType_FLOAT,
    PixelType_OTHER
} Pixel_Type;




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
            pType = PixelType_UCHAR;
            break;

        case itk::ImageIOBase::FLOAT:
            pType = PixelType_FLOAT;
            break;

        default:
            pType = PixelType_OTHER;
        }

    if (pType == PixelType_UCHAR)
        {
        switch (pixelType)
            {
            case itk::ImageIOBase::SCALAR:
                break;

            case itk::ImageIOBase::RGB:
                pType = PixelType_UCHAR_RGB;
                break;

            case itk::ImageIOBase::RGBA:
                pType = PixelType_UCHAR_RGBA;
                break;

            default:
                pType = PixelType_OTHER;
            }
        }
    else if (pType == PixelType_FLOAT)
        {
        if (pixelType != itk::ImageIOBase::SCALAR)
            {
            pType = PixelType_OTHER;
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
    FilterSupport::setNominalImageCoordSys ( inPtr );

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
    FilterSupport::setNominalImageCoordSys ( inPtr );


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
    FilterSupport::setNominalImageCoordSys ( inPtr );

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
    FilterSupport::setNominalImageCoordSys ( inPtr );

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
        case PixelType_FLOAT:
            // printf ("FLOAT IMAGE!\n");
            doResamp (fr, xform, inImg, outImg);
            break;
            
        case PixelType_UCHAR:
            // printf ("UCHAR IMAGE!\n");
            doResamp (ur, xform, inImg, outImg);
            break;
            
        case PixelType_UCHAR_RGB:
            // printf ("UCHAR RGB IMAGE!\n");
            doResamp (urgbr, xform, inImg, outImg);
            break;

        case PixelType_UCHAR_RGBA:
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
  const QString &outputDir,
  const QString &projectShortTag
  )
    {

    QString inImgDir, inImgProjTag, inImgOpTag, inImgExt;
    QString inXDir, inXProjTag, inXOpTag, inXExt;
    QString outImgFN;
    int inImgN, inImgNDigits;
    int inXN, inXNDigits;



    writeLog ("RegisterImages: Applying registration transformations...\n");
    writeLog ("RegisterImages: Processing " +
              QString::number (inputImgs.size()) + 
              " images in " + outputDir + ".\n");

    for (int i = 0; i < inputImgs.size(); i++)
        {

        writeLog ("RegisterImages:      " + inputImgs[i].fileName() + 
                  "       " + inputXforms[i].fileName() + "\n");

        FilterSupport::parseFileName (inputImgs[i], inImgDir, inImgN,
                inImgNDigits, inImgProjTag, inImgOpTag, inImgExt);

        FilterSupport::parseFileName (inputXforms[i], inXDir, inXN,
                inXNDigits, inXProjTag, inXOpTag, inXExt);

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

        // qDebug() << "out tag = " << inImgProjTag+"."+inImgOpTag+".registered";

        if (inImgProjTag == "")
            {
            inImgProjTag = projectShortTag;
            }

        FilterSupport::makeOutFN (inputImgs[i].filePath(), outputDir,
                                  deriveSeq, i, 
                                  inImgProjTag+"."+inImgOpTag+".registered", 
                                  inImgExt, outImgFN);


        xformImage (inputImgs[i].filePath(),
                        inputXforms[i].filePath(), outImgFN);

        // qDebug() << "out fn = " << outImgFN;

        // writeLog (".");

        }

    writeLog ("RegisterImages: Done applying transformations.\n");

    return;
    } // end of applyImageRegistration




static void
applyRegistration (
  const QString &inputDir,
  const QString &inputTransformDir,
  const QString &outputDir,
  const QString &projectShortTag
  )
    {

    QFileInfoList imgFileList;
    QFileInfoList aposFileList;
    QFileInfoList xformFileList;


    FilterSupport::getImageFileList (inputDir, imgFileList);
    FilterSupport::getXformFileList (inputTransformDir, xformFileList);

    bool regDone = false;

    if (imgFileList.size() == xformFileList.size())
        {
        applyImageRegistration (imgFileList, xformFileList, 
                                    outputDir, projectShortTag);
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

    FilterSupport::parseFileName (inFN, inDir, inSeqNum, inDigits, inProjTag,
                                  inOpTag, inExt);

    int seqNum =  (inDigits < 0) ? altSeqNum : inSeqNum;

    FilterSupport::makeOutFN (inFN, outXformDir, false, seqNum, 
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
  const QString outputTransformDir,
  const bool allowRotation,
  const QString projectShortTag
  )
    {

    writeLog ("RegisterImages: Generating registration transformations...\n");

    QFileInfoList inputFileList;
    FilterSupport::getImageFileList (inputImageDir, inputFileList);

    if ( inputFileList.size() <= 0 )
        {
        // error
        writeLog ("ERROR (RegisterImages) : No input files.\n");
        return;
        }



//////////////////////////////////////////////////////////////

#if 0
    // construct an ij macro file
    QTemporaryFile ijmacroFile;
    if (!ijmacroFile.open())
        {
        writeLog ("ERROR (Cannot open temporary file for ImageJ macro.\n");
        return;
        }

    const char * transformType = 
            allowRotation ? "[Rigid Body]" : "Translation" ;

    ijmacroFile.write( "\n" );
    ijmacroFile.write( "run(\"Image Sequence...\",\n" );
    ijmacroFile.write( "    \"open=[" );
    ijmacroFile.write(
        inputFileList[0].absoluteFilePath().toStdString().c_str() );
    ijmacroFile.write( "]  sort\");\n");

    ijmacroFile.write( "run(\"VerboseStackReg \", \"transformation=" );
    ijmacroFile.write( allowRotation ? "[Rigid Body]" : "Translation" ) ;
    ijmacroFile.write( "\");\n" );

        // "run(\"VerboseStackReg \", \"transformation=[Rigid Body]\");\n");

    ijmacroFile.write( "run(\"Quit\");\n" );
    ijmacroFile.flush();
    ijmacroFile.close();


#else

    QString macroFN = outputTransformDir + "/" + 
                        projectShortTag + ".register.macro";

    // construct an ij macro file
    QFile ijmacroFile (macroFN) ;
    if (!ijmacroFile.open(QIODevice::WriteOnly))
        {
        writeLog ("ERROR (Cannot open temporary file for ImageJ macro.\n");
        return;
        }

    const char * transformType = 
            allowRotation ? "[Rigid Body]" : "Translation" ;

    ijmacroFile.write( "\n" );
    ijmacroFile.write( "run(\"Image Sequence...\",\n" );
    ijmacroFile.write( "    \"open=[" );
    ijmacroFile.write(
        inputFileList[0].absoluteFilePath().toStdString().c_str() );
    ijmacroFile.write( "]  sort\");\n");

    ijmacroFile.write( "run(\"VerboseStackReg \", \"transformation=" );
    ijmacroFile.write( allowRotation ? "[Rigid Body]" : "Translation" ) ;
    ijmacroFile.write( "\");\n" );

        // "run(\"VerboseStackReg \", \"transformation=[Rigid Body]\");\n");

    ijmacroFile.write( "run(\"Quit\");\n" );
    ijmacroFile.flush();
    ijmacroFile.close();


    ijmacroFile.open (QIODevice::ReadOnly);

    QTextStream macroStrm (&ijmacroFile);
    QString line;

    qDebug () << "Contents of imagej macro file " + ijmacroFile.fileName();
    for (line = macroStrm.readLine(); 
                            !line.isNull(); line = macroStrm.readLine( ))
        {
        qDebug () << line;
        }
    qDebug () << "End of imagej macro file " + ijmacroFile.fileName();

    ijmacroFile.close ();

    

#endif



//////////////////////////////////////////////////////////////


    // hard-code a few locations for ImageJ
    QDir ijDir( "./ImageJ" );
    if (!ijDir.exists()) ijDir = QDir( "../../external/ImageJ" );
    if (!ijDir.exists()) ijDir = QDir( "../../sdk/ImageJ" );

    if (!ijDir.exists("ij.jar"))
        {
        writeLog ("ERROR (RegisterImages) : Unable to find ImageJ.\n");
        return;
        }
    qDebug() << "Using ImageJ in: " << ijDir.absolutePath();

    // use the QProcess object to execute ImageJ
    QProcess ijReg;
    ijReg.setWorkingDirectory( ijDir.absolutePath() );
    ijReg.setProcessChannelMode( QProcess::MergedChannels );
    ijReg.setProcessEnvironment( QProcessEnvironment::systemEnvironment() );

    // Windows only: try and first use the bundled JRE
    QString ijRegProg (QCoreApplication::applicationDirPath() + "/jre7/bin/java.exe");
    if (!QFileInfo::exists( ijRegProg )) ijRegProg = "java"; // Fallback to path
    QStringList ijRegArgs;
    ijRegArgs << "-jar" << "ij.jar" << "-macro"
        << QDir::toNativeSeparators(ijmacroFile.fileName());


    ijReg.start(ijRegProg, ijRegArgs);
    if ( !ijReg.waitForFinished(-1) )
        {
        writeLog ("ERROR (RegisterImages) : Unable to run command: <");
        writeLog (ijReg.program().toLocal8Bit().constData());

        for (int i=0; i<ijRegArgs.size(); ++i)
            {
            writeLog (" ");
            writeLog (ijRegArgs.at(i).toLocal8Bit().constData());
            }

        writeLog (">\n");
        return;
        }


    writeRegistration ( 0, "VSG_XFORM:9:  1 0 0  0 1 0  0 0 1\n", 
                           inputFileList[0].filePath(), outputTransformDir );


    QString ijRegToken =  "VSG_XFORM:9:";
    int i = 1;

    char ijLine[1000];
    while ( ijReg.canReadLine() )
        {
        ijReg.readLine( ijLine, sizeof(ijLine) );
        qDebug() << "ijL: " << ijLine;
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


    writeLog (
        "RegisterImages: Done generating registration transformations.\n");

    return;
    } // end of generateRegistrationTransformsIJ


void FilterRegisterImages::execute
           (const QVector<QMap<QString, QVariant> >& parameters) 
{
    CurrentModuleInstance = this;  // this should be executable line


    qDebug() << "Entering module " + getMetaData()->getName() + "\n";


    QString projectShortTag = getProject()->getShortTag();
    if (projectShortTag == "")
    {
        projectShortTag = "p";
    }
    // qDebug() << "Project short tag is |" + projectShortTag + "|";



    //
    QString inputImageDir;   
    QString outputTransformDir;
    QString outputImageDir;
    bool allowRotation = false;
   
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
       else if (paramName == "allowRotation")
        {
            allowRotation = parameters[i]["value"].toString() == "1";
        }

        else
        {
            // error
           writeLog (
               "ERROR (RegisterImages) : Invalid parameter name <" +
               paramName+ ">.\n");
 
        }
      

    }  // end of loop over parameters

    writeParameters (parameters, outputTransformDir);


    // qDebug() << "EXECUTE PARAMS >>>";  
    // qDebug() << inputImageDir;
    // qDebug() << outputTransformDir;
    // qDebug() << outputImageDir;


    bool transformImages = ( outputImageDir != "");

    if ( ! FilterSupport::isDirectory (inputImageDir) )
        {
        writeLog ("RegisterImages: Input image folder does not exist: " + inputImageDir);
        return;
        }

    if (transformImages)
        {
        if ( ! FilterSupport::mkDirectory (outputImageDir) )
            {
            writeLog ("RegisterImages: Error accessing or creating output image folder: " + outputImageDir);
            return;
            }
        }

    if ( ! FilterSupport::mkDirectory (outputTransformDir) )
        {
        writeLog ("RegisterImages: Error accessing or creating output transform folder: " + outputTransformDir);
        return;
        }



    // QFileInfoList inputFileList;
    // getImageFileList (inputDir, inputFileList);

    writeLog ("\nRegisterImages: Begin...\n");
    generateRegistrationTransformsIJ (inputImageDir, 
                                      outputTransformDir, 
                                      allowRotation,
                                      projectShortTag);

    if (transformImages)    
        {
        applyRegistration (inputImageDir, outputTransformDir, 
                                            outputImageDir, projectShortTag);
        }
    
    writeLog ("RegisterImages: Done.\n\n");

    qDebug() << "Exiting module " + getMetaData()->getName() + "\n";

}  // end of FilterRegisterImages::execute




