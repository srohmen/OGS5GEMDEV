// SelectedOutput.cpp: implementation of the CSelectedOutput class.
//
//////////////////////////////////////////////////////////////////////
#if defined(WIN32)
#include <windows.h>                // OutputDebugString
#endif

#if defined(_DEBUG)
#include <sstream>                  // std::ostringstream
#endif

#include <stdarg.h>
#include <stdio.h>

#include "Debug.h"                  // ASSERT
#include "SelectedOutput.hxx"       // CSelectedOutput

const size_t RESERVE_ROWS = 80;
const size_t RESERVE_COLS = 80;

CSelectedOutput::CSelectedOutput()
: m_nRowCount(0)
{
	this->m_arrayVar.reserve(RESERVE_COLS);
}

CSelectedOutput::~CSelectedOutput()
{
}

void CSelectedOutput::Clear(void)
{
	this->m_nRowCount = 0;
	this->m_vecVarHeadings.clear();
	this->m_arrayVar.clear();
	this->m_mapHeadingToCol.clear();
}

size_t CSelectedOutput::GetRowCount(void)const
{
	if (this->GetColCount())
	{
		return this->m_nRowCount + 1; // rows + heading
	}
	else
	{
		return 0;
	}
}

size_t CSelectedOutput::GetColCount(void)const
{
	return this->m_vecVarHeadings.size();
}

CVar CSelectedOutput::Get(int nRow, int nCol)const
{
	CVar v;
	this->Get(nRow, nCol, &v);
	return v;
}

VRESULT CSelectedOutput::Get(int nRow, int nCol, VAR* pVAR)const
{
	if (::VarClear(pVAR) == VR_BADVARTYPE)
	{
		return VR_BADVARTYPE;
	}
	if ((size_t)nRow >= this->GetRowCount() || nRow < 0)
	{
		pVAR->type = TT_ERROR;
		pVAR->vresult = VR_INVALIDROW;
		return pVAR->vresult;
	}
	if ((size_t)nCol >= this->GetColCount() || nCol < 0)
	{
		pVAR->type = TT_ERROR;
		pVAR->vresult = VR_INVALIDCOL;
		return pVAR->vresult;
	}
	if (nRow)
	{
		ASSERT((size_t)nRow <= this->m_arrayVar[nCol].size());
		return ::VarCopy(pVAR, &(this->m_arrayVar[nCol])[nRow - 1]);
	}
	else
	{
		return ::VarCopy(pVAR, &(this->m_vecVarHeadings[nCol]));
	}
}


int CSelectedOutput::EndRow(void)
{
	if (size_t ncols = this->GetColCount())
	{
		++this->m_nRowCount;

		// make sure array is full
		for (size_t col = 0; col < ncols; ++col)
		{
			size_t nrows = this->m_arrayVar[col].size();
			if (nrows < this->m_nRowCount)
			{
				// fill w/ empty
				this->m_arrayVar[col].resize(this->m_nRowCount);
			}
#if defined(_DEBUG)
			else if (nrows > this->m_nRowCount)
			{
				ASSERT(false);
			}
#endif
		}
	}
	return 0;
}

int CSelectedOutput::PushBack(const char* key, const CVar& var)
{
	try
	{
		// check if key is new
		std::map< std::string, size_t >::iterator find;
		find = this->m_mapHeadingToCol.find(std::string(key));
		if (find == this->m_mapHeadingToCol.end())
		{
			// new key(column)
			//
			this->m_mapHeadingToCol.insert(std::map< std::string, size_t >::value_type(std::string(key),
				this->m_mapHeadingToCol.size()) );

			// add heading
			//
			this->m_vecVarHeadings.push_back(CVar(key));


			// add new vector(col)
			//
			this->m_arrayVar.resize(this->m_arrayVar.size() + 1);
			this->m_arrayVar.back().reserve(RESERVE_ROWS);

			// add empty rows if nec
			if (this->m_nRowCount)
				this->m_arrayVar.back().resize(this->m_nRowCount);

			this->m_arrayVar.back().push_back(var);
		}
		else
		{
			if (this->m_arrayVar[find->second].size() == this->m_nRowCount) {
				this->m_arrayVar[find->second].push_back(var);
			}
			else {
				ASSERT(this->m_arrayVar[find->second].size() == this->m_nRowCount + 1);
				this->m_arrayVar[find->second].at(this->m_nRowCount) = var;
			}
		}
		return 0;
	}
	catch(...)
	{
		ASSERT(false);
	}
	return 1; // error
}


int CSelectedOutput::PushBackDouble(const char* key, double value)
{
	CVar v(value);
	return this->PushBack(key, v);
}

int CSelectedOutput::PushBackLong(const char* key, long value)
{
	CVar v(value);
	return this->PushBack(key, v);
}

int CSelectedOutput::PushBackString(const char* key, const char* value)
{
	CVar v(value);
	return this->PushBack(key, v);
}

int CSelectedOutput::PushBackEmpty(const char* key)
{
	CVar v;
	return this->PushBack(key, v);
}

#if defined(_DEBUG)
void CSelectedOutput::Dump(const char* heading)
{
	::OutputDebugStringA(heading);
	std::ostringstream oss;
	oss << *this;
	std::istringstream iss(oss.str());
	std::string line;
	while (std::getline(iss, line))
	{
		::OutputDebugStringA(line.c_str());
		::OutputDebugStringA("\n");
	}
}

void CSelectedOutput::AssertValid(void)const
{
	if (size_t cols = this->GetColCount())
	{
		size_t rows = this->m_arrayVar[0].size();
		for (size_t col = 0; col < cols; ++col)
		{
			ASSERT(rows == this->m_arrayVar[col].size());
		}
	}
}
#endif

std::ostream& operator<< (std::ostream &os, const CSelectedOutput &a)
{
	os << "CSelectedOutput(rows=" << a.GetRowCount() << ", cols=" << a.GetColCount() << ")\n";

	CVar v;
	for (size_t r = 0; r < a.GetRowCount(); ++r)
	{
		for (size_t c = 0; c < a.GetColCount(); ++c)
		{
			a.Get((int)r, (int)c, &v);
			os << v << ", ";
			::VarClear(&v);
		}
		os << "\n";
	}
	os << "\n";
	return os;
}
