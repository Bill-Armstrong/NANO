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

// plimit.cpp


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

// calculate probability p of an event occuring, such that
// the probablity of m or less such events occuring in n trials
// is x; currently limited to accuracy of 1.e-7

// implementation based on Ridders method documented in Press et al

// method: advance p until cumulative binomial dist 0 to m events in n 
//         trials drops to x

float ALNAPI PLimit(int n, int m, float fltX)
{
  static const float fltInc = 0.1;     // coarse increment
  static const float fltAcc = 1.0e-7;  // maximum accuracy

  if (fltX < 0.0 || fltX > 1.0 || n < 0)
  {
    return NAN;
  }
  
  // known cases
  if (m < 0)
    return 0.0;
  if (m >= n)
    return 1.0;

 
  // P is desired probability, Y is difference between desired area
  // under tail and the area under the tail given P

  // therfore, Y goes to 0 as P approaches desired value

  // lower bound
  float fltP1 = 0.0;
  float fltY1 = fltX - 1.0;
  ASSERT(fltY1 <= 0.0);

  // begin coarse approximation of P
  
  // scan upward until Y3 is +ve
  float fltP3, fltY3;
  for (fltP3 = fltInc; fltP3 < 1.0; fltP3 += fltInc)
  {
    fltY3 = fltX - (1.0 - ibeta(m + 1, n - m, fltP3));  //ibet??
                   // cumulative binomial dist 0 to m events in n trials, 
                   // see Press et al p229

    // convergence test (unlikely at this point)
    if (fabs(fltY3) < fltAcc)
      return fltP3;

    // check for sign change
    if (fltY3 > 0.0)
      break;  // we've bracketed desired value

    // else, new lower bound
    fltP1 = fltP3;
    fltY1 = fltY3;
  }

  // P1 and P3 bracket desired value... refine using ridders method
  const int nMaxIt = 100;
  
  for (int i = 0; i < nMaxIt; i++)
  {
    // get mid-values
    float fltP2 = 0.5 * (fltP1 + fltP3);
    if ((fltP3 - fltP1) < fltAcc)   // convergence test
      return fltP2;
    
    float fltY2 = fltX - (1.0 - ibeta(m + 1, n - m, fltP2)); //ibeta??

    // convergence test
    if (fabs(fltY2) < fltAcc)
      return fltP2;

    float fltDenom = sqrt(fltY2 * fltY2 - fltY1 * fltY3);  // y1, y3 opposite sign
    float fltTrial = fltP2 + (fltP1 - fltP2) * fltY2 / fltDenom;

    float fltY = fltX - (1.0 - ibeta(m + 1, n - m, fltTrial));  //ibeta

    // convergence test
    if (fabs(fltY) < fltAcc)
      return fltTrial;

    // between mid and test point?
    if ((fltY2 < 0.0) && (fltY > 0.0))
    {
      fltP1 = fltP2;    // new lower bound
      fltY1 = fltY2;
      fltP3 = fltTrial; // new upper bound
      fltY3 = fltY;
    }
    else if ((fltY < 0.0) && (fltY2 > 0.0))
    {
      fltP1 = fltTrial; // new lower bound
      fltY1 = fltY;
      fltP3 = fltP2;    // new upper bound
      fltY3 = fltY2;
    }
    else if (fltY < 0.0)  // both negative
    {
      fltP1 = fltTrial;
      fltY1 = fltY;
    }
    else  // both positive
    {
      fltP3 = fltTrial;
      fltY3 = fltY;
    }
  }

  // convergence failed... return best guess?
  return 0.5 * (fltP1 + fltP3); 
}