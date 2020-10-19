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

// alnconfidencetlimit.cpp


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"
#include "boost\math\special_functions\beta.hpp"
using namespace boost::math;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int ALNAPI ValidateALNConfidenceTLimit(const ALNCONFIDENCE* pConfidence,
    float fltInterval,
    float* pfltTLimit);

#ifdef _DEBUG
static void DebugValidateALNConfidenceTLimit(const ALNCONFIDENCE* pConfidence,
    float fltInterval,
    float* pfltTLimit);
#endif

// much of this is derived from theory documented in Master95 p302-323
// and Press et al p228-229

ALNIMP int ALNAPI ALNConfidenceTLimit(const ALNCONFIDENCE* pConfidence,
    float fltInterval,
    float* pfltTLimit)
{
    int nReturn = ValidateALNConfidenceTLimit(pConfidence, fltInterval, pfltTLimit);
    if (nReturn != ALN_NOERROR)
        return nReturn;

#ifdef _DEBUG
    DebugValidateALNConfidenceTLimit(pConfidence, fltInterval, pfltTLimit);
#endif

    // calc number of samples in tail
    int nTailSamples = (int)floor((float)pConfidence->nSamples * pConfidence->fltP - 1);

    // calculate probablity of exceeding desired interval
    *pfltTLimit = (float)1.0 - ibeta((float)(pConfidence->nSamples - 2 * nTailSamples + 1), // need incomp beta fn
        (float)(2 * nTailSamples),
        fltInterval);

    return nReturn;
}


// validate params
static int ALNAPI ValidateALNConfidenceTLimit(const ALNCONFIDENCE* pConfidence,
    float fltInterval,
    float* pfltTLimit)
{
    if (pConfidence == NULL || pfltTLimit == NULL)
        return ALN_GENERIC;

    return ALN_NOERROR;
}

// debug version ASSERTS if bad params
#ifdef _DEBUG
static void DebugValidateALNConfidenceTLimit(const ALNCONFIDENCE* pConfidence,
    float fltInterval,
    float* pfltTLimit)
{
    ASSERT(fltInterval >= 0.0 && fltInterval <= 1.0);
}
#endif
