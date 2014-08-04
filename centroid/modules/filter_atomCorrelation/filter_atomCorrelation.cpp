
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

#include <QDebug>
#include <QDir>

#if 1
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCropImageFilter.h"
#endif

#include "filter_atomCorrelation.h"

/////////////////////////////////////////////////////////////////////



static FilterAtomCorrelation *CurrentModuleInstance = NULL;

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



static bool
isFile (const QString & filePath)
    {
    QFileInfo info = QFileInfo (filePath);
    return info.isFile ();
    } // end of isFile

static bool
isDirectory (const QString & dirPath)
    {
    QFileInfo info = QFileInfo (dirPath);
    return info.isDir ();
    }  // end of isDirectory

static bool
mkDirectory (const QString & dirPath)
    {
    if (isFile (dirPath))
        {
        // error: it exists and it's a regular file
        return false;
        }
    else if (isDirectory (dirPath))
        {
        // it already exists so we don't have to create
        return true;
        }

    QDir qdir = QDir (dirPath);

    return qdir.mkpath (".");
    // return qdir.mkpath (dirPath);

    } // end of mkDirectory







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
   qDebug() << "Number of files in " << dirPath << ": " << imgFileList.size() ;

    for (int i=0; i < imgFileList.size(); ++i)
    {
        QFileInfo fileInfo = imgFileList.at(i);
        // qDebug() << "file name " << fileInfo.fileName();
    }


}  // end of getImageFileList





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
// First the itk-based functions




#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskedFFTNormalizedCorrelationImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkNoiseImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkThresholdImageFilter.h"




typedef    unsigned char    InputPixelType;
typedef    float FloatPixelType;

typedef itk::Image< InputPixelType,  2 >   InputImageType;
typedef itk::Image< FloatPixelType, 2 >   FloatImageType;
typedef itk::ImageFileReader< InputImageType >  ReaderType;

typedef unsigned char                          OutputPixelType;
typedef itk::Image< OutputPixelType, 2 >        OutputImageType;



////////////////////////////////////////////////////////////////////////////

static int 
readImage (const char *inFN, InputImageType::Pointer & inImg)
    {

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( inFN );

    inImg = reader->GetOutput();

    inImg->Update();

    return 0;
    }  // end of readImage

static int 
writeFloatImage 
  (
  const char *outFN, 
  FloatImageType::Pointer & outImg
  )
    {
    outImg->Update ();

    typedef itk::ImageFileWriter< FloatImageType >  WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outFN );
 
    writer->SetInput( outImg );
    writer->Update();

    // NOTE: output floating point, use .mhd, dicom, or nrrd
    //       itk, imagej, and matlab all handle dicom floating point?

    return 0;
    }  // end of writeFloatImage



int stretch (   FloatImageType::Pointer & beforeImg,
                double lowerP,
                double upperP,
                FloatImageType::Pointer & afterImg   )
    {
    
    typedef itk::MinimumMaximumImageCalculator <FloatImageType>
            ImageCalculatorFilterType;

    ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(beforeImg);
    imageCalculatorFilter->Compute();


    double minPix = imageCalculatorFilter->GetMinimum ();
    double maxPix = imageCalculatorFilter->GetMaximum ();
    minPix = minPix + (maxPix-minPix) * lowerP;
    maxPix = minPix + (maxPix-minPix) * upperP;
    // The following make sense because the beforeImg is a correlation
    // image.  We really don't care about negative correlations.
    if ((minPix < 0) && (maxPix > 0)) minPix = 0.0;
    // printf ("min max pix = %f %f\n", minPix, maxPix);


    typedef itk::IntensityWindowingImageFilter<
               FloatImageType, FloatImageType >  ContrastFilterType;
    ContrastFilterType::Pointer contrastFilter = ContrastFilterType::New();

    contrastFilter->SetInput ( beforeImg );

    contrastFilter->SetWindowMinimum (minPix + (maxPix-minPix)/4.0);
    contrastFilter->SetWindowMaximum (maxPix);

    contrastFilter->SetOutputMinimum (0.0);
    contrastFilter->SetOutputMaximum (1.0);

    afterImg = contrastFilter->GetOutput();
    afterImg->Update ();


    return 0;

    }  // end of stretch



//.............................
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





void printImageInfo (const char *label, InputImageType::Pointer &img)
    {

    const InputImageType::RegionType & bufferedRegion = 
                                            img->GetBufferedRegion ();

    const InputImageType::RegionType & largestRegion = 
                                            img->GetLargestPossibleRegion ();

    const InputImageType::RegionType & requestedRegion = 
                                            img->GetRequestedRegion ();

    const InputImageType::SpacingType & spacing = img->GetSpacing();

    const InputImageType::PointType & origin = img->GetOrigin();

    const InputImageType::DirectionType & dirMatrix = img->GetDirection();

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



//.....................................


int cropCorrImg (
  InputImageType::Pointer & inImg, 
  InputImageType::Pointer & inTemplateImg, 
  FloatImageType::Pointer & corrImg,
  FloatImageType::Pointer & outImg 
  )
    {

    // There's something about this function that is very kludgey.
    // I really think that the need for this routine in this form
    // reflects bugs in how ITK handles the origin & index of
    // images.


    const InputImageType::SizeType & inSize = 
                        inImg->GetLargestPossibleRegion ().GetSize();

    const InputImageType::IndexType & inIndex = 
                        inImg->GetLargestPossibleRegion ().GetIndex();

    const InputImageType::SizeType & inTemplateSize = 
                        inTemplateImg->GetLargestPossibleRegion ().GetSize();

    const InputImageType::SizeType & corrSize = 
                        corrImg->GetLargestPossibleRegion ().GetSize();

    FloatImageType::IndexType outIndex = 
                        corrImg->GetLargestPossibleRegion ().GetIndex();

    FloatImageType::SizeType cropSize;

// printf ("corrSize = %d %d\n", corrSize[0], corrSize[1]);
// printf ("inSize = %d %d\n", inSize[0], inSize[1]);

    cropSize[0] = corrSize[0] - inSize[0];
    cropSize[1] = corrSize[1] - inSize[1];
    cropSize[0] /= 2;
    cropSize[1] /= 2;

    if ((cropSize[0] < 0) || (cropSize[1] < 0))
        {
        fprintf (stderr, "Bad crop size : %d %d\n", cropSize[0], cropSize[1]);
        exit (-1);
        }

    typedef itk::CropImageFilter <FloatImageType, FloatImageType>
                                                    CropImageFilterType;

    CropImageFilterType::Pointer cropFilter = CropImageFilterType::New();

    cropFilter->SetInput (corrImg);
    cropFilter->SetBoundaryCropSize (cropSize);
    cropFilter->Update();

    outImg = cropFilter->GetOutput();
    outImg->Update();

    outIndex[0] = inIndex[0];
    outIndex[1] = inIndex[1];

    // outImg->GetLargestPossibleRegion().SetIndex (outIndex);
    // outImg->GetRequestedRegion().SetIndex (outIndex);
    // outImg->GetBufferedRegion().SetIndex (outIndex);

    FloatImageType::RegionType region;
    region.SetIndex (outIndex);
    region.SetSize (inSize);
    outImg->SetRegions (region);


    outImg->Update();


    return 0;
    }  // end of cropCorrImg

// ...........


// Now we're getting into the code that is specific to the correlation method

int cropCorrImg (
  FloatImageType::Pointer & inImg, 
  FloatImageType::Pointer & inTemplateImg, 
  FloatImageType::Pointer & corrImg,
  FloatImageType::Pointer & outImg 
  )
    {

    // There's something about this function that is very kludgey.
    // I really think that the need for this routine in this form
    // reflects bugs in how ITK handles the origin & index of
    // images.


    const InputImageType::SizeType & inSize = 
                        inImg->GetLargestPossibleRegion ().GetSize();

    const InputImageType::IndexType & inIndex = 
                        inImg->GetLargestPossibleRegion ().GetIndex();

    const InputImageType::SizeType & inTemplateSize = 
                        inTemplateImg->GetLargestPossibleRegion ().GetSize();

    const InputImageType::SizeType & corrSize = 
                        corrImg->GetLargestPossibleRegion ().GetSize();

    FloatImageType::IndexType outIndex = 
                        corrImg->GetLargestPossibleRegion ().GetIndex();

    FloatImageType::SizeType cropSize;

// printf ("corrSize = %d %d\n", corrSize[0], corrSize[1]);
// printf ("inSize = %d %d\n", inSize[0], inSize[1]);

    cropSize[0] = corrSize[0] - inSize[0];
    cropSize[1] = corrSize[1] - inSize[1];
    cropSize[0] /= 2;
    cropSize[1] /= 2;

    if ((cropSize[0] < 0) || (cropSize[1] < 0))
        {
        fprintf (stderr, "Bad crop size : %d %d\n", cropSize[0], cropSize[1]);
        exit (-1);
        }

    typedef itk::CropImageFilter <FloatImageType, FloatImageType>
                                                    CropImageFilterType;

    CropImageFilterType::Pointer cropFilter = CropImageFilterType::New();

    cropFilter->SetInput (corrImg);
    cropFilter->SetBoundaryCropSize (cropSize);
    cropFilter->Update();

    outImg = cropFilter->GetOutput();
    outImg->Update();

    outIndex[0] = inIndex[0];
    outIndex[1] = inIndex[1];

    // outImg->GetLargestPossibleRegion().SetIndex (outIndex);
    // outImg->GetRequestedRegion().SetIndex (outIndex);
    // outImg->GetBufferedRegion().SetIndex (outIndex);

    FloatImageType::RegionType region;
    region.SetIndex (outIndex);
    region.SetSize (inSize);
    outImg->SetRegions (region);


    outImg->Update();


    return 0;
    }  // end of cropCorrImg F F F F




int correlateImg 
  ( 
  InputImageType::Pointer & inImg, 
  InputImageType::Pointer & inTemplateImg, 
  FloatImageType::Pointer & outImg 
  )
{

  typedef itk::MaskedFFTNormalizedCorrelationImageFilter
     <InputImageType, FloatImageType, InputImageType > CorrelationFilterType;

  CorrelationFilterType::Pointer correlate = CorrelationFilterType::New();
  correlate->SetFixedImage(inImg);
  correlate->SetMovingImage(inTemplateImg);
  correlate->SetMovingImageMask(inTemplateImg);
  correlate->SetRequiredFractionOfOverlappingPixels (1.0);
  correlate->Update();

  FloatImageType::Pointer corrImg = FloatImageType::New();

  corrImg = correlate->GetOutput();
  

  // note that correlate->GetOutput() is not registered with inImg. 
  return cropCorrImg (inImg, inTemplateImg, corrImg, outImg);

  // outImg = correlate->GetOutput ();
  // outImg->Update ();

  return 0;
}  // end of correlateImg


static void
conformImgSpace (const FloatImageType::Pointer & imgFixed, 
                       FloatImageType::Pointer & imgMoved)
    {
    const FloatImageType::RegionType & fixedRegion = 
                                        imgFixed->GetLargestPossibleRegion ();

    const FloatImageType::RegionType & movedRegion = 
                                        imgMoved->GetLargestPossibleRegion ();

    const FloatImageType::SizeType & fixedSz = fixedRegion.GetSize ();
    const FloatImageType::SizeType & movedSz = movedRegion.GetSize ();

    if ( (fixedSz[0] != movedSz[0]) || (fixedSz[1] != movedSz[1]) )
        {
        // error ; let itk throw the exception
        return;
        }

    const FloatImageType::SpacingType & spacing = imgFixed->GetSpacing();
    const FloatImageType::PointType & origin = imgFixed->GetOrigin();
    const FloatImageType::DirectionType & dirMatrix = imgFixed->GetDirection();

    imgMoved->SetSpacing (spacing);
    imgMoved->SetOrigin (origin);
    imgMoved->SetDirection (dirMatrix);

    return;
    }  // end of conformImgSpace


static int
modByStdDev (
  InputImageType::Pointer & inImg, 
  FloatImageType::Pointer & corrImg, 
  FloatImageType::Pointer & modImg
  )
    {
    typedef itk::NoiseImageFilter<
               InputImageType, FloatImageType >  NoiseFilterType;

    double radius = 15.0;

    NoiseFilterType::Pointer sdFilter = NoiseFilterType::New();

    sdFilter->SetInput ( inImg );
    sdFilter->SetRadius ( radius );

    FloatImageType::Pointer sdImg = sdFilter->GetOutput();
    sdImg->Update();


    FloatImageType::Pointer sdStretchImg;

    // eliminate the lower quintile of the sd range
    stretch (sdImg, 0.0, 1.0, sdStretchImg);


    typedef itk::MultiplyImageFilter <FloatImageType, FloatImageType >
        MultiplyImageFilterType;
 
    MultiplyImageFilterType::Pointer sqIF = 
                                    MultiplyImageFilterType::New ();
    sqIF->SetInput1(sdStretchImg);
    sqIF->SetInput2(sdStretchImg);

    FloatImageType::Pointer multiplierImg;
    multiplierImg = sqIF->GetOutput();

    multiplierImg = sdStretchImg;

    multiplierImg->Update();


    // printf ("Before conforming\n");
    // printImageInfo ("mult input 1 (multiplierImg) ", multiplierImg);
    // printImageInfo ("mult input 2 (corrImg) ", corrImg);


    // conformImgSpace (corrImg, multiplierImg);
    conformImgSpace (multiplierImg, corrImg);

    // printf ("After conforming\n");
    // printImageInfo ("mult input 1 (multiplierImg) ", multiplierImg);
    // printImageInfo ("mult input 2 (corrImg) ", corrImg);



    MultiplyImageFilterType::Pointer multIF = 
                                    MultiplyImageFilterType::New ();

    multIF->SetInput1 (multiplierImg);
    multIF->SetInput2 (corrImg);

    modImg = multIF->GetOutput();

    modImg->Update();

    

    return 0;
    
    }  // end of modByStdDev


static int
convertImgType (
  InputImageType::Pointer & inImg, 
  FloatImageType::Pointer & convertedImg
  )
    {
    typedef itk::CastImageFilter< InputImageType, FloatImageType> 
                                                        IF_CastFilterType;
    IF_CastFilterType::Pointer castFilter = IF_CastFilterType::New();
    castFilter->SetInput(inImg);

    convertedImg = castFilter->GetOutput();
    convertedImg->Update();

    return 0;
    }  // end of convertImgType

static int
invertImg (
  FloatImageType::Pointer & inImg,
  FloatImageType::Pointer & invertedImg
  )
    {
    typedef itk::MinimumMaximumImageCalculator <FloatImageType>
            ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
    imageCalculatorFilter->SetImage(inImg);
    imageCalculatorFilter->Compute();


    double minPix = imageCalculatorFilter->GetMinimum ();
    double maxPix = imageCalculatorFilter->GetMaximum ();

    typedef itk::InvertIntensityImageFilter <FloatImageType>
            InvertIntensityImageFilterType;
 
    InvertIntensityImageFilterType::Pointer invertIntensityFilter
                                = InvertIntensityImageFilterType::New();
    invertIntensityFilter->SetInput(inImg);
    invertIntensityFilter->SetMaximum(minPix+maxPix);

    invertedImg = invertIntensityFilter->GetOutput();
    invertedImg->Update ();

    return 0;
    }  // end of invertImg


static int
correlateImg (
  FloatImageType::Pointer & inImg,
  FloatImageType::Pointer & templateImg,
  FloatImageType::Pointer & correlationImg
  )
    {
    typedef itk::MaskedFFTNormalizedCorrelationImageFilter
      <FloatImageType, FloatImageType, FloatImageType > 
                                        FFF_CorrelationFilterType;

    FFF_CorrelationFilterType::Pointer correlate = 
                            FFF_CorrelationFilterType::New();

    correlate->SetFixedImage(inImg);
    correlate->SetMovingImage(templateImg);
    correlate->SetMovingImageMask(templateImg);
    correlate->SetRequiredFractionOfOverlappingPixels (1.0);
    correlate->Update();

    FloatImageType::Pointer tmpCorrImg = correlate->GetOutput();
    tmpCorrImg->Update();

    // set everything below zero to zero.
    typedef itk::ThresholdImageFilter <FloatImageType> ThresholdImageFilterType;
 
    ThresholdImageFilterType::Pointer thresholdFilter
                            = ThresholdImageFilterType::New();
    thresholdFilter->SetInput(tmpCorrImg);
    thresholdFilter->ThresholdBelow (0.0);
    thresholdFilter->SetOutsideValue(0.0);

    FloatImageType::Pointer tmpCorrImg2 = thresholdFilter->GetOutput();
    tmpCorrImg2->Update ();

    // note that correlate->GetOutput() is not registered with inImg. 
    return cropCorrImg (inImg, templateImg, tmpCorrImg2, correlationImg);

    }  // end of correlateImg flt flt flt



/////////////




// generate the atom template image on the fly......



static void
makeTemplateImage 
(
double radius, 
double minVal, 
double maxVal,
double expon,
InputImageType::Pointer & templateImg
)
    {

    templateImg = InputImageType::New ();

    int i, j;
    // double radius, frac, radi;
    int iRad, dim , center;


    iRad = ceil (1.4*radius) + 0.5;

    dim = 2 * iRad + 1;
    center = iRad;

    double scale = (maxVal - minVal)/pow (radius, expon);

    // Set up the image data
    InputImageType::RegionType region;
    InputImageType::IndexType start;
    start[0] = 0;
    start[1] = 0;
 
    InputImageType::SizeType size;
    unsigned int NumRows = dim;
    unsigned int NumCols = dim;
    size[0] = NumRows;
    size[1] = NumCols;
 
    region.SetSize(size);
    region.SetIndex(start);
 
    templateImg->SetRegions(region);
    templateImg->Allocate();

    // Fill in the image data
    InputImageType::IndexType pixelIndex;
    for (i = 0; i < dim; i++)
        {
        double ii = i - center;
        for (j = 0; j < dim; j++)
            {
            double jj = j - center;
            double d = sqrt (ii*ii + jj*jj);
            // d *= 5.0 / radius; // normalize to original case
            int p;
            d = minVal + scale*pow(d,expon);
            if (d >= maxVal) d = 0;
            p = (d+0.5);

            pixelIndex[0] = i;
            pixelIndex[1] = j;
            templateImg->SetPixel (pixelIndex, p);
            }
        }

    }  // end of makeTemplateImage


static void
atomCorr ( 
  double atomTemplateRadius,
  const char * inImgFN, 
  const char *outImgFN 
  )
    {
    InputImageType::Pointer inImg;
    InputImageType::Pointer inTemplateImg;
    FloatImageType::Pointer fltInImg;
    FloatImageType::Pointer fltTemplateImg;
    FloatImageType::Pointer corrImg1;
    FloatImageType::Pointer invertedCorrImg1;
    FloatImageType::Pointer corrImg2;
    FloatImageType::Pointer modCorrImg1;


    readImage (inImgFN, inImg);
    // makeTemplateImage (3.5, 39.0, 255.0, 2.0, inTemplateImg);
    makeTemplateImage (atomTemplateRadius, 39.0, 255.0, 2.0, inTemplateImg);

    convertImgType (inTemplateImg, fltTemplateImg);
    convertImgType (inImg, fltInImg);

    // printImageInfo ("input img", inImg);
    // printImageInfo ("template img", inTemplateImg);

    correlateImg (inImg, inTemplateImg, corrImg1);

    // printImageInfo ("correlation img", corrImg1);

    modByStdDev (inImg, corrImg1, modCorrImg1);

    FloatImageType::Pointer stretchImg;
    stretch (modCorrImg1, 0.4, 1.0, stretchImg );

    // printImageInfo ("mod corr 1 img", stretchImg);

    invertImg (stretchImg, invertedCorrImg1);

    correlateImg (invertedCorrImg1, fltTemplateImg, corrImg2);

    writeFloatImage (outImgFN, corrImg2);

    return ;
    } // end of atomCorr













// ..........................................
//
// Here are the functions that connect up the plugin Qt framework to
// the itk-based functions

static void
atomCorrelateImages
  (
  double atomTemplateRadius,
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
    QString opTag; 
    QString extension; 
    QString outExtension; 

    // we parse the first file name to see if it conforms
    // to our naming scheme.  This is signaled by whether
    // seqNumDigits > 0.
    parseFileName ( inputFileList[0], 
                    dirName, seqNum, seqNumDigits, 
                    outProjTag, opTag, extension);

    if (outProjTag == "")
        {
        outProjTag = projectShortTag; 
        }
    outProjTag += ".atomCorr";

    outExtension = "tif";

    bool deriveSeqNumFromInputFN = seqNumDigits > 0;

    
    writeLog (QString ("\nAtomCorrelation: Begin...\n"));
    for (int i=0; i < inputFileList.size(); i++)
        {
        QString outImgFN;

        makeOutFN ( inputFileList[i].filePath(), outputDir,
                    deriveSeqNumFromInputFN, i,
                    outProjTag, outExtension, outImgFN );

        writeLog ("AtomCorrelation: Processing image " + 
                                    inputFileList[i].filePath() + "\n");
        atomCorr (  atomTemplateRadius,
                    inputFileList[i].filePath().toStdString().c_str(), 
                    outImgFN.toStdString().c_str()     );
        qDebug () << "  " << i << "  processing " << 
                                      inputFileList[i].filePath();
        }
    writeLog (QString ("AtomCorrelation: Done.\n\n"));


    return;

    }  // end of atomCorrelateImages




void FilterAtomCorrelation::execute
           (const QVector<QMap<QString, QVariant> >& parameters) 
{
    CurrentModuleInstance = this;  // this should be first line in execute

    // std::cout << "\n>>> BLOCK " << getName().toStdString() << std::endl;


   QString projectShortTag = getProject()->getShortTag();
   if (projectShortTag == "")
   {
        projectShortTag = "p";
   }
   // qDebug() << "Project short tag is |" + projectShortTag + "|";



    //
    QString inputDir;   
    QString outputDir;
    double atomTemplateRadius = 3.0;  // smaller radius seems to work better
   
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
        else if (paramName == "atomTemplateRadius")
        {
            atomTemplateRadius = parameters[i]["value"].toFloat();
        }

        else
        {
            // error
        }
      

    }  // end of loop over parameters





    // qDebug() << "EXECUTE PARAMS >>>";  
    // qDebug() << inputDir;
    // qDebug() << outputDir;



    if ( ! isDirectory (inputDir) )
        {
        writeLog ("AtomCorrelation: Input folder does not exist: " + inputDir);
        return;
        }
    else if ( ! mkDirectory (outputDir) )
        {
        writeLog (
          "AtomCorrelation: Error accessing or creating output folder: " + 
          outputDir);
        return;
        }


    QFileInfoList inputFileList;
    getImageFileList (inputDir, inputFileList);

    for (int i=0; i < inputFileList.size(); ++i)
    {
        QFileInfo fileInfo = inputFileList.at(i);
        // qDebug() << "FILE NAME " << fileInfo.fileName();
    }



    

    qDebug () << "Begin atom correlation....\n";

    // processing code below
    //
    atomCorrelateImages (atomTemplateRadius, inputFileList, 
                                    outputDir, projectShortTag);

    qDebug () << "Done with atom correlation.\n";

}  // end of FilterAtomCorrelation::execute


