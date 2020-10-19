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

// alncalcconfidence.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int ALNAPI ValidateALNCalcConfidence(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo,
    ALNCONFIDENCE* pConfidence);

#ifdef _DEBUG
static void DebugValidateALNCalcConfidence(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo,
    ALNCONFIDENCE* pConfidence);
#endif

static int __cdecl CompareErrors(const void* pElem1, const void* pElem2)
{
    float flt1 = *(float*)pElem1;
    float flt2 = *(float*)pElem2;

    if (flt1 < flt2)
        return -1;
    else if (flt1 > flt2)
        return 1;

    return 0;
}

// set ALN confidence intervals based on a data set
// much of this is derived from theory documented in Master95 p302-323
// and Press et al p228-229

ALNIMP int ALNAPI ALNCalcConfidence(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo,
    ALNCONFIDENCE* pConfidence)
{
    int nReturn = ValidateALNCalcConfidence(pALN, pDataInfo, pCallbackInfo, pConfidence);
    if (nReturn != ALN_NOERROR)
        return nReturn;

#ifdef _DEBUG
    DebugValidateALNCalcConfidence(pALN, pDataInfo, pCallbackInfo, pConfidence);
#endif

    // result array
    float* afltResult = NULL;

    try
    {
        // see how many points there are
        long nTRcurrSamples = pDataInfo->nTRcurrSamples;

        // allocate results array
        afltResult = new float[nTRcurrSamples];

        // evaluate on data
        int nStart, nEnd;
        nReturn = EvalTree(pALN->pTree, pALN, pDataInfo, pCallbackInfo,
            afltResult, &nStart, &nEnd, TRUE);
        if (nReturn != ALN_NOERROR)
        {
            ThrowALNException();
        }

        // set the number of errors and error vector
        int nErr = (nEnd - nStart + 1);
        float* afltErr = afltResult + nStart;
        if (nErr <= 0)
        {
            ThrowALNException();  // bad error count
        }

        // EvalTree returned the errors in afltResult... now we sort them!
        qsort(afltErr, nErr, sizeof(float), CompareErrors);

        // calculate upper an lower bound indexes by discarding np-1 from each end
        // (conservative approach.. see Masters95 p305)
        int nDiscard = (int)floor((float)nErr * pConfidence->fltP - 1);

        // if nErr * pConfidence->fltP is less than 1, then nDiscard will be less than 0
        if (nDiscard < 0)
            nDiscard = 0;

        ASSERT(nDiscard >= 0 && nDiscard < nErr / 2);

        // set number of samples used 
        pConfidence->nSamples = nErr;

        // set lower bound
        int nLower = nDiscard;
        ASSERT(nLower >= 0 && nLower < nErr);
        pConfidence->fltLowerBound = afltErr[nLower];

        // set upper bound
        int nUpper = nErr - nDiscard - 1;
        ASSERT(nUpper >= 0 && nUpper < nErr);
        pConfidence->fltUpperBound = afltErr[nUpper];
    }
    catch (CALNUserException* e)	  // user abort exception
    {
        nReturn = ALN_USERABORT;
        e->Delete();
    }
    catch (CALNMemoryException* e)	// memory specific exceptions
    {
        nReturn = ALN_OUTOFMEM;
        e->Delete();
    }
    catch (CALNException* e)	      // anything other exception we recognize
    {
        nReturn = ALN_GENERIC;
        e->Delete();
    }
    catch (...)		                  // anything else, including FP errs
    {
        nReturn = ALN_GENERIC;
    }

    // deallocate mem
    delete[] afltResult;

    return nReturn;
}


// validate params
static int ALNAPI ValidateALNCalcConfidence(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo,
    ALNCONFIDENCE* pConfidence)
{
    int nReturn = ValidateALNDataInfo(pALN, pDataInfo, pCallbackInfo);
    if (nReturn != ALN_NOERROR)
        return nReturn;

    if (pConfidence->fltP <= 0.0 || pConfidence->fltP >= 0.5)
        return ALN_GENERIC;

    return ALN_NOERROR;
}

// debug version ASSERTS if bad params
#ifdef _DEBUG
static void DebugValidateALNCalcConfidence(const ALN* pALN,
    ALNDATAINFO* pDataInfo,
    const ALNCALLBACKINFO* pCallbackInfo,
    ALNCONFIDENCE* pConfidence)
{
    DebugValidateALNDataInfo(pALN, pDataInfo, pCallbackInfo);
    ASSERT(pConfidence->fltP > 0.0 && pConfidence->fltP < 0.5);
}
#endif
