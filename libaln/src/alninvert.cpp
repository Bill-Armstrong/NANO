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

// alninvert.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// helper decl
void DoInvert(ALNNODE* pTree, ALN* pALN, int nVar, int nMono);
void InvertMinMax(ALNNODE* pTree, ALN* pALN, int nVar, int nMono);
void InvertLFN(ALNNODE* pTree, ALN* pALN, int nVar, int nMono);
void InvertConstraints(ALN* pALN, int nVar);

ALNIMP int ALNAPI ALNInvert(ALN* pALN, int nVar)
{
    // parameter variance
    if (pALN == NULL)
        return ALN_GENERIC;

    if (nVar < 0 || nVar >= pALN->nDim)
        return ALN_GENERIC;

    if (nVar == pALN->nOutput)
        return ALN_NOERROR; // nothing to change

      // check current monotonicity of desired output
    int nMono = CheckMonotonicity(pALN->pTree, pALN, nVar);
    if (nMono == MONO_STRONGINC || nMono == MONO_STRONGDEC ||
        nMono == MONO_WEAKINC || nMono == MONO_WEAKDEC)
    {
        DoInvert(pALN->pTree, pALN, nVar, nMono);
        InvertConstraints(pALN, nVar);

        ASSERT(pALN->nOutput == nVar);

        return ALN_NOERROR;
    }
    else
    {
        return ALN_GENERIC;
    }
}

void DoInvert(ALNNODE* pTree, ALN* pALN, int nVar, int nMono)
{
    ASSERT(nMono == MONO_STRONGINC || nMono == MONO_STRONGDEC ||
        nMono == MONO_WEAKINC || nMono == MONO_WEAKDEC);

    // minmax type
    if (NODE_ISMINMAX(pTree))
    {
        InvertMinMax(pTree, pALN, nVar, nMono);
    }
    else
    {
        InvertLFN(pTree, pALN, nVar, nMono);
    }
}

void InvertMinMax(ALNNODE* pTree, ALN* pALN, int nVar, int nMono)
{
    ASSERT(NODE_ISMINMAX(pTree));

    // save current minmax type
    int nMinMaxType = MINMAX_TYPE(pTree);

    // clear current minmax type
    pTree->fNode &= ~(GF_MAX | GF_MIN);

    // this node is a MIN iff AND and decreasing mono, or OR and increasing mono
    // else node is a MAX iff AND and increasing mono, or OR and decreasing mono
    // NOTE: MONO_CONSTANT will be treated as decreasing!

    // since we constrain output var to have weights of -1, 
    // MIN becomes AND, and MAX becomes OR

    // set new minmax type
    BOOL bIncreasing = ((nMono == MONO_STRONGINC) || (nMono == MONO_WEAKINC));
    if (((nMinMaxType & GF_MIN) && !bIncreasing) || ((nMinMaxType & GF_MAX) && bIncreasing))
        pTree->fNode |= GF_MIN;
    else
        pTree->fNode |= GF_MAX;

    // recurse to children
    DoInvert(MINMAX_LEFT(pTree), pALN, nVar, nMono);
    DoInvert(MINMAX_RIGHT(pTree), pALN, nVar, nMono);
}

void InvertLFN(ALNNODE* pTree, ALN* pALN, int nVar, int nMono)
{
    ASSERT(NODE_ISLFN(pTree));

    // get new output var weight
    float* afltW = LFN_W(pTree);
    float* afltC = LFN_C(pTree); // WWA

    float fltWOutput = afltW[nVar + 1];  // account for bias weight

    if (fltWOutput == 0)
    {
        ASSERT(nMono == MONO_WEAKINC || nMono == MONO_WEAKDEC || nMono == MONO_CONSTANT);

        // adjust weight to be non-zero by dividing current output var epsilon
        // with range of desired output var at the topmost region... this is the
        // smallest meaningful value that a weight can take (the largest is the
        // range of current output divided by epsilon of desired output)

        // get current output var epsilon
        ASSERT(nVar != pALN->nOutput);
        // otherwise weight would be non-zero (-1) on def output var!

        ALNCONSTRAINT* pConstrOut = GetVarConstraint(NODE_REGION(pTree), pALN,
            pALN->nOutput);
        ASSERT(pConstrOut);
        float fltEpsilon = pConstrOut->fltEpsilon;

        // get min and max on new output var in topmost region
        ASSERT(pALN->aRegions[0].aConstr[nVar].nVarIndex == nVar);
        float fltMin = pALN->aRegions[0].aConstr[nVar].fltMin;
        float fltMax = pALN->aRegions[0].aConstr[nVar].fltMax;

        // calc new output
        fltWOutput = fltEpsilon / (fltMax - fltMin);
        ASSERT(fltWOutput > 0);

        // determine sign of output weight
        if (nMono != MONO_WEAKINC && nMono != MONO_STRONGINC)
            fltWOutput *= -1; // make it a decreasing function

          // otherwise leave it positive
    }

    ASSERT(fltWOutput != 0); // must not have a zero weight!

    // re-normalize weights so new output var is -1
    float fltFactor = -1.0f / fltWOutput;

#ifdef _DEBUG
    if (nMono == MONO_STRONGDEC || nMono == MONO_WEAKDEC || nMono == MONO_CONSTANT)
    {
        // expect factor to be positive
        ASSERT(fltFactor > 0);
    }
    else
    {
        // expect factor to be negative 
        ASSERT(fltFactor < 0);
    }
#endif

    int nDim = pALN->nDim;
    for (int i = 0; i < nDim; i++) // don't include bias WWA
    {
        afltW[i + 1] *= fltFactor;
    }
    // explicitly set output var weight in case original afltW[nVar] was zero
    afltW[nVar + 1] = -1.0; // account for bias weight

      // recalculate the bias weight from the new weights and the centroid WWA
    float sum = 0.0;
    for (int i = 0; i < nDim; i++) //WWA
    {
        sum += afltW[i + 1] * afltC[i];  // there is no bias centroid component WWA
    }
    afltW[0] = -sum;
}

void InvertConstraints(ALN* pALN, int nVar)
{
    ASSERT(nVar >= 0 && nVar < pALN->nDim);
    ASSERT(nVar != pALN->nOutput);

    // for all regions force weight constraints on new output var, and relax constraints
    // on old output var
    int nRegions = pALN->nRegions;
    for (int i = 0; i < nRegions; i++)
    {
        ALNREGION* pRegion = pALN->aRegions + i;

        // Very loose bounds are usually required after inversion
        // In those cases where tighter bounds are possible, they are useless

        ASSERT(pALN->aRegions->nConstr == pALN->nDim);
        for (int n = 0; n < pALN->nDim; n++)
        {
            pALN->aRegions->aConstr[n].nVarIndex = n;
            // leave the existing ranges of variables unchanged
            if (n != pALN->nOutput)
            {
                pALN->aRegions->aConstr[n].fltWMin = -1000000.0F;
                pALN->aRegions->aConstr[n].fltWMax = 1000000.0F;
            }
            else
            {
                pALN->aRegions->aConstr[n].fltWMin = -1.0F;
                pALN->aRegions->aConstr[n].fltWMax = -1.0F;
            }
        }
    }
    // set new output var
    pALN->nOutput = nVar;
}