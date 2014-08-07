
#include <QDebug>
#include <QSettings>
#include <QDateTime>

#include "interface.h"

void ModuleInterface::setMetaData(const QJsonObject & loaderMetaData ) 
{


    const QVariantMap interfaceMetaData = 
        loaderMetaData.value("MetaData").toObject().toVariantMap();

        // loaderMetaData.value("MetaData").toObject().toVariantMap()
    

    // qDebug () << interfaceMetaData;

    // extract various values from the interfaceMetaData map

    if (interfaceMetaData.contains("name"))
    {
        name = interfaceMetaData.value("name").toString();
    }

    if (interfaceMetaData.contains("seq"))
    {
        sequenceNumber = interfaceMetaData.value("seq").toInt ();
    }

    if (interfaceMetaData.contains("version"))
    {
        version = interfaceMetaData.value("version").toString ();
    }

    if (interfaceMetaData.contains("shortTag"))
    {
        shortTag = interfaceMetaData.value("shortTag").toString ();
    }

    if (interfaceMetaData.contains("type"))
    {
        moduleType = interfaceMetaData.value("type").toString ();
    }

    if (shortTag == "")
    {
        shortTag = name;
    }

#if 0
    qDebug () << "";
    qDebug () << "setMetaData name " + name;
    qDebug () << "setMetaData sequenceNumber " + sequenceNumber;
    qDebug () << "setMetaData version " + version;
    qDebug () << "setMetaData shortTag " + shortTag;
    qDebug () << "setMetaData moduleType " + moduleType ;
    qDebug () << "";
#endif

}  // end of  void ModuleInterface::setMetaData(const QVariantMap & )


void FilterInterface::setMetaData(const QJsonObject & loaderMetaData ) 
{
    // I had originally thought that I would do more here.....
    ModuleInterface::setMetaData (loaderMetaData);
}


void FilterInterface::writeParameters 
       (const QVector<QMap<QString, QVariant> > & parameters, 
        const QString & dirName )
{

    // qDebug () << "Entered FilterInterface::writeParameters \n";

    QString projTag = getProject()->getShortTag();

    QString outDir = (dirName == "") ?  
               getProject()->getBaseDirectory().absolutePath() : dirName;

    // we'll use the interfaces shortTag and 

    QString startFilter = shortTag + "." + projTag + ".";
    QString wildCard = "????";
    QString endFilter = ".params";
    int startLen = startFilter.size();
    int endLen = endFilter.size();
    int minFNLen = startLen + 1 + endLen;
    QStringList nameFilter ( startFilter + wildCard + endFilter );

#if 0
    QDir qdir;
    dir.setFilter (QDir::Files);
    dir.setSorting (QDir::Name);
    dir.setPath (outDir);
    dir.setNameFilters (nameFilter);
    QFileInfoList fileList = dir.entryInfoList ();
    for (int i = 0; i < fileList.size(); i++)
        {
        QFileInfo fileInfo = fileList.at(i);

        figure out max seq num
        }

#else

#define MAX(x,y) (((x)>(y))?(x):(y))

    QDir qdir = QDir (outDir) ;
    QStringList fnList = qdir.entryList (nameFilter, QDir::Files, QDir::Name);
    int maxSeqNum = -1;
    for (int i = 0; i < fnList.size(); i++)
        {
        // qDebug () << "param file " + fnList.at(i);
        QString fn = fnList.at(i);
        if ( fn.size() > (startLen+endLen) )
            {
            
            QString seqStr = fn.mid (startLen, fn.size() - (startLen+endLen) );
            
            bool isInt;
            int seqNum = seqStr.toInt (&isInt);
            if (isInt)
                {
                maxSeqNum = MAX (maxSeqNum, seqNum);
                }
            }
        }


    char seqNumStr[10];
    sprintf (seqNumStr, "%04d", maxSeqNum+1);
    QString outFN = startFilter + seqNumStr + endFilter;
    // qDebug () << "output params file = " + outFN;

    QString fullFN = qdir.absolutePath() + "/" + outFN;


    QSettings settings (fullFN, QSettings::IniFormat);

    settings.setValue ("__DATE__", 
              QDateTime::currentDateTime().toString("dd.MM.yyyy_hh:mm:ss"));

    // loop through the parameters and set all the parameter values
    for (int i=0; i<parameters.size(); ++i)
        {
        settings.setValue ( parameters[i]["name"].toString(),
                            parameters[i]["value"].toString() );
        }
   

    // it syncs the settings to disk in the QSettings destructor



#endif

} // end of writeParameters

