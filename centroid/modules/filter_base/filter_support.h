#ifndef __H__FILTER_SUPPORT__
#define __H__FILTER_SUPPORT__

#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>

#include "itkImage.h"

#include "project.h"



typedef itk::Image< unsigned char,  2 >   UCharImageType;
typedef itk::Image< float, 2 >   FltImageType;


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
   static void setNominalImageCoordSys ( UCharImageType::Pointer & img );
   static void setNominalImageCoordSys ( FltImageType::Pointer & img );
};

#endif // __H__FILTER_SUPPORT__

