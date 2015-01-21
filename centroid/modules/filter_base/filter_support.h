#ifndef __H__FILTER_SUPPORT__
#define __H__FILTER_SUPPORT__

#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>

#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkRGBAPixel.h"

#include "project.h"



typedef itk::Image< unsigned char,  2 >   UCharImageType;
typedef itk::Image< unsigned short,  2 >   UShortImageType;
typedef itk::Image< float, 2 >   FltImageType;

typedef itk::RGBPixel<unsigned char>  UCharRGBPixelType;
typedef itk::RGBAPixel<unsigned char>  UCharRGBAPixelType;
typedef itk::Image< UCharRGBPixelType,  2 >   UCharRGBImageType;
typedef itk::Image< UCharRGBAPixelType,  2 >   UCharRGBAImageType;

class FilterSupport
{
public:
   static bool isFile (const QString & filePath);
   static bool isDirectory (const QString & dirPath);
   static bool mkDirectory (const QString & dirPath);


   static void parseFileName (const QFileInfo fileInfo, QString & dirName,
      int & seqNum, int & seqNumDigits, QString &projTag, QString &opTag,
      QString &extension);

   static bool filesAreSame (const char *fnA, const char *fnB);


   static void getFilteredFileList (const QString & dirPath,
      const QStringList & fnFilters, QFileInfoList & imgFileList);

   static void getImageFileList (const QString dirPath,
                                 QFileInfoList & fileList);

   static void getXformFileList (const QString dirPath,
                                 QFileInfoList & fileList);

   static void getAtomPosFileList (const QString dirPath,
                                   QFileInfoList & fileList);


   static void makeOutFN (const char *inImgFN, const char *outDir,
      const bool deriveSeqNumFromInputFN, const int altSeqNum,
      const char *outImgFNTag, const char *outExt, char *outImgFN);

   static void makeOutFN (const QString & fileName,
      const QString & outputDir, const bool deriveSeqNumFromInputFN,
      const int altSeqNum, const QString & outImgFNTag,
      const QString & outExt, QString & outImgFN);


   static bool imgIsGray (const char *imgFN);

   // There must be a better way than repeating the same code N times.
   static void setNominalImageCoordSys ( UCharImageType::Pointer & img );
   static void setNominalImageCoordSys ( FltImageType::Pointer & img );
   static void setNominalImageCoordSys ( UShortImageType::Pointer & img );
   static void setNominalImageCoordSys ( UCharRGBImageType::Pointer & img );
   static void setNominalImageCoordSys ( UCharRGBAImageType::Pointer & img );
};

#endif // __H__FILTER_SUPPORT__

