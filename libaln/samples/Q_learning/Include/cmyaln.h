//file cmyaln.h
// ALN Library sample
// License: LGPL
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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

#include <iostream>
extern double dblEps3;
class CMyAln;
void genvector(CMyAln*);

class CMyAln : public CAln
{
  // notification overrides
  public:
  virtual BOOL OnTrainStart(TRAININFO* pTrainInfo, void* pvData)
    { 
      std::cout << "Training starts..." << std::endl;
      return TRUE; 
    }

  virtual BOOL OnTrainEnd(TRAININFO* pTrainInfo, void* pvData) 
  { 
	  std::cout << "This iteration of training finished.  RMSE: " << pTrainInfo->dblRMSErr << std::endl;
      return TRUE; 
  }
  
  virtual BOOL OnEpochStart(EPOCHINFO* pEpochInfo, void* pvData) 
  { 
    std::cout << "Epoch " << pEpochInfo->nEpoch << ":" << std::endl; 
    return TRUE; 
  }
  
  virtual BOOL OnEpochEnd(EPOCHINFO* pEpochInfo, void* pvData) 
  {
	  //std::cout << "  Estimated RMSE: " << pEpochInfo->dblEstRMSErr << std::endl;
	  std::cout << "Number of active leaf nodes " << pEpochInfo->nActiveLFNs << " Total LFNs " << pEpochInfo->nLFNs << std::endl;
      dblEps3 = pEpochInfo->dblEstRMSErr;
      return TRUE;
  }

  virtual BOOL OnVectorInfo(VECTORINFO* pVectorInfo, void* pvData) 
  {
	  //genvector(this);
	  return TRUE;
  }

};

