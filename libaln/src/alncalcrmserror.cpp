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

// calcrmserror.cpp


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// calc RMS error on data set

ALNIMP int ALNAPI ALNCalcRMSError(const ALN* pALN,
                                  ALNDATAINFO* pDataInfo,
                                  const ALNCALLBACKINFO* pCallbackInfo,
                                  float* pfltRMSErr)
{
  int nReturn = ValidateALNDataInfo(pALN, pDataInfo, pCallbackInfo);

  try
  {
    *pfltRMSErr = DoCalcRMSError(pALN, pDataInfo, pCallbackInfo);
  }
	catch(CALNUserException* e)
  {
    nReturn = ALN_USERABORT;
    e->Delete();
    *pfltRMSErr = -1.0;
  }
  catch (CALNMemoryException* e)	// memory specific exceptions
	{
		nReturn = ALN_OUTOFMEM;
    *pfltRMSErr = -1.0;
    e->Delete();
	}
	catch (CALNException* e)	      // anything other exception we recognize
	{
		nReturn = ALN_GENERIC;
    *pfltRMSErr = -1.0;
    e->Delete();
  }
  catch(...)
  {
    nReturn = ALN_GENERIC;
    *pfltRMSErr = -1.0;
  }

  return nReturn;
}

float ALNAPI DoCalcRMSError(const ALN* pALN,
                             ALNDATAINFO* pDataInfo,
                             const ALNCALLBACKINFO* pCallbackInfo)
{
#ifdef _DEBUG
  DebugValidateALNDataInfo(pALN, pDataInfo, pCallbackInfo);
#endif

  long nStart, nEnd;
  //CalcDataEndPoints(nStart, nEnd, pALN, pDataInfo);
  nStart = 0;
  nEnd = pDataInfo->nTRcurrSamples - 1;
  
  float fltRMSError = -1.0;
	ALNNODE* pTree = pALN->pTree;
  int nDim = pALN->nDim;
  float* afltX = NULL;
  const float** apfltBase = NULL;
  CCutoffInfo* aCutoffInfo = NULL;  
  
  try
  {
    // allocate eval vector
    afltX = new float[nDim];
    if (!afltX) ThrowALNMemoryException();
    memset(afltX, 0, sizeof(float) * nDim);

    // allocate and init cutoff info array
    aCutoffInfo = new CCutoffInfo[nEnd - nStart + 1];
    if (!aCutoffInfo) ThrowALNMemoryException();
		
    for (int i = nStart; i <= nEnd; i++)
			aCutoffInfo[i - nStart].pLFN = NULL;

    // calc rms error
    float fltSqErrorSum = 0;
    for (int nSample = nStart; nSample <= nEnd; nSample++)
	  {
      // get vector (cvt to zero based point index)
      FillInputVector(pALN, afltX, nSample - nStart, nStart, pDataInfo, pCallbackInfo);
		
      // do an eval to get active LFN and distance
      ALNNODE* pActiveLFN = NULL;
      CCutoffInfo& cutoffinfo = aCutoffInfo[nSample - nStart];
      float flt = CutoffEval(pTree, pALN, afltX, &cutoffinfo, &pActiveLFN);
      
		  // now add square of distance from surface to error
		  fltSqErrorSum += flt * flt;
	  }	// end for each point

	  fltRMSError = sqrt(fltSqErrorSum / (nEnd - nStart + 1));
  }
  catch(...)
  {
    delete[] afltX;
    delete[] aCutoffInfo;
    throw;
  }
  
  delete[] afltX;
  delete[] aCutoffInfo;
  return fltRMSError;
}
