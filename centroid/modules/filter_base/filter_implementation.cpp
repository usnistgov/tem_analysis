#include "filter_implementation.h"

#include <QSettings>
#include <QPlainTextEdit>

#include <QtCore>
#include <QtGlobal>
#include <QDebug>

FilterImplementation::FilterImplementation()
{
}

void FilterImplementation::writeLog ( const QString & message )
{
    if (logs) {
        logs->insertPlainText(message);
        QTextCursor c = logs->textCursor();
        c.movePosition(QTextCursor::End);
        logs->setTextCursor(c);
        logs->repaint (); // make the output appear immediately
                          // do we need something like: qApp->processEvents()?
        logs->viewport()->update();
    }

    qDebug() << message;
}

void FilterImplementation::writeParameters (
    const QVector<QMap<QString, QVariant> > & parameters,
    const QString & dirName )
{
    // qDebug () << "Entered FilterInterface::writeParameters \n";

    QString projTag = getProject()->getShortTag();

    QString outDir = (dirName == "") ?  
               getProject()->getBaseDirectory().absolutePath() : dirName;

    // we'll use the interfaces shortTag and 

    QString startFilter = getMetaData()->getShortTag() + "." + projTag + ".";
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
                maxSeqNum = qMax (maxSeqNum, seqNum);
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


