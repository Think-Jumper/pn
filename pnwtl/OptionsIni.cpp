/**
 * @file OptionsIni.h
 * @brief Ini configuration functionality.
 * @author Simon Steele
 * @note Copyright (c) 2004 Simon Steele <s.steele@pnotepad.org>
 *
 * Programmers Notepad 2 : The license file (license.[txt|html]) describes 
 * the conditions under which this source may be modified / distributed.
 */

#include "stdafx.h"
#include "OptionsManager.h"
#include "OptionsIni.h"

#pragma warning( push )
#pragma warning(disable: 4996) // see MSDN on hash_map

#if (_ATL_VER >= 0x0700)
	#include <hash_map>
	class KeyMap : public std::hash_map<tstring, tstring>{};
#else
	#include <map>
	class KeyMap : public std::map<tstring, tstring>{};
#endif

IniOptions::IniOptions()
{
	keyMap = new KeyMap();

	_filename = NULL;
	
	tstring userSettingsPath;
	GetPNPath(userSettingsPath, PNPATH_USERSETTINGS);
	userSettingsPath += _T("UserSettings.ini");
	_filename = new TCHAR[userSettingsPath.length()+1];
	_tcscpy(_filename, userSettingsPath.c_str());
}

IniOptions::~IniOptions()
{
	if(keyMap)
	{
		delete keyMap;
		keyMap = NULL;
	}

	if(_filename)
	{
		delete [] _filename;
		_filename = NULL;
	}
}

void IniOptions::Set(LPCTSTR subkey, LPCTSTR value, bool bVal)
{
	::WritePrivateProfileString(groupLocked ? _group : subkey, value, bVal ? _T("1") : _T("0"), _filename);
}

void IniOptions::Set(LPCTSTR subkey, LPCTSTR value, int iVal)
{
	TCHAR cbuf[40];
	_itot(iVal, cbuf, 10);
	::WritePrivateProfileString(groupLocked ? _group : subkey, value, cbuf, _filename);
}

void IniOptions::Set(LPCTSTR subkey, LPCTSTR value, LPCTSTR szVal)
{
	::WritePrivateProfileString(groupLocked ? _group : subkey, value, szVal, _filename);
}


bool IniOptions::Get(LPCTSTR subkey, LPCTSTR value, bool bDefault)
{
	return Get(subkey, value, bDefault ? 1 : 0) != 0;
}

int IniOptions::Get(LPCTSTR subkey, LPCTSTR value, int iDefault)
{
	if(groupLocked)
	{
		KeyMap::const_iterator i = keyMap->find(tstring(value));
		if(i != keyMap->end())
			return _ttoi((*i).second.c_str());
		else
			return iDefault;
	}
	else
	{
		return GetPrivateProfileInt(subkey, value, iDefault, _filename);
	}
}

tstring IniOptions::Get(LPCTSTR subkey, LPCTSTR value, LPCTSTR szDefault)
{
	return tstring(_T(""));
}

void IniOptions::group(LPCTSTR location)
{
	//The return value specifies the number of characters copied to the buffer, not including the terminating null character. If the buffer is not large enough to contain all the key name and value pairs associated with the named section, the return value is equal to nSize minus two.
	int bufsize = 2048;
	TCHAR* buffer = new TCHAR[bufsize];
	while( GetPrivateProfileSection(location, buffer, bufsize, _filename) == (bufsize -2) )
	{
		bufsize *= 2;
		delete [] buffer;
		buffer = new TCHAR[bufsize];
	}

	TCHAR* p = buffer;
	TCHAR* comma;
	while(*p)
	{
		comma = _tcschr(p, _T('='));
		*comma = NULL;
		comma++;
		keyMap->insert(KeyMap::value_type(tstring(p), tstring(comma)));
		p = comma + _tcslen(comma) + 1;
	}

	delete [] buffer;

	groupLocked = true;
	_group = location;
}

void IniOptions::ungroup()
{
	keyMap->clear();
	groupLocked = false;
}

#pragma warning( pop ) // 4996 - deprecated hash_map.