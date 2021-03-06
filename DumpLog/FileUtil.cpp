/************************************************************
 * FileUtil.h
 * Miscellaneous File Utilities
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		26th Mar 2018	0.0.5    Official Release to Aurzion
 ************************************************************/

#include "FileUtil.h"
#include <sys/stat.h>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iterator>

namespace aurizon
{

//Create the file system directory given by strPath if it does not exist already and apply the file permissions given in mode
int mkpath(const std::string &strPath, unsigned int mode)
{
    std::size_t startIndex = 0;
    std::size_t endIndex = 0;
    std::string currentPath;
    currentPath.reserve(strPath.size());
    struct stat buf;
    while (startIndex < strPath.size())
    {
        endIndex = strPath.find(PATH_SEP, startIndex);
        std::size_t length;
        if (endIndex == 0)
        {
            // root
            currentPath += PATH_SEP;
            startIndex++;
            continue;
        }
        else
        {
            if (endIndex == std::string::npos)
            {
                // last dir
                endIndex = strPath.size();
            }
            length = endIndex - startIndex;

            if (length == 0)
            {
                return -1;
            }

            std::string dir = strPath.substr(startIndex, length);
            currentPath += dir;

            if (stat(currentPath.c_str(), &buf) != 0)
            {
                if (mkdir(currentPath.c_str(), mode) != 0 && errno != EEXIST)
                {
                    return -1;
                }
            }
            else if (!S_ISDIR(buf.st_mode))
            {
                return -1;
            }
        }

        currentPath += PATH_SEP;
        startIndex = endIndex + 1;
    }
    return 0;
}

//Read the text file given by the strPath filepath (if it exists) and load its contents into strText parameter
bool loadText(const std::string &strPath, std::string &strText)
{
    std::ifstream in(strPath);
    if (in.is_open())
    {
        std::stringstream buffer;
        buffer << in.rdbuf();
        strText = buffer.str();
        return true;
    }
    else
    {
        return false;
    }
}

//Read the binary file given by the strPath filepath (if it exists) and load its contents into vData array parameter
bool loadBin(const std::string &strPath, std::vector<uint8_t> &vData)
{
    vData.clear();
    std::ifstream in(strPath, std::ios::binary);
    if (in.is_open())
    {
        in.unsetf(std::ios::skipws);

        std::streampos nFileSize;

        in.seekg(0, std::ios::end);
        nFileSize = in.tellg();
        in.seekg(0, std::ios::beg);

        vData.reserve(nFileSize);
        vData.insert(vData.begin(), std::istream_iterator<uint8_t>(in), std::istream_iterator<uint8_t>());

        return true;
    }
    else
    {
        return false;
    }
}

//Save the contents of the strText parameter into a text file given by the strPath filepath (overwrite if it exists) 
bool saveText(const std::string &strPath, const std::string &strText)
{
    std::string strTempPath = strPath + "~";

    // Save to temp file first then move to real file.
    std::ofstream out(strTempPath.c_str());
    if (out.is_open())
    {
        out << strText;
        out.flush();
        out.close();
        if (!out.good())
        {
            return false;
        }

        return rename(strTempPath.c_str(), strPath.c_str()) == 0;
    }
    else
    {
        return false;
    }
}
	
//Save the contents of the vData array parameter into a binary file given by the strPath filepath (overwrite if it exists) 
bool saveBin(const std::string &strPath, const std::vector<uint8_t> &vData)
{
    std::string strTempPath = strPath + "~";

    // Save to temp file first then move to real file.
    std::ofstream out(strTempPath.c_str(), std::ios::out | std::ofstream::binary);
    if (out.is_open())
    {
        std::copy(vData.begin(), vData.end(), std::ostreambuf_iterator<char>(out));
        out.flush();
        out.close();
        if (!out.good())
        {
            return false;
        }

        return rename(strTempPath.c_str(), strPath.c_str()) == 0;
    }
    else
    {
        return false;
    }
}

} /* namespace aurizon */
