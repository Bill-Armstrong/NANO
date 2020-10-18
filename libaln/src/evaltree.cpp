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

// evaltree.cpp
// eval support routines


#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
static void DebugValidateEvalTreeInfo(const ALN* pALN,
                                      ALNDATAINFO* pDataInfo,
                                      const ALNCALLBACKINFO* pCallbackInfo,
                                      float* afltResult,
                                      int* pnStart, int* pnEnd,
                                      ALNNODE** apActiveLFNs,
                                      float* afltInput,
                                      float* afltOutput);
#endif

// evaluation of ALN on data

int ALNAPI EvalTree(const ALNNODE* pNode, 
                    const ALN* pALN,
                    ALNDATAINFO* pDataInfo,
                    const ALNCALLBACKINFO* pCallbackInfo,
                    float* afltResult,
                    int* pnStart, int* pnEnd,
                    BOOL bErrorResults /*= FALSE*/,
                    ALNNODE** apActiveLFNs /*= NULL*/,
                    float* afltInput /*= NULL*/,
                    float* afltOutput /*= NULL*/)
{
  ASSERT(pNode);
#ifdef _DEBUG
  DebugValidateEvalTreeInfo(pALN, pDataInfo, pCallbackInfo,
                            afltResult, pnStart, pnEnd, 
                            apActiveLFNs, afltInput, afltOutput);
#endif
  
  int nDim = pALN->nDim;
  int nDimt2p1 = 2 * nDim + 1;
  int nDimt2p1ti;
  long nTRcurrSamples = pDataInfo->nTRcurrSamples;

  // calc start and end points
  long nStart, nEnd; 
  //CalcDataEndPoints(nStart, nEnd, pALN, pDataInfo);
  nStart = 0;
  nEnd = nTRcurrSamples - 1;

  if (pnStart != NULL)
    *pnStart = nStart;
  if (pnEnd != NULL)
    *pnEnd = nEnd;

  // evaluation loop
  int nReturn = ALN_NOERROR;        // assume OK
  float* afltX = NULL;             // eval vector
  ALNNODE* pTree = pALN->pTree;		  // on stack for quicker access
  CCutoffInfo* aCutoffInfo = NULL;  
 
	try
 	{
    // reset active lfn array
    if (apActiveLFNs != NULL)
    {
      memset(apActiveLFNs, 0, pDataInfo->nTRcurrSamples * sizeof(ALNNODE*));
    }


  	// allocate input vector     
   	afltX = new float[nDim];   
    if (!afltX) ThrowALNMemoryException();
    memset(afltX, 0, sizeof(float) * nDim);

    // main loop
    ALNNODE* pActiveLFN = NULL;
    for (int i = nStart; i <= nEnd; i++)
    {
		nDimt2p1ti = nDimt2p1 * i;
      // fill input vector
      FillInputVector(pALN, afltX, i - nStart, nStart, pDataInfo, pCallbackInfo);

      // copy input vector?
      if (afltInput)
      {
        // get the input row
        float* afltRow = afltInput + nDimt2p1ti; // Changed this to give the correct buffer width.

        // copy values
        memcpy(afltRow, afltX, pALN->nDim * sizeof(float));
        
        // set the bias value in the output var spot
        afltRow[pALN->nOutput] = 1.0; // should we change to - 1.0 from 1.0 ?
      }

      // copy desired output 
      // ... do this before setting output value in input vector to zero below
      if (afltOutput)
      {
        afltOutput[i] = afltX[pALN->nOutput];
      }

      // CutoffEval returns distance from surface to point in the direction of
	    // the output variable, so we need to add that to the existing output value
      // to get the actual surface value
      if (!bErrorResults)
      {
        afltX[pALN->nOutput] = 0; // set output value to zero...

        // ... since output value is zero, the distance CutoffEval returns
        // is the value of the function surface
      }

      // get the distance from the point to the surface defined by the ALN
      afltResult[i] = CutoffEval(pTree, pALN, afltX, CEvalCutoff(),
                                 &pActiveLFN);
      
      // save the active LFN
      if (apActiveLFNs != NULL)
      {
        apActiveLFNs[i] = pActiveLFN;
      }
    }
  }
  catch (CALNUserException* e)
  {
  	nReturn = ALN_USERABORT;
    e->Delete();
  }
  catch (CALNMemoryException* e)
  {
  	nReturn = ALN_OUTOFMEM;
    e->Delete();
  }
  catch (CALNException* e)
  {
  	nReturn = ALN_GENERIC;
    e->Delete();
  }
  catch (...)
  {
  	nReturn = ALN_GENERIC;
  }

  // clear memory	
	delete[] afltX;
  return nReturn;
}

// debug version ASSERTS if bad params
#ifdef _DEBUG
static void DebugValidateEvalTreeInfo(const ALN* pALN,
                                      ALNDATAINFO* pDataInfo,
                                      const ALNCALLBACKINFO* pCallbackInfo,
                                      float* afltResult,
                                      int* pnStart, int* pnEnd,
                                      ALNNODE** apActiveLFNs,
                                      float* afltInput,
                                      float* afltOutput)
{
  DebugValidateALNDataInfo(pALN, pDataInfo, pCallbackInfo);
  
  ASSERT(afltResult != NULL);
}
#endif
