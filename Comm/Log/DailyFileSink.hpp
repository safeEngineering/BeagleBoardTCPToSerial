/************************************************************
 * DailyFileSink.h
 * Data Dump Log Daily Rotating Log File Implementation
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		26th Mar 2018	0.0.5    Official Release to Aurzion
 ************************************************************/

#ifndef __DAILYFILESINK_H_
#define __DAILYFILESINK_H_

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/details/file_helper.h"
#include <chrono>
#include <mutex>
#include <regex>
#include <string>
#include <dirent.h>

namespace SafeEngineering
{

/*
* Rotating file sink based on date.
*/
template<class Mutex>
class DailyFileSink :public spdlog::sinks::base_sink < Mutex >
{
public:
    //create daily file sink which rotates on given time
    DailyFileSink(const std::string &strBaseDir, const std::string &strBaseFilename,
            const std::string &strExtension, uint32_t nMaxDays, bool bForceFlush = false);

    void flush() override;

protected:
    void _sink_it(const spdlog::details::log_msg &msg) override;

private:
    std::chrono::system_clock::time_point nextRotation();

    //Create filename for the form basename.YYYY-MM-DD.extension
    static std::string calcFilename(const std::string &strBaseDir, const std::string &strBaseFile,
            const std::string &strExtension);

    std::string m_strBaseDir;
    std::string m_strBaseFilename;
    std::string m_strExtension;
    uint32_t m_nMaxDays;    // Log files older than this will be deleted one at a time.
    std::chrono::system_clock::time_point m_rotationTP;
    spdlog::details::file_helper m_fileHelper;
};

template<class Mutex>
DailyFileSink<Mutex>::DailyFileSink(const std::string &strBaseDir,
        const std::string &strBaseFilename, const std::string &strExtension, uint32_t nMaxDays,
        bool bForceFlush) :
        m_strBaseDir(strBaseDir), m_strBaseFilename(strBaseFilename), m_strExtension(strExtension),
        m_nMaxDays(nMaxDays), m_fileHelper(bForceFlush)
{
    m_rotationTP = nextRotation();
    m_fileHelper.open(calcFilename(m_strBaseDir, m_strBaseFilename, m_strExtension));
}

template<class Mutex>
void DailyFileSink<Mutex>::flush()
{
    m_fileHelper.flush();
}

template<class Mutex>
void DailyFileSink<Mutex>::_sink_it(const spdlog::details::log_msg& msg)
{
    if (std::chrono::system_clock::now() >= m_rotationTP)
    {
        std::regex expLog("^" + m_strBaseFilename + "_\\d{4}-\\d{2}-\\d{2}\\." + m_strExtension + "$");
        std::vector<std::string> logFiles;

        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(m_strBaseDir.c_str())))
        {
            while ((ent = readdir(dir)))
            {
                std::smatch smLog;
                std::string strFilename = ent->d_name;
                if (std::regex_match(strFilename, smLog, expLog))
                {
                    logFiles.push_back(strFilename);
                }
            }
            closedir(dir);
            if (logFiles.size() > m_nMaxDays)
            {
                // Delete the oldest log files if there are too many
                std::sort(logFiles.begin(), logFiles.end());
                for (uint32_t i=0; i<logFiles.size()-m_nMaxDays; i++)
                {
                    std::string strPath = m_strBaseDir + "/" + logFiles[i];
                    std::remove(strPath.c_str());
                }
            }
        }
        else
        {
            // could not open directory
        }

        m_fileHelper.open(calcFilename(m_strBaseDir, m_strBaseFilename, m_strExtension));
        m_rotationTP = nextRotation();
    }

    m_fileHelper.write(msg);
}

template<class Mutex>
std::chrono::system_clock::time_point DailyFileSink<Mutex>::nextRotation()
{
    auto now = std::chrono::system_clock::now();
    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm date = spdlog::details::os::localtime(tnow);
    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    auto rotationTime = std::chrono::system_clock::from_time_t(std::mktime(&date));
    if (rotationTime > now)
    {
        return rotationTime;
    }
    else
    {
        return std::chrono::system_clock::time_point(rotationTime + std::chrono::hours(24));
    }
}

//Create filename for the form basename.YYYY-MM-DD.extension
template<class Mutex>
std::string DailyFileSink<Mutex>::calcFilename(const std::string& strBaseDir,
        const std::string& strBaseFile, const std::string& extension)
{
    std::tm tm = spdlog::details::os::localtime();
    fmt::MemoryWriter w;
    w.write("{}/{}_{:04d}-{:02d}-{:02d}.{}", strBaseDir, strBaseFile, tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, extension);
    return w.str();
}

typedef DailyFileSink<std::mutex> DailyFileSink_mt;
typedef DailyFileSink<spdlog::details::null_mutex> DailyFileSink_st;

} /* namespace aurizon */

#endif /* __DAILYFILESINK_H_ */
