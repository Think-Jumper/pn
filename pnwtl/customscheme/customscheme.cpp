/**
 * @file CustomScheme.cpp
 * @brief Defines the entry point for the custom schemes DLL.
 * @author Simon Steele
 * @note Copyright (c) 2002 Simon Steele <s.steele@pnotepad.org>
 *
 * Programmers Notepad 2 : The license file (license.[txt|html]) describes 
 * the conditions under which this source may be modified / distributed.
 */
#include "stdafx.h"
#include "CustomScheme.h"
#include "scintilla/Accessor.h"
#include "scintilla/WindowAccessor.h"

HMODULE theModule;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		theModule = static_cast<HMODULE>( hModule );
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

Lexers theLexers;

//////////////////////////////////////////////////////////////////////////
// CustomLexerFactory
//////////////////////////////////////////////////////////////////////////

CustomLexerFactory::CustomLexerFactory(const char* path)
{
	m_parser.SetParseState(this);

	//Load and parse customlexers
	tstring sPath(path);
	if(sPath[sPath.size()-1] != '/' && sPath[sPath.size()-1] != '\\')
		sPath += '\\';

	tstring sPattern(sPath);
	sPattern += "*.schemedef";

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(sPattern.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		//Found the first file...
		BOOL found = TRUE;
		tstring to_open;

		while (found) {
			to_open = sPath;
			to_open += FindFileData.cFileName;
			
			// Now make the CustomLexer object.
			m_pCurrent = new CustomLexer;
			if( Parse(to_open.c_str()) )
				theLexers.AddLexer(m_pCurrent);
			else
				delete m_pCurrent;

			found = FindNextFile(hFind, &FindFileData);
		}

		FindClose(hFind);

	}
}

bool CustomLexerFactory::Parse(LPCTSTR file)
{
	m_bFileOK = true;
	m_state = STATE_DEFAULT;

	m_parser.LoadFile(file);
	m_parser.Reset();

	return m_bFileOK;
}

void CustomLexerFactory::doScheme(const XMLAttributes& atts)
{
	// ??? braces="{[()]}

	LPCTSTR pName;
	LPCTSTR pVal;

	for(int i = 0; i < atts.getCount(); i++)
	{
		pName = atts.getName(i);
		pVal = atts.getValue(i);

		if(_tcscmp(pName, _T("casesensitive")) == 0)
		{
			m_pCurrent->bCaseSensitive = (pVal[0] == _T('t') || pVal[0] == _T('T'));
		}
		else if(_tcscmp(pName, _T("name")) == 0)
		{
			m_pCurrent->tsName = pVal;
		}
	}

	m_state = STATE_INSCHEME;
}

void CustomLexerFactory::doStringType(const XMLAttributes& atts)
{
	int id;
	
	LPCTSTR szID = atts.getValue(_T("id"));
	if(!szID)
		return;

	id = _ttoi(szID);

	if(id < 0 || id >= MAX_STRINGTYPES)
		return;
	
	StringType_t& st = m_pCurrent->stringTypes[id];

	LPCTSTR pName, pVal;

	for(int i = 0; i < atts.getCount(); i++)
	{
		pName = atts.getName(i);
		pVal = atts.getValue(i);

		if(_tcscmp(pName, _T("start")) == 0)
		{
			st.start = pVal[0];
		}
		else if(_tcscmp(pName, _T("end")) == 0)
		{
			st.end = pVal[0];
		}
		else if(_tcscmp(pName, _T("multiline")) == 0)
		{
			st.multiLine = SBOOL(pVal);
		}
		else if(_tcscmp(pName, _T("continuation")) == 0)
		{
			st.bContinuation = true;
			st.continuation = pVal[0];
		}
		else if(_tcscmp(pName, _T("escape")) == 0)
		{
			st.bEscape = true;
			st.escape = pVal[0];
		}
	}

	if(st.start != 0 && st.end != 0)
		st.bValid = true;
}

void CustomLexerFactory::doKeyword(const XMLAttributes& atts)
{
	LPCTSTR szKey = atts.getValue(_T("key"));
	if(!szKey)
		return;

	int key = _ttoi(szKey);

	if(key < 0 || key >= MAX_KEYWORDS)
		return;

	m_pCurrent->kwEnable[key] = true;
}

void CustomLexerFactory::doPreProcessor(const XMLAttributes& atts)
{
	LPCTSTR pszStart = atts.getValue(_T("start"));
	if(!pszStart)
		return;
	m_pCurrent->bPreProc = true;
	m_pCurrent->preProcStart = pszStart[0];

	LPCTSTR pszCont = atts.getValue(_T("continuation"));
	if(!pszCont)
		return;
	m_pCurrent->bPreProcContinuation = true;
	m_pCurrent->preProcContinue = pszCont[0];
}

void CustomLexerFactory::doNumbers(const XMLAttributes& atts)
{
	LPCTSTR pszStart = atts.getValue(_T("start"));
	if(!pszStart)
		return;

	// start will be something like [a-z]. This needs parsing into a character set.
	m_pCurrent->numberStartSet.ParsePattern(pszStart);

	LPCTSTR pszContent = atts.getValue(_T("content"));
	if(!pszContent)
		return;

	m_pCurrent->numberContentSet.ParsePattern(pszContent);
}

void CustomLexerFactory::doIdentifiers(const XMLAttributes& atts)
{
	LPCTSTR pszStart = atts.getValue(_T("start"));
	if( pszStart )
	{
		CharSet chSet;
		if( chSet.ParsePattern(pszStart) )
            m_pCurrent->wordStartSet = chSet;
	}
	
	LPCTSTR pszContent = atts.getValue(_T("content"));
	if( pszContent )
	{
		CharSet chSet;
		if( chSet.ParsePattern(pszContent) )
            m_pCurrent->wordContentSet = chSet;
	}
}

void CustomLexerFactory::doIdentifiers2(const XMLAttributes& atts)
{
	LPCTSTR pszStart = atts.getValue(_T("start"));
	if(!pszStart)
		return;

	// start will be something like [a-z]. This needs parsing into a character set.
	m_pCurrent->identStartSet.ParsePattern(pszStart);

	LPCTSTR pszContent = atts.getValue(_T("content"));
	if(!pszContent)
		return;

	m_pCurrent->identContentSet.ParsePattern(pszContent);
}

void CustomLexerFactory::SetCommentTypeCode(LPCTSTR pVal, ECodeLength& length, TCHAR* code, TCHAR*& pCode, CommentType_t* type)
{
	if(pVal && pVal[0] != NULL)
	{
		code[0] = pVal[0];
		if(pVal[1] == NULL)
		{
			length = eSingle;
		}
		else if(pVal[2] == NULL)
		{
			length = eDouble;
			code[1] = pVal[1];
		}
		else
		{
			length = eMore;
			pCode = new TCHAR[_tcslen(pVal)+1];
			_tcscpy(pCode, pVal);
		}
	}
	else
	{
		// Not sure what to do here...
		length = eSingle;
		code[0] = '\0';
	}
}

void CustomLexerFactory::doCommentType(int commentType, const XMLAttributes& atts)
{
	CommentType_t* type;
	switch(commentType)
	{
		case CT_LINE:
			type = &m_pCurrent->singleLineComment;
			break;
		case CT_BLOCK:
			type = &m_pCurrent->blockComment;
			break;
		default:
			return; // unknown type.
	}

	type->bValid = true;

	LPCTSTR pVal = atts.getValue(_T("start"));
	SetCommentTypeCode(pVal, type->scLength, type->scode, type->pSCode, type);
	pVal = atts.getValue(_T("end"));
	SetCommentTypeCode(pVal, type->ecLength, type->ecode, type->pECode, type);

	if(commentType == CT_LINE)
	{
		pVal = atts.getValue(_T("continuation"));
		if(pVal)
		{
			type->bContinuation = true;
			type->continuation = pVal[0];
		}
	}
}

void CustomLexerFactory::startElement(LPCTSTR name, XMLAttributes& atts)
{
	if(_tcscmp(name, _T("schemedef")) == 0 && m_state == STATE_DEFAULT)
	{
		doScheme(atts);
	}
	else if( m_state == STATE_INSCHEME )
	{
		if(_tcscmp(name, _T("strings")) == 0 )
		{
			m_state = STATE_INSTRINGS;
		}
		else if(_tcscmp(name, _T("language")) == 0 )
		{
			m_state = STATE_INLANG;
		}
		else if( _tcscmp(name, _T("comments")) == 0 )
		{
			m_state = STATE_INCOMMENTS;
		}
		else if( _tcscmp(name, _T("preprocessor")) == 0 )
		{
			doPreProcessor(atts);
		}
		else if( _tcscmp(name, _T("numbers")) == 0 )
		{
			doNumbers(atts);
		}
		else if( _tcscmp(name, _T("identifiers")) == 0 )
		{
			doIdentifiers(atts);
		}
		else if( _tcscmp(name, _T("identifiers2")) == 0 )
		{
			doIdentifiers2(atts);
		}
	}
	else if( m_state == STATE_INSTRINGS )
	{
		if( _tcscmp(name, _T("stringtype")) == 0 )
		{
			doStringType(atts);
		}
	}
	else if( m_state == STATE_INLANG )
	{
		if( _tcscmp(name, _T("use-keywords")) == 0 )
			m_state = STATE_INUSEKW;
	}
	else if( m_state == STATE_INUSEKW )
	{
		if( _tcscmp(name, _T("keyword")) == 0 )
		{
			doKeyword(atts);
		}
	}
	else if( m_state == STATE_INCOMMENTS )
	{
		if( _tcscmp(name, _T("line")) == 0 )
		{
			doCommentType(CT_LINE, atts);
		}
		else if( _tcscmp(name, _T("block")) == 0 )
		{
			doCommentType(CT_BLOCK, atts);
		}
	}
}

void CustomLexerFactory::endElement(LPCTSTR name)
{
	if( m_state == STATE_INSTRINGS && _tcscmp(name, _T("strings")) == 0 )
	{
		m_state = STATE_INSCHEME;
	}
	else if( m_state == STATE_INUSEKW && _tcscmp(name, _T("use-keywords")) == 0 )
	{
		m_state = STATE_INLANG;
	}
	else if( m_state == STATE_INLANG && _tcscmp(name, _T("language")) == 0 )
	{
		m_state = STATE_INSCHEME;
	}
	else if( m_state == STATE_INCOMMENTS && _tcscmp(name, _T("comments")) == 0 )
	{
		m_state = STATE_INSCHEME;
	}
	else if( m_state == STATE_INSCHEME && _tcscmp(name, _T("schemedef")) == 0 )
	{
		m_state = STATE_DEFAULT;
	}
}

void CustomLexerFactory::characterData(LPCTSTR data, int len)
{

}

//////////////////////////////////////////////////////////////////////////
// Exported Functions
//////////////////////////////////////////////////////////////////////////

int FindLastSlash(char *inp) {
	int ret = -1;
	for (int i = static_cast<int>(strlen(inp)) - 1; i >= 0; i--) {
		if (inp[i] == '\\' || inp[i] == '/') {
			ret = i;
			break;
		}
	}
	return ret;
}

void FindLexers()
{
	static bool bFound = false;

	if(!bFound)
	{
		char path[MAX_PATH + 1];

		GetModuleFileName(theModule, path, MAX_PATH);

		int i = FindLastSlash(path);

		if (i == -1)
			i = static_cast<int>(strlen(path));

		tstring sPath(path, 0, i);

		CustomLexerFactory factory(sPath.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
// Exported Functions
//////////////////////////////////////////////////////////////////////////

int EXPORT __stdcall GetLexerCount()
{
	// We've been loaded and Scintilla wants our lexers, better find them...
	FindLexers();

	return theLexers.GetCount();
}

static void LengthSet(char *val, const char* newval, int size)
{
	if ( (int)strlen(newval) < size-1)
	{
		strcpy(val, newval);
	} 
	else 
	{
		strncpy(val, newval, size-1);
	}
}

void EXPORT __stdcall GetLexerName(unsigned int Index, char *name, int buflength)
{
	LengthSet(name, theLexers[Index]->GetName(), buflength);
}

void EXPORT __stdcall Lex(unsigned int lexer, 
	unsigned int startPos, int length, int initStyle, char *words[], WindowID window, char *props)
{
	WordList **wordlists = StringToWordLists(words);

	PropSet ps;
	ps.SetMultiple(props);
	
	WindowAccessor wa(window, ps);
	
	theLexers[lexer]->DoLex(startPos, length, initStyle, wordlists, wa);

	wa.Flush();

	FreeWordLists(wordlists);	
}

void EXPORT __stdcall Fold(unsigned int lexer, 
	unsigned int startPos, int length, int initStyle, char *words[], WindowID window, char *props)
{
	WordList **wordlists = StringToWordLists(words);

	PropSet ps;
	ps.SetMultiple(props);
	
	WindowAccessor wa(window, ps);
	
	theLexers[lexer]->DoFold(startPos, length, initStyle, wordlists, wa);

	wa.Flush();

	FreeWordLists(wordlists);
}