// ALN Library

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

// calcdataendpoints.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
void ALNAPI CalcDataEndPoints(long& nStart, long& nEnd, const ALN* pALN,
                              ALNDATAINFO* pDataInfo)
{
  ASSERT(pALN);
  ASSERT(pDataInfo);

  int nDim = pALN->nDim;
  const VARINFO* aVarInfo = pDataInfo->aVarInfo;

  nStart = 0; 
  nEnd = 0;
  if (aVarInfo != NULL)
  {
    // scan var info structs and account for any time shifts
    for (int i = 0; i < nDim; i++)
    {
      if (aVarInfo[i].nDelta < 0 && abs(aVarInfo[i].nDelta) > nStart)
        nStart = abs(aVarInfo[i].nDelta);
      else if (aVarInfo[i].nDelta > 0 && aVarInfo[i].nDelta > nEnd)
        nEnd = aVarInfo[i].nDelta;
    }

    ASSERT(nStart + nEnd <= pDataInfo->nTRcurrSamples);
    
    // adjust nEnd to reflect end point
    nEnd = pDataInfo->nTRcurrSamples - nEnd - 1;
  }
  else
  {
    nStart = 0;
    nEnd = pDataInfo->nTRcurrSamples - 1;
  }

  ASSERT(nStart >= 0 && nStart < pDataInfo->nTRcurrSamples && nStart <= nEnd);
  ASSERT(nEnd >= 0 && nEnd < pDataInfo->nTRcurrSamples);
}
*/