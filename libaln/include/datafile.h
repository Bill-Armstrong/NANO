// ALN Library C++ data file class
/* MIT License

Copyright (c) 2020 William Ward Armstrong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

// datafile.h


#ifndef __DATAFILE_H__
#define __DATAFILE_H__

#ifndef ASSERT
#ifndef ALNASSERT
#define ASSERT assert
#else
#define ASSERT ALNASSERT
#endif
#endif

#ifndef assert
#include <assert.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// library files (Microsoft compilers only)

#ifndef ALNPP_NOFORCE_LIBS

#endif  // ALNPP_NOFORCE_LIBS



/////////////////////////////////////////////////////////////////////////////
// data type definitions

typedef unsigned char BYTE;
typedef int BOOL;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


///////////////////////////////////////////////////////////////////////////////
// class CDataFile

class CDataFile
{ 
// Constructors
public:
  CDataFile();    
  CDataFile(const CDataFile& datafile);
    
  BOOL Create(long lRows, long lColumns);

// Attributes
public:
  
  long ColumnCount() const
    { return m_lColumns; }
  long RowCount() const
    { return m_lRows; }
  
  float GetColMax(long lCol) const;
  float GetColMin(long lCol) const;
  
  float operator[](long lIndex) const
    {
      ASSERT(lIndex < (m_lBufferLen / (long)sizeof(float)));
      return m_pBuffer[lIndex];
    }
  float& operator[](long lIndex)
    {
      ASSERT(lIndex < (m_lBufferLen / (long)sizeof(float)));
      return m_pBuffer[lIndex];
    }

  long CalcDataIndex(long lRow, long lColumn, long lDelta = 0) const
    {
      ASSERT((lRow + lDelta) >= 0 && (lRow + lDelta) < m_lRows && 
             lColumn >=0 && lColumn < m_lColumns);
      ASSERT((lRow + lDelta) * m_lColumns + lColumn < (m_lBufferLen / (long)sizeof(float)));
      return (lRow + lDelta) * m_lColumns + lColumn;
    }

  const float* GetRowAt(long lRow) const
    {
      ASSERT(m_pBuffer);
      return m_pBuffer + CalcDataIndex(lRow, 0, 0);
    }

  float* GetRowAt(long lRow)
    {
      ASSERT(m_pBuffer);
      return m_pBuffer + CalcDataIndex(lRow, 0, 0);
    }

  float GetAt(long lRow, long lColumn, long lDelta = 0) const
    { 
      ASSERT(m_pBuffer);
      return m_pBuffer[CalcDataIndex(lRow, lColumn, lDelta)];
    }
  void SetAt(long lRow, long lColumn, float flt, long lDelta = 0)
    { 
      ASSERT(m_pBuffer);
      m_pBuffer[CalcDataIndex(lRow, lColumn, lDelta)] = flt;
    }

	const float* GetDataPtr() const
		{	return m_pBuffer; }

  float* GetDataPtr()
		{	return m_pBuffer; }

// Operations:
public:

  BOOL Append(const CDataFile& datafile);
    // appends datafile to end of this... truncates or adds columns
    // from datafile as necessary to match our columns

  BOOL Read(const char* pszFileName);
  BOOL ReadBinary(const char* pszFileName);
    // read data from a file, erase current contents
  
  BOOL ReadAppend(const char* pszFileName);
  BOOL ReadAppendBinary(const char* pszFileName);
    // read data from a file, appending to current contents
  
  BOOL Write(const char* pszFileName, int nDelim = '\t');
  BOOL WriteBinary(const char* pszFileName);
 
  void Destroy();
  
// implementation
public:
  virtual ~CDataFile();  
  CDataFile& operator = (const CDataFile& datafile);

protected:  

  // growing the data file
  BOOL Grow(long lNewLen);
  
  float* m_pBuffer;      // data block
  long m_lBufferLen;      // length of block
  long m_lColumns;        // number of columns
  long m_lRows;           // number of rows
};

///////////////////////////////////////////////////////////////////////////////

#endif  // __DATAFILE_H__
