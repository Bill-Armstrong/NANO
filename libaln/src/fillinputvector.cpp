// ALN Library
// Copyright (C) 2018 William W. Armstrong.
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// Version 3 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// 
// For further information contact 
// William W. Armstrong
// 3624 - 108 Street NW
// Edmonton, Alberta, Canada  T6J 1B4

// file  fillinputvector.cpp

#ifdef ALNDLL
#define ALNIMP __declspec(dllexport)
#endif

#include <aln.h>
#include "alnpriv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void ALNAPI FillInputVector(const ALN* pALN,
                            double* adblX, 
                            int nPoint,
                            int nStart,
                            ALNDATAINFO* pDataInfo,
                            const ALNCALLBACKINFO* pCallbackInfo)
{
  ASSERT(adblX);
  ASSERT(pALN);

  int nDim = pALN->nDim;  // register statement removed
  int nCols = pDataInfo->nTRcols; // This is 2 * nDim + 1
  const double* adblTRdata = pDataInfo->adblTRdata;
  const VARINFO* aVarInfo = pDataInfo->aVarInfo; // Not used

  // fill input vector
  VECTORINFO vectorinfo;        
  vectorinfo.bNeedData = FALSE;
  if (adblTRdata != NULL)
  {
    ASSERT(nStart == 0);	// we must start at zero, since no aVarInfo
    memcpy(adblX, adblTRdata + (nPoint * nCols), sizeof(double) * nDim); 
  }
  // send vector info message
	if (pCallbackInfo && CanCallback(AN_VECTORINFO, pCallbackInfo->pfnNotifyProc,
									pCallbackInfo->nNotifyMask))
	{
	vectorinfo.nPoint = nPoint + nStart; // nStart based
	vectorinfo.aVarInfo = aVarInfo;
	vectorinfo.adblX = adblX;
	Callback(pALN, AN_VECTORINFO, &vectorinfo, pCallbackInfo->pfnNotifyProc,
				pCallbackInfo->pvData);
	
	}
}