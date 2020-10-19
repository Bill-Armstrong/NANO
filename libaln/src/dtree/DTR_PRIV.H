
// DTREE implementation support routines

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

// dtr_priv.h

#ifndef __DTR_PRIV_H__
#define __DTR_PRIV_H__

/*
/////////////////////////////////////////////////////////////////////
// parser - returns DTE_NOERR on success
// returns new DTREE in *ppDtree - use DestroyDtree to delete
*/
int ParseDtreeFile(FILE* pFile, DTREE** ppDtree);

/*
/////////////////////////////////////////////////////////////////////
// exporter - returns DTE_NOERR on success
// exports pDtree to a text file
*/
int ExportDtreeFile(FILE* pFile, const char* pszFileName, DTREE* pDtree);

/*
/////////////////////////////////////////////////////////////////////
// binary file import - returns DTE_NOERR on success
// returns new DTREE in *ppDtree - use DestroyDtree to delete
*/
int ReadBinDtreeFile(FILE* pFile, DTREE** ppDtree);

/*
/////////////////////////////////////////////////////////////////////
// binary file export - returns DTE_NOERR on success
// exports pDtree to a binary file
*/
int WriteBinDtreeFile(FILE* pFile, DTREE* pDtree);


/*
/////////////////////////////////////////////////////////////////////
// error code messages
// returns new DTREE in *ppDtree - use DestroyDtree to delete
*/
void GetErrMsg(int nErrno, char* pBuf, int nMaxBufLen);


#endif  /* __DTR_PRIV_H__ */
