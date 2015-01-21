
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
#include "filter_support.h"

/////////////////////////////////////////////////////////////////////
//
// This is the source code for a plugin that generates correlation images 
// based on a set of input images and the specification of a template
// for an atom image.
//
// All plugins are sub-classes of the class FilterInterface. Note that
// the plugin scheme is based on the Qt library. So there are parts of 
// the code that make considerable use of Qt classes and functions.
//
// The main part of writing a plugin is coding the method called "execute".
//
// The code here (and in corresponding .h file) implements a class
// called "FilterAtomCorrelation".  In this source file, essentially
// all of the code serves the implementation of 
// FilterAtomCorrelation::execute.
//
// To understand this code, you should probably go to 
// FilterAtomCorrelation::execute and follow the code from there.
//
// Only the method FilterAtomCorrelation::execute is actually part of 
// the class.  The supporting functions are simply static functions
// that are accessible only from within this source file.
//
// The organization of this code is in a traditional C style in that
// it has the higher-level functions near the end, with the lower 
// level funtions near the top.
//
// Also, it should be noted that this code was originally developed
// as a stand-alone program and it was then adapted to be a plugin.
// This may account for some aspects of the organization.
//
// Some of the functions here are probably useful within other plugins
// and these should be moved to a separate library where they could
// used without duplicating code. But for now, we are duplicating the
// code.  It's not ideal, but it's no big deal (yet).
//
//
/////////////////////////////////////////////////////////////////////


//
// The CurrentModuleInstance and static writeLog function
// allow static methods in this file to write log messages
//
FilterAtomCorrelation* CurrentModuleInstance = NULL;

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



// Essentially all of the ITK classes are based on templates.
// Typical ITK style makes great use of typedefs.
//
// We're assuming that the input files are 8-bit grey-level.
// Is this a valid assumption?
//
// Output is going to be floating point.

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


// Do a contrast stretch on a floating point correlation image.
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
// several ITK-based debug output functions

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


// Now we're getting into the code that is specific to the correlation method


// This crops the correlation image to account for differences in 
// sizes between the input and the correlation image.
// There's something weird going on here.....
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



// This crops the correlation image to account for differences in 
// sizes between the input and the correlation image.
// There's something weird going on here.....
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



// Do the correlation of the two input images.
// This version takes two uchar images and produces a float image.
// See other version of this below.  
// (Should these be combined? Are both used?)
static int correlateImg 
  ( 
  InputImageType::Pointer & inImg, 
  InputImageType::Pointer & inTemplateImg, 
  FloatImageType::Pointer & outImg 
  )
{

#if 0
    printf ("IIF\n");
    printImageInfo ("inImg", inImg);
    printImageInfo ("inTemplateImg", inTemplateImg);
#endif

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


// Force the two images to be in the same coordinate system.
// ITK is very sensitive about this sort of thing.
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


// Modify the correlation image by enhancing regions of high standard deviation.
static int
modByStdDev (
  InputImageType::Pointer & inImg, 
  FloatImageType::Pointer & corrImg, 
  FloatImageType::Pointer & modImg
  )
    {
    typedef itk::NoiseImageFilter<
               InputImageType, FloatImageType >  NoiseFilterType;

    double radius = 15.0;  // should be parameterized

    NoiseFilterType::Pointer sdFilter = NoiseFilterType::New();

    sdFilter->SetInput ( inImg );
    sdFilter->SetRadius ( radius );

    FloatImageType::Pointer sdImg = sdFilter->GetOutput();
    sdImg->Update();


    FloatImageType::Pointer sdStretchImg;

    // stretch the std dev image.  
    // (note that this is disabled by the use of (0,1) range.).
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


// convert from uchar image to float image
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


// invert pixel values
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



// Do the correlation of the two input images.
// This version takes two float images and produces a float image.
// See other version of this above.
// (Should these be combined? Are both used?)
static int
correlateImg (
  FloatImageType::Pointer & inImg,
  FloatImageType::Pointer & templateImg,
  FloatImageType::Pointer & correlationImg
  )
    {

#if 0
    printf ("FFF\n");
    printImageInfo ("inImg", inImg);
    printImageInfo ("inTemplateImg", templateImg);
#endif

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




// Generate the atom template image on the fly......
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



// This is the main ITK-based correlation function. This is called by the
// Qt-based stuff, and it calls the ITK stuff.
// 
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
    FilterSupport::setNominalImageCoordSys ( inImg );



    // makeTemplateImage (3.5, 39.0, 255.0, 2.0, inTemplateImg);
    makeTemplateImage (atomTemplateRadius, 39.0, 255.0, 2.0, inTemplateImg);
    FilterSupport::setNominalImageCoordSys ( inTemplateImg );

    // So we now have the two images for correlation.
    // There are actually two correlation steps.

    // but first, we confer them both to floating point
    convertImgType (inTemplateImg, fltTemplateImg);
    convertImgType (inImg, fltInImg);

    // printImageInfo ("input img", inImg);
    // printImageInfo ("template img", inTemplateImg);

    // generate the correlation between the two inputs
    correlateImg (inImg, inTemplateImg, corrImg1);

    // printImageInfo ("correlation img", corrImg1);

    // enhance the intermediate correlation to emphasize regions
    // of high variance in the original input image
    modByStdDev (inImg, corrImg1, modCorrImg1);

    // do an additional constrast stretch of the image
    FloatImageType::Pointer stretchImg;
    stretch (modCorrImg1, 0.4, 1.0, stretchImg );

    // printImageInfo ("mod corr 1 img", stretchImg);

    // invert the pixel values of the correlation image because
    // we expect the atoms to be dark in the middle, but the correlation
    // image has bright spots where we think the atoms are.
    invertImg (stretchImg, invertedCorrImg1);

    FilterSupport::setNominalImageCoordSys ( invertedCorrImg1 );

    // do the correlation again with the inverted correlation image
    correlateImg (invertedCorrImg1, fltTemplateImg, corrImg2);

    FilterSupport::setNominalImageCoordSys ( corrImg2 );


    // Done! Write it out.
    writeFloatImage (outImgFN, corrImg2);

    return ;
    } // end of atomCorr



// ..........................................
// ..........................................
//
//
// Here are the functions that connect up the plugin Qt framework to
// the itk-based functions

// Generate a set of correlation images based on a list of input image
// files and an atom radius.  Write the correlation images to the given
// output directory using our naming convention.
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
    FilterSupport::parseFileName ( inputFileList[0],
                                   dirName, seqNum, seqNumDigits,
                                   outProjTag, opTag, extension);

    // We use the "project tag" from the incoming files if it is present.
    // If it isn't, we use the tag the current project.
    // Maybe we should use the tag from the current project all the time.
    if (outProjTag == "")
        {
        outProjTag = projectShortTag; 
        }
    outProjTag += ".atomCorr";

    outExtension = "tif";

    bool deriveSeqNumFromInputFN = seqNumDigits > 0;

    
    // Tell the user that we're begining processing.
    writeLog (QString ("\nAtomCorrelation: Begin...\n"));
    writeLog (QString ("AtomCorrelation: Processing images in directory ") + 
                        dirName + "\n");

    // loop through all of the input images
    for (int i=0; i < inputFileList.size(); i++)
        {
        QString outImgFN;

        FilterSupport::makeOutFN ( inputFileList[i].filePath(), outputDir,
                                   deriveSeqNumFromInputFN, i,
                                   outProjTag, outExtension, outImgFN );

        writeLog ("AtomCorrelation:      " + 
                        inputFileList[i].fileName() + "\n");

        // Do the correlation, write the output file.
        atomCorr (  atomTemplateRadius,
                    inputFileList[i].filePath().toStdString().c_str(), 
                    outImgFN.toStdString().c_str()     );

        qDebug () << "  " << i << "  processing " << 
                                      inputFileList[i].filePath();

        }
    writeLog (QString ("AtomCorrelation: Done.\n\n"));


    return;

    }  // end of atomCorrelateImages


//.......

// Here's the method that gets called when the user presses 
// the "Execute" button.
//
void FilterAtomCorrelation::execute
           (const QVector<QMap<QString, QVariant> >& parameters) 
{

    CurrentModuleInstance = this;  // this is just to get writeLog to work.
                                   

    qDebug() << "Entering module: " + getMetaData()->getName() + "\n";


   QString projectShortTag = getProject()->getShortTag();
   if (projectShortTag == "")
   {
        // I hope that this will never happen
        projectShortTag = "p";
   }
   // qDebug() << "Project short tag is |" + projectShortTag + "|";



    // Get the parameters values....

    QString inputDir;   
    QString outputDir;
    double atomTemplateRadius = 3.0;  // smaller radius seems to work better
   
    // The parameters are just a list of QMaps where each
    // map is just a set of (keyword, value) pairs.
    // The keyword is always a QString.  The value can be of different
    // types, however I think that it is actually always a QString.

    // Loop through the parameters looking for the parameters that we need.
    for (int i=0; i<parameters.size(); ++i)
    {
        // qDebug() << parameters[i]["name"].toString();

        // For each parameter, we are really only interested in 
        // the "name" and "value".

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
            // error? 
        }
      

    }  // end of loop over parameters

    // write out the parameters to a parameter file in the outputDir.
    writeParameters (parameters, outputDir);




    // qDebug() << "EXECUTE PARAMS >>>";  
    // qDebug() << inputDir;
    // qDebug() << outputDir;



    if ( ! FilterSupport::isDirectory (inputDir) )
        {
        // if input directory doesn't exist, then exit
        writeLog ("AtomCorrelation: Input folder does not exist: " + inputDir);
        return;
        }
    else if ( ! FilterSupport::mkDirectory (outputDir) )
        {
        // if output directory doesn't exist, then make it
        writeLog (
          "AtomCorrelation: Error accessing or creating output folder: " + 
          outputDir);
        return;
        }


    // get the list of input files
    QFileInfoList inputFileList;
    FilterSupport::getImageFileList (inputDir, inputFileList);

    for (int i=0; i < inputFileList.size(); ++i)
    {
        QFileInfo fileInfo = inputFileList.at(i);
        // qDebug() << "FILE NAME " << fileInfo.fileName();
    }


    qDebug () << "Begin atom correlation....\n";

    // do the correlations!
    atomCorrelateImages (atomTemplateRadius, inputFileList, 
                                    outputDir, projectShortTag);

    qDebug () << "Done with atom correlation.\n";

    qDebug () << "Exiting module: " + getMetaData()->getName() + "\n";

}  // end of FilterAtomCorrelation::execute


