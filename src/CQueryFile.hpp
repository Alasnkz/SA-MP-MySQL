#pragma once

#include "CSingleton.hpp"

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

#include "CError.hpp"
#include "types.hpp"
#include "sdk.hpp"
#include "CCallback.hpp"

class CQueryFile
{
public:
	friend class CQueryFile;
 //constructor / deconstructor
	CQueryFile(QueryFileId_t id, std::string const filepath);
	~CQueryFile();

private: //variables
	const QueryFileId_t m_Id;
	std::map<int, size_t> m_ReplacementLocations;
	std::string m_Query;
	CCallback* m_Callback;

public: //functions
	inline QueryFileId_t GetId() const
	{
		return m_Id;
	}

	std::string const RenderString(Handle_t handle, std::string const & specifiers, AMX* amx, cell* params, int param_start);
	void SetCallback(CCallback* callback);
	CCallback* GetCallback()
	{
		return m_Callback;
	}
};

class CQueryFileManager : public CSingleton<CQueryFileManager>
{
	friend class CSingleton<CQueryFileManager>;
private: //constructor / deconstructor
	CQueryFileManager() = default;
	~CQueryFileManager() = default;

private: //variables
	unordered_map<QueryFileId_t, QueryFile_t> m_Handles;

public: //functions
	QueryFileId_t Create(std::string const & filepath);

	bool Destroy(QueryFile_t& handle);

	inline bool IsValidHandle(const QueryFileId_t id)
	{
		return m_Handles.find(id) != m_Handles.end();
	}
	inline QueryFile_t GetHandle(const QueryFileId_t id)
	{
		return IsValidHandle(id) ? m_Handles.at(id) : nullptr;
	}
};

