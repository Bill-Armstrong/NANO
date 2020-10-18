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

// shuffle.cpp

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
// shuffle

void ALNAPI Shuffle(long nStart, long nEnd, long* anShuffle)
{
  ASSERT(anShuffle);

  if ((nEnd - nStart) > 1)
  {
  	for (int nSwap = nStart; nSwap <= nEnd; nSwap++)
  	{
  		// calc swap indexes
  	  int a, b;
  	  a = ALNRand() % (nEnd - nStart + 1);
  	  do { b = ALNRand() % (nEnd - nStart + 1); } while (a == b);

  	  // swap indexes
  		#define _SWAP(a, b) (a) = (a)^(b); (b) = (a)^(b); (a) = (a)^(b)
  		_SWAP(anShuffle[a], anShuffle[b]);
  		#undef _SWAP
  	} // end shuffle    
  }
}
