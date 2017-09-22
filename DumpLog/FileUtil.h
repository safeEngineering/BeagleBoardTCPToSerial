/************************************************************
 * FileUtil.h
 *
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		13 Oct 2015	1.0.0    Original
 ************************************************************/

#ifndef __FILEUTIL_H_
#define __FILEUTIL_H_

#include <string>
#include <vector>

namespace aurizon
{

const char PATH_SEP = '/';

int mkpath(const std::string &strPath, unsigned int mode = 0777);
bool loadText(const std::string &strPath, std::string &strText);
bool loadBin(const std::string &strPath, std::vector<uint8_t> &vData);
bool saveText(const std::string &strPath, const std::string &strText);
bool saveBin(const std::string &strPath, const std::vector<uint8_t> &vData);

} /* namespace aurizon */

#endif /* __FILEUTIL_H_ */
