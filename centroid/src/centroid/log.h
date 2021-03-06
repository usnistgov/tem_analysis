#ifndef __H_LOG__
#define __H_LOG__

#include <QMutex>
#include <QDateTime>

#include <stdexcept>

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

namespace logging {

    enum LogLevel {
        LOG_NONE,
        LOG_TRACE,
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL,
        LOG_NUM_LEVELS
    };


    class LogPolicyInterface {
    public:
        virtual bool open(char const* name) = 0;
        virtual void close() = 0;
        virtual void write(char const* message) = 0;
        virtual void write(char const* message, va_list ap) = 0;

        virtual ~LogPolicyInterface() {}
    };

    class FileLogPolicy : public LogPolicyInterface {
    public:
        virtual bool open(char const* name) {
            _fh = fopen(name, "wb");
            if (!_fh) return false;
            setvbuf(_fh, (char*)NULL, _IOLBF, 0);
            return true;
        }

        virtual void close() {
            if (_fh) {
                fclose(_fh);
                _fh = NULL;
            }
        }

        virtual void write(char const* message) {
            fprintf(_fh, message); // POSIX defines stdio as thread-safe
            fflush(_fh);
        }

        virtual void write(char const* message, va_list ap) {
            vfprintf(_fh, message, ap); // POSIX defines stdio as thread-safe
            fflush(_fh);
        }

        FileLogPolicy() : LogPolicyInterface(), _fh(NULL) {}
        virtual ~FileLogPolicy() { close(); }

    private:
        FileLogPolicy(FileLogPolicy const&);
        FileLogPolicy& operator=(FileLogPolicy const&);

        FILE* _fh;
    };

    class ConsoleLogPolicy : public FileLogPolicy {
    public:
        virtual bool open(char const* name = "stderr") {
            if (strcasecmp("stdout", name) == 0) _fh = stdout;
            else if (strcasecmp("stderr", name) == 0) _fh = stderr;
            else return false;

            return true;
        }

        virtual void close() {}

        ConsoleLogPolicy() : FileLogPolicy() {}
        virtual ~ConsoleLogPolicy() {}

    private:
        ConsoleLogPolicy(ConsoleLogPolicy const&);
        ConsoleLogPolicy& operator=(ConsoleLogPolicy const&);

        FILE* _fh;
    };

    class Logger {
    public:
        Logger(LogLevel level, QSharedPointer<LogPolicyInterface> const& policy)
            : _level(level), _policy(policy) {
        }

        template <LogLevel level>
        void print(char const* message, ...) {
            if (level < _level) return;
            QMutexLocker lock(&_write_mtx);

            static size_t const HEADER_LEN = 64;
            char header[HEADER_LEN];

            QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss-zzz");
            snprintf(header, HEADER_LEN, "%s [%-5s] : ",
                     now.toLocal8Bit().constData(), Logger::to_string(level));

            va_list ap;
            va_start(ap, message);

            _policy->write(header);
            _policy->write(message, ap);
            _policy->write("\r\n");

            va_end(ap);
        }

    private:
        LogLevel _level;
        QSharedPointer<LogPolicyInterface> _policy;
        QMutex _write_mtx;

        static char const* to_string(LogLevel level) {
            switch(level) {
            case LOG_NONE: return "NONE";
            case LOG_TRACE: return "TRACE";
            case LOG_DEBUG: return "DEBUG";
            case LOG_INFO: return "INFO";
            case LOG_WARN: return "WARN";
            case LOG_ERROR: return "ERROR";
            case LOG_FATAL: return "FATAL";
            case LOG_NUM_LEVELS:
            default: throw std::logic_error("Logger: invalid LogLevel");
            }
        }
    };

} // namespace log

#define LOG(logger, level, ...) logger.print<level>(__VA_ARGS__)
#define LOG_TRACE(logger, ...) logger.print<logging::LOG_TRACE>(__VA_ARGS__);
#define LOG_DEBUG(logger, ...) logger.print<logging::LOG_DEBUG>(__VA_ARGS__);
#define LOG_INFO(logger, ...) logger.print<logging::LOG_INFO>(__VA_ARGS__);
#define LOG_WARN(logger, ...) logger.print<logging::LOG_WARN>(__VA_ARGS__);
#define LOG_ERROR(logger, ...) logger.print<logging::LOG_ERROR>(__VA_ARGS__);
#define LOG_FATAL(logger, ...) logger.print<logging::LOG_FATAL>(__VA_ARGS__);

#endif // __H_LOG__

