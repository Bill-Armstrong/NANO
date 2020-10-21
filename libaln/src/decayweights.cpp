

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

// decayweights.cpp

#include "aln.h"
#include "alnpriv.h"
#include <iostream>
#include <algorithm>
extern BOOL bClassify2;

ALNIMP void ALNAPI DecayWeights(const ALNNODE* pNode, const ALN* pALN, float WeightBound, float WeightDecay)
{
    // This routine should not be called except for classification tasks
    if (NODE_ISMINMAX(pNode))
    {
        DecayWeights(MINMAX_RIGHT(pNode), pALN, WeightBound, WeightDecay);
        DecayWeights(MINMAX_LEFT(pNode), pALN, WeightBound, WeightDecay);
    }
    else
    {
        ASSERT(NODE_ISLFN(pNode));
        if (NODE_ISCONSTANT(pNode))return;
        int nDimm1 = pALN->nDim - 1; // The assumed output axis
        // If the centroid is not near the constant level pieces, leave it alone
        // Otherwise we move the centroid towards a place where the output value is 0.
        // That should place it between the target class and some others.
        // Then when we increase the slope of the piece by calling this routine with WeightDecay > 1.0, the rotation about the centroid
        // should pull away from all samples both at the -1 and +1 levels.
        // We have to stop the turning if a weight bound is reached
        float *pW, *pWkeep, priorOutC, newOutC, check;
        pW = pWkeep = LFN_W(pNode);
        pW++; // Shift by 1 to ignore the bias weight, unshifted is stored as pWkeep
        // We allow rotations around the nDim -2-dimensional hyperplane where the LFN has value 0
        // Assume pW[i] satisfies the weight bound before the call to this routine
        float* pC = LFN_C(pNode);
        priorOutC = pC[nDimm1]; // We want to move the output centroid to level 0 in the direction of maximum slope magnitude
        // If the rotation hits a weight bound, that will rotate the LFN in an undesired directon, so we find
        // the maximum factor by which we can change the weights without hitting a bound.
        float maxDecay = WeightDecay;
        float accu = 0;
        for (int i = 0; i < nDimm1; i++)
        {
            accu += pW[i] * pW[i];
            if (fabs(pW[i]) * maxDecay > WeightBound) maxDecay = WeightBound / fabs(pW[i]); 
            // We shall never have to divide if the denominator is close to 0 as long as the bound is positive
        }
        if (accu < 0.000001F) return;  // If the piece is too flat, the centroid would be too far away from the current position 
        // Note that this depends on the, perhaps unknown, distance between classes!

        // Now we change the centroids and the weights in the domain axes(no need to test if bounds are breached)
        float factor = pC[nDimm1] / accu;
        newOutC = priorOutC; // We have the starting point for the new centroid and the increases:
        for (int i = 0; i < nDimm1; i++)
        {
            // First we move the centroid partly towards the place where the ALN has value 0
            // the amount moved in the direction i is proportional to pW[i]
            pC[i] -= factor * pW[i];
            newOutC -= factor * pW[i] * pW[i];
            // now we change the weight (it must be within bounds)
            pW[i] *= maxDecay;
        }
        pC[nDimm1] = newOutC; // update the output centroid.

        // If all the domain weights and the *bias* value are all changed by the same factor
        // the place where the LFN value is 0 does not change.
        // Now we compress the weighted centroid info into afltW[0]
        check = *pWkeep * maxDecay; // what we expect the new bias to be
        *pWkeep = newOutC;
        for (int i = 0; i < nDimm1; i++)
        {
            *pWkeep -= pW[i] * pC[i]; // here the pW pointer is still shifted up by one float pointer
        }
        if(fabs(priorOutC) > 0.1 && maxDecay > 1.1) std::cout << "\n  Output centroid value before DecayWeights = " <<
            priorOutC << " and after = " << newOutC << " Is check at 0 ? -> " << *pWkeep - check <<std::endl;
    }
}
