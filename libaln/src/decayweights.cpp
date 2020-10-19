

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
        float* pC = LFN_C(pNode);
        int nDimm1 = pALN->nDim - 1; // The assumed output axis
        // if (fabs(pC[nDimm1]) < 0.75) return; // If the centroid is not near the constant level pieces, leave it alone
        // Otherwise we move the centroid towards a place where the output value is 0.
        // That should place it between the target class and some others.
        // Then when we increase the slope of the piece by calling this routine with WeightDecay > 1.0, the rotation about the centroid
        // should pull away from all samples both at the -1 and +1 levels.
        float lambda = 1.0F; // lambda = 1.0 means the centroid goes all the way to level 0, if lambda < 1, we go part way.
        float Cout_inc = 0;
        float* pW = LFN_W(pNode);
        pW++; // Shift by 1 to ignore the bias weight, we don't need the value.
        float Wtemp, priorC;
        priorC = pC[nDimm1];
        float lambdaOvernDimm1 = lambda / (float)nDimm1;
        for (int i = 0; i < nDimm1; i++) // change the weights in the domain axes, testing if bounds are breached
        {
            if (fabs(pW[i]) < 0.006) continue; // We wait until the sign of pW[i] is clear before increasing or decreasing the weight (i.e.  increasing the magnitude)
            // This value may have to be changed later; it should be less than 1.0 / (minimum distance between current or anticipated classes)
            Wtemp = pW[i];
            //first we move the centroid towards the place where the ALN has value 0 in the direction i (just a fraction of it) 
            pC[i] -= lambdaOvernDimm1 * pC[nDimm1] / pW[i];
            // now we change the weight and bound it
            pW[i] *= WeightDecay;
            pW[i] = std::max(std::min(WeightBound, pW[i]), -WeightBound);
            Cout_inc += pW[i] / Wtemp; // The increment will be WeightDecay if no weight has hit the bound
        }
        Wtemp = pC[nDimm1] *= lambdaOvernDimm1 * Cout_inc; // This is the average factor, and if no weight hit a bound then pW[0] becomes pW[0] * WeightDecay
        // compress the weighted centroid info into W[0]
        for (int i = 0; i < nDimm1; i++)
        {
            Wtemp -= pW[i] * pC[i]; // here the pW pointer is still shifted up by one float pointer
        }
        pW = LFN_W(pNode); // Get the unshifted weight vector
        *pW = Wtemp;
        //There is some inaccuracy if some weight changes hit the bound, the WeightDecay is assumed to be close to 1.0
        // std::cout << "\n  Output centroid value before DecayWeights = " << priorC << " and after = " << pC[nDimm1] << std::endl;
    }
}
