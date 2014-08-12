#ifndef __H__MODULES_UTIL__
#define __H__MODULES_UTIL__

#include <QFileInfo>
#include <QString>

#include "interface.h"

class ModulesUtil
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
};

#endif // __H__MODULES_UTIL__

