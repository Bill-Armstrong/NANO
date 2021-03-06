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

// adaptminmax.cpp


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
// minmax specific adapt

static const float fltRespThresh = 1.0;//.9999999999; 
static const float fltRespMin = 0.0;

void ALNAPI AdaptMinMax(ALNNODE* pNode, ALN* pALN, const float* afltX,
    float fltResponse, BOOL bUsefulAdapt,
    const TRAINDATA* ptdata)
{
    ASSERT(NODE_ISMINMAX(pNode));
    ASSERT(NODE_ISEVAL(pNode));
    ASSERT(ptdata != NULL);

    if (bUsefulAdapt)
    {
        NODE_RESPCOUNT(pNode)++;
    }

    // get output var constraint
    ALNCONSTRAINT* pConstrOutput = GetVarConstraint(NODE_REGION(pNode), pALN, pALN->nOutput);

    // get children
    ALNNODE* pChild0 = MINMAX_LEFT(pNode);
    ALNNODE* pChild1 = MINMAX_RIGHT(pNode);
    BOOL bUsefulAdapt0 = FALSE;
    BOOL bUsefulAdapt1 = FALSE;

    // get resp counts
    int nResp0 = NODE_RESPCOUNT(pChild0) + NODE_RESPCOUNTLASTEPOCH(pChild0);
    int nResp1 = NODE_RESPCOUNT(pChild1) + NODE_RESPCOUNTLASTEPOCH(pChild1);

    // calculate the responsibilities of the children
    float fltResp0, fltResp1;
    float fltRespActive = MINMAX_RESPACTIVE(pNode);

    ASSERT(MINMAX_ACTIVE(pNode) != NULL);
    if (MINMAX_ACTIVE(pNode) == pChild0) // child 0 active
    {
        bUsefulAdapt0 = bUsefulAdapt;

        float fltR;

        if ((fabs(ptdata->fltGlobalError) > pConstrOutput->fltEpsilon) &&
            (fltRespActive > fltRespThresh) && (nResp1 < nResp0))
        {
            // bring in useless piece 1
            fltR = fltRespThresh;

            // eval if necessary before adapting
            if (!NODE_ISEVAL(pChild1))
            {
                ALNNODE* pActiveLFN1;
                int nCutoffs = 0;
                AdaptEval(pChild1, pALN, afltX, CEvalCutoff(), &pActiveLFN1);
            }
        }
        else
        {
            fltR = fltRespActive;
        }

        // divide each quantity by 1 - 2r(1-r)

        float fltFactor = 1.0f / (1 - 2 * fltR * (1 - fltR));

        fltResp0 = fltR * fltFactor;
        fltResp1 = (1.0f - fltR) * fltFactor;
    }
    else // child 1 is active
    {
        bUsefulAdapt1 = bUsefulAdapt;

        float fltR;

        if ((fabs(ptdata->fltGlobalError) > pConstrOutput->fltEpsilon) &&
            (fltRespActive > fltRespThresh) && (nResp0 < nResp1))
        {
            // bring in useless piece 0
            fltR = fltRespThresh;

            // eval child 0 if necessary before adapting
            if (!NODE_ISEVAL(pChild0))
            {
                ALNNODE* pActiveLFN0;
                int nCutoffs = 0;
                AdaptEval(pChild0, pALN, afltX, CEvalCutoff(), &pActiveLFN0);
            }
        }
        else
        {
            fltR = fltRespActive;
        }

        // divide each quantity by 1 - 2r(1-r)

        float fltFactor = 1.0f / (1 - 2 * fltR * (1 - fltR));

        fltResp1 = fltR * fltFactor;
        fltResp0 = (1.0f - fltR) * fltFactor;
    }

    // adapt child 0
    fltResp0 *= fltResponse;
    if (fltResp0 > fltRespMin)
    {
        Adapt(pChild0, pALN, afltX, fltResp0, bUsefulAdapt0, ptdata);
    }

    // adapt child 1
    fltResp1 *= fltResponse;
    if (fltResp1 > fltRespMin)
    {
        Adapt(pChild1, pALN, afltX, fltResp1, bUsefulAdapt1, ptdata);
    }
}


