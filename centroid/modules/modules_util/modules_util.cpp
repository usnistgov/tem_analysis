#include "modules_util.h"

#include "itkImage.h"
#include "itkImageFileReader.h"

#include <QtCore>
#include <QtGlobal>
#include <QDebug>

bool
ModulesUtil::isFile (const QString & filePath)
    {
    QFileInfo info = QFileInfo (filePath);
    return info.isFile ();
    } // end of isFile

bool
ModulesUtil::isDirectory (const QString & dirPath)
    {
    QFileInfo info = QFileInfo (dirPath);
    return info.isDir ();
    }  // end of isDirectory

bool
ModulesUtil::mkDirectory (const QString & dirPath)
    {
    // qDebug () << "mkDir " + dirPath;

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

    // qDebug () << "about to mkDir " + dirPath;
    QDir qdir = QDir (dirPath);


    return qdir.mkpath (".");

    } // end of mkDirectory

void
ModulesUtil::parseFileName 
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

void
ModulesUtil::getFilteredFileList
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

void
ModulesUtil::getImageFileList
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



void
ModulesUtil::getXformFileList
  (
  const QString dirPath,
  QFileInfoList & fileList
  )
    {
    QStringList filters;
    filters << "*.xform" << "*.XFORM";

    getFilteredFileList (dirPath, filters, fileList);
    } // end of getXformFileList


void
ModulesUtil::getAtomPosFileList
  (
  const QString dirPath, 
  QFileInfoList & fileList
  )
    {
    QStringList filters;
    filters << "*.apos" << "*.APOS";

    getFilteredFileList (dirPath, filters, fileList);
    } // end of getAtomPosFileList





#if 0
void
ModulesUtil::makeOutFN 
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
#endif


void
ModulesUtil::makeOutFN 
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




void
ModulesUtil::makeOutFN 
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

bool
ModulesUtil::imgIsGray (const char *imgFN)
    {
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

bool
ModulesUtil::filesAreSame (const char *fnA, const char *fnB)
    {
    // printf ("filesAreSame ( %s , %s )\n", fnA, fnB );

    unsigned char *a, *aa, *b, *bb;
    unsigned long lenA, lenB;
    FILE *fpA, *fpB;
    bool rtn;


    fpA = fopen (fnA, "rb");
    fpB = fopen (fnB, "rb");

    if (fpA == NULL)
        {
        // fprintf (stderr, "Can't open file %s or %s\n", fnA, fnB);
        qWarning() << "Can't open file " + QString(fnA);
        exit (-1);
        }

    if (fpB == NULL)
        {
        qWarning() << "Can't open file " + QString(fnB);
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
        qWarning () << "Bad read of A " + QString(fnA);
        exit (-1);
        }

    if (fread (b, sizeof(unsigned char), lenB, fpB) != lenB)
        {
        qWarning () << "Bad read of B " + QString(fnB);
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





