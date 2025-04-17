#pragma once

extern int s_oomBellaLogContext; //  Declare the s_dlLogContext variable

namespace oom {
    namespace bella {

        // Define color codes
        #define OOMRESET   "\033[0m"
        #define OOMRED     "\033[31m"
        #define OOMGREEN   "\033[32m"
        #define OOMYELLOW  "\033[33m"
        #define OOMBLUE    "\033[34m"

        // In your log function:
        static void log(void* /*ctx*/, dl::LogType type, const char* msg)
        {
            switch (type)
            {
            case dl::LogType_Info:
                DL_PRINT(OOMGREEN "[INFO] %s" OOMRESET "\n", msg);
                break;
            case dl::LogType_Warning:
                if (strcmp(msg, "Redefining arg with long form: 'input'") == 0) {
                    break;
                } else if (strcmp(msg, "Core node implementations are not linked.") == 0) {
                    break;
                }
                DL_PRINT(OOMYELLOW "[WARN] %s" OOMRESET "\n", msg);
                break;
            case dl::LogType_Error:
                DL_PRINT(OOMRED "[ERROR] %s" OOMRESET "\n", msg);
                break;
            case dl::LogType_Custom:
                DL_PRINT(OOMBLUE "%s\n" OOMRESET, msg);
                break;
            }
        }

    }
}