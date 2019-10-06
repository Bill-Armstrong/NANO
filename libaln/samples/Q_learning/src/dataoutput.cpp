// File: dataoutput.cpp
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
// Reinforcement learning tests

#include <stdlib.h>
#include <fstream>
#include "alnpp.h"
#include "aln.h"
#include "cmyaln.h"
using namespace std;
extern double ulimit;
extern ofstream pro;

class CMyAln;
int dynamics(double *, double *);

// dataoutput creates a raster image of the Q function 
int dataoutput()
{
	ofstream raster("Qraster.txt", ios_base::trunc);
	if (!raster.good())
	{
		cerr << "Opening raster image file failed" << endl;
	}
	else
	{
		pro << "Opening raster image file succeeded" << endl;
	}
	CMyAln aln;
	if(!(aln.Read("Qlearn.aln")))
	{
	std::cerr << "Qlearn.aln not read!" << endl;
	return 1;
	};
	// eval the ALN
	pro << "Evaluating the ALN to create a raster image... " << endl;
	// build data vectors	for a raster scan
	double adblX[4] = {0,0,0,0};
	const double m_dblMin1 = -10.0;
	const double m_dblMax1 = 10.0;
	const double m_dblMin2 = -10.0;
	const double m_dblMax2 = 10.0;
	const double m_nQuant1 = 80;
	const double m_nQuant2 = 80;
	const double dblStep1 = (m_nQuant1 == 1) ? 0.0 : ((m_dblMax1 - m_dblMin1) / (double)(m_nQuant1 - 1));
	const double dblStep2 = (m_nQuant2 == 1) ? 0.0 : ((m_dblMax2 - m_dblMin2) / (double)(m_nQuant2 - 1));
	adblX[0] = m_dblMin2;            
	for(int ii = 0; ii < m_nQuant2; ii++, adblX[0] += dblStep2)
	{
		for (int jj = 0; jj < m_nQuant1; jj++)
		{
			double dblDiff = dblStep1 * jj;
			adblX[1] = (ii % 2) ? (m_dblMin1 + dblDiff) : (m_dblMax1 - dblDiff);
			adblX[3] = 9999.0;				// not used as input to the ALNEval
			//check the first value of control (more generally done with the weights)
			adblX[2] = -ulimit;
 			ALNNODE*  pActiveLFN;
			double dblRes1 = aln.QuickEval( adblX, &pActiveLFN);
 			adblX[2] = 0;
			double dblRes2 = aln.QuickEval( adblX, &pActiveLFN);
 			adblX[2] = ulimit;
			double dblRes3 = aln.QuickEval( adblX, &pActiveLFN);
			// pick the best control value
			double dblMaxVatY = dblRes1;
			double u = -ulimit;
			if(dblRes2 > dblMaxVatY)
			{
				dblMaxVatY = dblRes2;
				u = 0;   
			}
			if(dblRes3 > dblMaxVatY)
			{
				dblMaxVatY = dblRes3;
					u = ulimit;
			}
			raster << adblX[0] << " " << adblX[1] <<" " << dblMaxVatY << " " << u << std::endl;
		}
	}
	aln.Destroy();
	return 0;
}

