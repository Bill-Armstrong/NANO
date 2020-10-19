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

// validatedatainfo.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int ALNAPI ValidateALNDataInfo(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo)
{
    // parameter variance
    if (pDataInfo == NULL)
    {
        return ALN_GENERIC;
    }

    if (pALN == NULL)
    {
        return ALN_GENERIC;
    }
    /*
    // must have at least one training point or not have a training set
      if ((pDataInfo->nTRcurrSamples <= 0) && (pDataInfo->fltMSEorF <= 0))
    {
      return ALN_GENERIC;
    }

    // need proc if no data
    if(pDataInfo->afltTRdata == NULL &&
       (pCallbackInfo == NULL || pCallbackInfo->pfnNotifyProc == NULL)   )
    {
      return ALN_GENERIC;
    }

    // need AN_VECTORINFO if no data
    if(pDataInfo->afltTRdata == NULL &&
       (pCallbackInfo == NULL || !(pCallbackInfo->nNotifyMask & AN_VECTORINFO)))
    {
      return ALN_GENERIC;
    }

    // make sure columns valid
    if (pDataInfo->aVarInfo == NULL &&
        pDataInfo->afltTRdata != NULL &&
        pDataInfo->nTRcols < pALN->nDim)
    {
      return ALN_GENERIC;
    }

    // check var info column, delta validity
    // const VARINFO* aVarInfo = pDataInfo->aVarInfo; this is not used
    */
    return ALN_NOERROR;
}

#ifdef _DEBUG
void ALNAPI DebugValidateALNDataInfo(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo)
{
    ASSERT(pDataInfo != NULL);

    // valid aln pointer
    ASSERT(pALN != NULL);

    // valid number of points
    ASSERT(pDataInfo->nTRcurrSamples > 0);

    // valid data cols
    ASSERT((pDataInfo->afltTRdata != NULL && pDataInfo->nTRcols > 0) ||
        pDataInfo->afltTRdata == NULL);

    // valid notify proc
    ASSERT(pDataInfo->afltTRdata != NULL ||
        (pCallbackInfo != NULL &&
            pCallbackInfo->pfnNotifyProc != NULL &&
            (pCallbackInfo->nNotifyMask & AN_VECTORINFO)));

    // valid varinfo
    ASSERT(pDataInfo->aVarInfo != NULL || pDataInfo->nTRcols >= (2 * pALN->nDim + 1));
    if (pDataInfo->aVarInfo != NULL)
    {
        for (int i = 0; i < pALN->nDim; i++)
        {
            // column validity
            ASSERT(pDataInfo->afltTRdata);
        }
    }
}
#endif
