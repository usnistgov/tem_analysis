#include "main_window.h"
#include "log.h"
using namespace logging;

#include <QtGlobal>

#include <functional>

static logging::Logger* s_logger;

//////////////////////////////////////////////////////////////////////////

static void messageLogger(QtMsgType type, const QMessageLogContext &context,
                          const QString &msg)
{

   QByteArray localMsg = msg.toLocal8Bit();

   switch(type) {
   case QtDebugMsg:
       LOG_DEBUG((*s_logger), "%s(%s:%u) [%s]", localMsg.constData(),
                 context.file, context.line, context.function);
       break;

   case QtWarningMsg:
       LOG_WARN((*s_logger), "%s(%s:%u) [%s]", localMsg.constData(),
                context.file, context.line, context.function);
       break;

   case QtCriticalMsg:
       LOG_ERROR((*s_logger), "%s(%s:%u) [%s]", localMsg.constData(),
                 context.file, context.line, context.function);
       break;

   case QtFatalMsg:
       LOG_FATAL((*s_logger), "%s(%s:%u) [%s]", localMsg.constData(),
                 context.file, context.line, context.function);
       abort();
       break;
   }
}

int main(int argc, char *argv[])  
{
   QSharedPointer<LogPolicyInterface> policy(new FileLogPolicy);
   if (!policy->open("tem_analysis.log")) {
      qFatal("Cannot open tem_analysis.log");
   }

   s_logger = new Logger(LOG_TRACE, policy);
   LOG_INFO((*s_logger), "centroid starting up; logging initialized");
   qInstallMessageHandler(messageLogger);


   QApplication app(argc, argv);
   app.setOrganizationName("NIST");
   app.setOrganizationDomain("nist.gov");
   app.setApplicationName("TEM Analysis");

   MainWindow mainWin;
   mainWin.show();

   return app.exec();
}
