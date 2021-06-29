
#include "execute.h"
#include "dmutil.h"
#include "dmlog.hpp"
#include "dmstrtk.hpp"
#include "dmflags.h"

DEFINE_string(PNAME, "dmconfigserver.exe", "process name");
DEFINE_int64(INDEX, 1, "index=1-255");
DEFINE_int64(TIME, 3600, "TIME={second}");
DEFINE_string(CONFIG, "cpuusagelow.json", "config name");

#include "dmutil.h"
#include "dmtimermodule.h"
#include "dmsingleton.h"
#include "dmthread.h"
#include "dmconsole.h"
#include "dmtypes.h"

class CMain : public IDMConsoleSink,
    public IDMThread,
    public CDMThreadCtrl,
    public CDMTimerNode,
    public TSingleton<CMain>
{
    friend class TSingleton<CMain>;

    typedef enum
    {
        eTimerID_UUID = 0,
        eTimerID_CRON = 1,
        eTimerID_STOP,
    } ETimerID;

    typedef enum
    {
        eTimerTime_UUID = 1000,
    } ETimerTime;

public:
    virtual void ThrdProc()
    {
        m_execute = executeGetModule();

        if (NULL != m_execute)
        {
            std::string strVSpathFmt =
                R"({}:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Team Tools\DiagnosticsHub\Collector\AgentConfigs\CPUUsageLow.json)";
            std::string strVSpath;

            for (char a = 'C'; a <= 'Z'; a++)
            {
                auto path = fmt::format(strVSpathFmt, a);

                if (DMIsFileExist(path))
                {
                    strVSpath = path;
                    break;
                }
            }

            if (strVSpath.empty())
            {
                fmt::print("{} {}", "cann't find", strVSpathFmt);
                return;
            }

            std::string strCmd = fmt::format(R"(tasklist | find "{}" | {})",
                                             FLAGS_PNAME, R"(awk "{ print $2 }")");
            std::string strRet = m_execute->exec(strCmd);

            if (strRet.empty())
            {
                fmt::print("{} {}", "cann't find", FLAGS_PNAME);
                return;
            }

            auto PID = fmt::to_number(strRet);

            std::string strVsp =
                R"(VSDiagnostics.exe {} {} /attach:{} /loadConfig:"{}")";

            std::string strVspCmd = fmt::format(strVsp, "start", FLAGS_INDEX, PID,
                                                strVSpath);
            std::string strVspRet = m_execute->exec(strVspCmd);

            SetTimer(eTimerID_STOP, FLAGS_TIME*1000);
        }

        std::cout << "test start" << std::endl;

        bool bBusy = false;

        while (!m_bStop)
        {
            bBusy = false;

            if (CDMTimerModule::Instance()->Run())
            {
                bBusy = true;
            }

            if (__Run())
            {
                bBusy = true;
            }

            if (!bBusy)
            {
                SleepMs(1);
            }
        }

        DispatchVsp();

        std::cout << "test stop" << std::endl;
    }

    virtual void Terminate()
    {
        m_bStop = true;
    }

    virtual void OnCloseEvent()
    {
        Stop();
    }

    virtual void OnTimer(uint64_t qwIDEvent, dm::any& oAny)
    {
        switch (qwIDEvent)
        {
        case eTimerID_STOP:
        {
            DispatchVsp();
            Stop();
        }
        break;

        default:
            break;
        }
    }

    void DispatchVsp()
    {
        if (!m_bVsp)
        {
            std::string strVspStop =
                R"(VSDiagnostics.exe {} {} /output:{})";
            std::string strVspStopCmd = fmt::format(strVspStop, "stop", FLAGS_INDEX,
                                                    DMGetRootPath() + PATH_DELIMITER_STR + FLAGS_PNAME + "." + fmt::to_string(
                                                            FLAGS_INDEX) + ".diagsession");

            std::string strVspStopRet = m_execute->exec(strVspStopCmd);
            m_bVsp = true;
        }
    }

private:
    CMain()
        : m_bStop(false), m_execute(nullptr), m_bVsp(false)
    {
        HDMConsoleMgr::Instance()->SetHandlerHook(this);
    }

    virtual ~CMain()
    {
        if (m_execute)
        {
            m_execute->Release();
        }
    }

private:
    bool __Run()
    {
        return false;
    }

private:
    volatile bool m_bStop;
    volatile bool m_bVsp;
    Iexecute* m_execute;
};

int main(int argc, char* argv[])
{
    DMFLAGS_INIT(argc, argv);
    CMain::Instance()->Start(CMain::Instance());
    CMain::Instance()->WaitFor();
    return 0;
}
