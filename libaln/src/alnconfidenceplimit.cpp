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

// alnconfidenceplimit.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int ALNAPI ValidateALNConfidencePLimit(const ALNCONFIDENCE* pConfidence,
                                              float fltSignificance,
                                              float* pfltPLimit);

#ifdef _DEBUG
static void DebugValidateALNConfidencePLimit(const ALNCONFIDENCE* pConfidence,
                                             float fltSignificance,
                                             float* pfltPLimit);
#endif

// much of this is derived from theory documented in Master95 p302-323
// and Press et al p228-229

ALNIMP int ALNAPI ALNConfidencePLimit(const ALNCONFIDENCE* pConfidence,
                                      float fltSignificance,
                                      float* pfltPLimit)
{
	int nReturn = ValidateALNConfidencePLimit(pConfidence, fltSignificance, pfltPLimit);
  if (nReturn != ALN_NOERROR)
    return nReturn;
  
  #ifdef _DEBUG
    DebugValidateALNConfidencePLimit(pConfidence, fltSignificance, pfltPLimit);
  #endif

  // calc number of samples in tail
  int nTailSamples = (int)floor((float)pConfidence->nSamples * pConfidence->fltP - 1);
  
  // calculate limit at desired significance level
	*pfltPLimit = PLimit(pConfidence->nSamples, nTailSamples + 1, fltSignificance);

	return nReturn;
}


// validate params
static int ALNAPI ValidateALNConfidencePLimit(const ALNCONFIDENCE* pConfidence,
                                              float fltSignificance,
                                              float* pfltPLimit)
{
  if (pConfidence == NULL || pfltPLimit == NULL)
    return ALN_GENERIC;

  return ALN_NOERROR;
}

// debug version ASSERTS if bad params
#ifdef _DEBUG
static void DebugValidateALNConfidencePLimit(const ALNCONFIDENCE* pConfidence,
                                             float fltSignificance,
                                             float* pfltPLimit)
{
  ASSERT(fltSignificance >= 0.0 && fltSignificance <= 1.0);
}
#endif
