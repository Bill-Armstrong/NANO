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

// file  fillinputvector.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void ALNAPI FillInputVector(const ALN* pALN,
                            float* afltX, 
                            long nSample,
                            int nStart,
                            ALNDATAINFO* pDataInfo,
                            const ALNCALLBACKINFO* pCallbackInfo)
{
  ASSERT(afltX);
  ASSERT(pALN);

  int nDim = pALN->nDim;  // register statement removed
  int nCols = pDataInfo->nTRcols; // This is 2 * nDim + 1
  const float* afltTRdata = pDataInfo->afltTRdata;
  const VARINFO* aVarInfo = pDataInfo->aVarInfo; // Not used

  // fill input vector
  VECTORINFO vectorinfo;        
  vectorinfo.bNeedData = FALSE;
  if (afltTRdata != NULL)
  {
    ASSERT(nStart == 0);	// we must start at zero, since no aVarInfo
    memcpy(afltX, afltTRdata + (nSample * nCols), sizeof(float) * nDim); 
  }
  // send vector info message
	if (pCallbackInfo && CanCallback(AN_VECTORINFO, pCallbackInfo->pfnNotifyProc,
									pCallbackInfo->nNotifyMask))
	{
	vectorinfo.nSample = nSample + nStart; // nStart based
	vectorinfo.aVarInfo = aVarInfo;
	vectorinfo.afltX = afltX;
	Callback(pALN, AN_VECTORINFO, &vectorinfo, pCallbackInfo->pfnNotifyProc,
				pCallbackInfo->pvData);
	
	}
}