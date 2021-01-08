#ifndef GLOG_H
#define GLOG_H

#include <string>
#include <iostream>

class GLog
{
public:
    enum GLogType
    {
        kInfo,
        kWarning,
        kError
    };


    template<typename... Targs> static void Log(GLogType logT, Targs... log)
    {
        std::string tag;
        switch (logT)
        {
            case GLogType::kInfo:
            tag = "INFO : ";
            break;
            case GLogType::kWarning:
            tag = "WARN : ";
            break;
            case GLogType::kError:
            tag = "ERRO : ";
            break;
        }
        std::cout << tag;
        for(auto msg : {log...})
        {
            std::cout << std::to_string(msg);
        }
        std::cout << std::endl;

    }

    template<typename... Targs> static void LogInfo(Targs... log)
    {
        Log(GLogType::kInfo, log...);
    }

    template<typename... Targs> static void LogWarning(Targs... log)
    {
        Log(GLogType::kWarning, log...);
    }

    template<typename... Targs> static void LogError(Targs... log)
    {
        Log(GLogType::kError, log...);
    }
};

#define CHECK_FAIL_LOG_INFO(CONDITION, LOG_STR, ...) \
    (CONDITION) ? true : (GLog::LogInfo(LOG_STR, ##__VA_ARGS__), false);

#define CHECK_FAIL_LOG_WARN(CONDITION, LOG_STR, ...) \
    (CONDITION) ? true : (GLog::LogWarning(LOG_STR, ##__VA_ARGS__), false);

#define CHECK_FAIL_LOG_ERROR(CONDITION, LOG_STR, ...) \
    (CONDITION) ? true : (GLog::LogError(LOG_STR, ##__VA_ARGS__), false);


#endif // GLOG_H
