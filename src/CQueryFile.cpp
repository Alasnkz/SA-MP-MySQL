#include <fstream>
#include <map>
#include "CQueryFile.hpp"
#include "CHandle.hpp"
#include "CLog.hpp"
#include <fmt/printf.h>
#include "misc.hpp"

CQueryFile::CQueryFile(QueryFileId_t id, std::string const filepath) :
	m_Id(id),
	m_Callback(nullptr)
{
	std::ifstream stream("scriptfiles/" + filepath);
	if (stream.fail())
	{
		return; // fail
	}

	std::string value(std::istreambuf_iterator<char>(stream), {});
	m_Query = value;
	int nonce = 0;
	
	size_t loc = m_Query.find("?");
	while (loc != std::string::npos)
	{
		size_t begin_quote = m_Query.substr(0, loc).find("\"");
		size_t end_quote = m_Query.substr(begin_quote + 1).find("\"");
		if (begin_quote == std::string::npos && end_quote == std::string::npos)
		{
			m_Query[loc] = ' ';
			m_ReplacementLocations.emplace(nonce, loc);
			nonce++;
		}
		loc = m_Query.find("?", loc + 1);
	}
}

CQueryFile::~CQueryFile()
{
	if (m_Callback)
	{
		delete m_Callback;
	}
}

std::string const CQueryFile::RenderString(Handle_t handle, std::string const & specifiers, AMX* amx, cell* params, int param_start)
{
	std::string value = m_Query; // Copy the query.
	int last_loc = 0;
	int current_location = 0;
	int current_param_count = 0;
	for (const char character : specifiers)
	{
		if (current_param_count > m_ReplacementLocations.size())
		{
			// bad!
			CLog::Get()->Log(LogLevel::ERROR, "CQueryFile::RenderString param count '{}' is bigger than the replacement count '{}'", current_param_count, m_ReplacementLocations.size());
			break;
		}
		auto replacement_loc = m_ReplacementLocations.find(current_location);
		if (replacement_loc == m_ReplacementLocations.end())
		{
			CLog::Get()->Log(LogLevel::ERROR, "CQueryFile::RenderString could not find param count '{}' in replacement list", current_param_count);
			break;
		}

		cell* amx_address = nullptr;
		amx_GetAddr(amx, params[param_start + current_param_count],
			&amx_address);

		value.replace(replacement_loc->second + last_loc, 1, ""); // remove space
		switch (character)
		{
			case 'e':
			{
				std::string str = amx_GetCppString(amx, params[param_start + current_param_count]);
				string escaped_str;
				if (handle->EscapeString(str.c_str(), escaped_str))
				{
					value.insert(replacement_loc->second + last_loc, escaped_str);
					last_loc += escaped_str.length();
				}
				else
				{
					CLog::Get()->LogNative(LogLevel::ERROR,
						"can't escape string '{}'",
						str.length() ? str : "(nullptr)");
				}
				break;
			}

			case 'd':
			case 'i':
			case 'o':
			case 'x':
			case 'X':
			case 'u':
			{
				std::string str = std::to_string(static_cast<int>(*amx_address));
				value.insert(replacement_loc->second + last_loc, str);
				last_loc += str.length();
				break;
			}
	
			case 's':
			{
				std::string str = amx_GetCppString(amx, params[param_start + current_param_count]);
				value.insert(replacement_loc->second + last_loc, str);
				last_loc += str.length();
				break;
			}

			case 'f':
			case 'F':
			case 'a':
			case 'A':
			case 'g':
			case 'G':
			{
				std::string str = std::to_string(amx_ctof(*amx_address));
				value.insert(replacement_loc->second + last_loc, str);
				last_loc += str.length();
				break;
			}
				
			case 'b':
			{
				std::string str;
				ConvertDataToStr<int, 2>(*amx_address, str);
				value.insert(replacement_loc->second + last_loc, str);
				last_loc += str.length();
				break;
			}

			default:
				CLog::Get()->LogNative(LogLevel::ERROR,
					"invalid format specifier '{}'",
					character);
				break;
		}
		current_location++;
		current_param_count++;
	}
	return value;
}

void CQueryFile::SetCallback(CCallback* callback)
{
	m_Callback = callback;
}

QueryFileId_t CQueryFileManager::Create(std::string const & filepath)
{
	QueryFileId_t id = 1;
	while (m_Handles.find(id) != m_Handles.end())
		++id;

	QueryFile_t queryfile = new CQueryFile(id, filepath);
	m_Handles.emplace(id, queryfile);
	return id;
}

bool CQueryFileManager::Destroy(QueryFile_t& handle)
{
	CLog::Get()->Log(LogLevel::DEBUG, "CQueryFileManager::Destroy(this={}, handle={})",
		static_cast<const void*>(this),
		static_cast<const void*>(handle));

	if (handle == nullptr)
		return false;

	if (m_Handles.erase(handle->GetId()) == 0)
		return false;


	delete handle;
	return true;
}