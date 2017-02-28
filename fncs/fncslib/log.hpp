#ifndef _LOG_HPP_
#define _LOG_HPP_

#include "log.h"

#if (defined WIN32 || defined _WIN32)
#   if defined LIBFNCS_STATIC
#       define FNCS_EXPORT
#   elif defined LIBFNCS_EXPORTS
#       define FNCS_EXPORT __declspec(dllexport)
#   else
#       define FNCS_EXPORT __declspec(dllimport)
#   endif
#else
#   define FNCS_EXPORT
#endif

class Output2Tee
{
public:
    static FILE*& Stream1()
    {
        static FILE* pStream = NULL;
        return pStream;
    }

    static FILE*& Stream2()
    {
        static FILE* pStream = NULL;
        return pStream;
    }

    static void Output(const std::string& msg)
    {
        FILE* pStream1 = Stream1();
        if (!pStream1)
            return;
        fprintf(pStream1, "%s", msg.c_str());
        fflush(pStream1);

        FILE* pStream2 = Stream2();
        if (!pStream2)
            return;
        fprintf(pStream2, "%s", msg.c_str());
        fflush(pStream2);
    }
};

class FNCS_EXPORT FNCSLog : public Log<Output2Tee> {};

#ifndef FNCS_LOG_MAX_LEVEL
#define FNCS_LOG_MAX_LEVEL logDEBUG4
#endif

#define FNCS_LOG(level) \
    if (level > FNCS_LOG_MAX_LEVEL) ;\
    else if (level > FNCSLog::ReportingLevel() || !Output2Tee::Stream1()) ; \
    else FNCSLog().Get(level)

#define LERROR FNCS_LOG(logERROR)
#define LWARNING FNCS_LOG(logWARNING)
#define LINFO FNCS_LOG(logINFO)
#define LDEBUG FNCS_LOG(logDEBUG)
#define LDEBUG1 FNCS_LOG(logDEBUG1)
#define LDEBUG2 FNCS_LOG(logDEBUG2)
#define LDEBUG3 FNCS_LOG(logDEBUG3)
#define LDEBUG4 FNCS_LOG(logDEBUG4)

#endif /* _LOG_HPP_ */
