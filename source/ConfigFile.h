/********************************************************************************
*	文件名称:	ConfigFile.h													*
*	创建时间：	2015/04/15														*
*	作   者 :	xzben															*
*	文件功能:	系统配置文件读取实现												*
*********************************************************************************/
#ifndef __2015_04_15_CONFIG_FILE_H__
#define __2015_04_15_CONFIG_FILE_H__

#include "SingleInstance.h"
#include <string>
#include <map>

class CConfigFile
{
public:
	CConfigFile();
	~CConfigFile();

	bool init(const char* file);
	void			dumpConfigs();
	int				getInt(const char *key, int def);
	double			getDouble(const char *key, double def);
	std::string		getString(const char* key, std::string def);
protected:
	int	 checkValue(char *line);
	bool checkKeyValid(char* key);
	bool checkValueValid(char	*& value);
private:
	std::string m_file;
	int			m_lineCount;
	typedef std::map<std::string, std::string> ValueContainer;
	ValueContainer m_values;
};
#endif//__2015_04_15_CONFIG_FILE_H__
