
#include "execute.h"
#include "dmlog.hpp"
#include "dmstrtk.hpp"
#include "dmutil.h"
#include "dmflags.h"

DEFINE_string(PNAME, "dmconfigserver.exe", "process name");
DEFINE_int64(INDEX, 1, "index=1-255");
DEFINE_string(CMD, "start", "CMD={start,stop}");
int main( int argc, char* argv[] )
{
    Iexecute* execute = executeGetModule();

    if (NULL != execute)
    {
        std::string strVSpath =
            R"(D:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Team Tools\DiagnosticsHub\Collector\AgentConfigs\CPUUsageLow.json)";
        std::string strJson = R"(CpuUsageLow.json)";

        std::string strPath = getenv("path");
        std::string strCwd = DMGetRootPath();

        std::string strCmd = fmt::format(R"(tasklist | find "{}" | {})",
                                         FLAGS_PNAME, R"(awk "{ print $2 }")");
        std::string strRet = execute->exec(strCmd);

        if (strRet.empty())
        {
            fmt::print("{} {}", "cann't find", FLAGS_PNAME);
            return -1;
        }

        auto PID = fmt::to_number(strRet);

        std::string strVsp =
            R"(VSDiagnostics.exe {} {} /attach:{} /loadConfig:"{}")";

        std::string strVspCmd = fmt::format(strVsp, "start", FLAGS_INDEX, PID,
                                            strVSpath);
        std::string strVspRet = execute->exec(strVspCmd);

        std::string strVspStop =
            R"(VSDiagnostics.exe {} {} /output:{})";
        std::string strVspStopCmd = fmt::format(strVspStop, "stop", FLAGS_INDEX,
                                                strCwd + PATH_DELIMITER_STR + FLAGS_PNAME + "." + fmt::to_string(
                                                        FLAGS_INDEX) + ".diagsession");

        std::string strVspStopRet = execute->exec(strVspStopCmd);

    }

    return 0;
}
