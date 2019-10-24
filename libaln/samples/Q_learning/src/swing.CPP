// ALN Library sample
// Reinforcement learning tests
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

//#include <stdlib.h>
#include <fstream>
#include <aln.h>
#include <alnpp.h> 
#include "cmyaln.h"
using namespace std;
class CMyAln;
void dynamics( double *, double *);
extern ofstream pro;
extern double deltat;
//extern double deltatsq;
extern double ulimit;
// The torque of the pendulum when held sideways is 0.166 Newton-metres
// and the torque ulimit is slightly greater

int swing()
{
	// open output file qmotion.txt
	pro << "Opening output file qmotion.txt... " << endl;
	ofstream of("qmotion.txt", ios_base::trunc);
	if (!of.good())
	{
	pro << "opening qmotion.txt file for output failed!" << endl;
	return -1;
	}
	else
	{
	pro << "opening qmotion.txt file for output succeeded!" << endl;
	}
	// reload the trained aln
	CMyAln aln;
	if (!aln.Read("Qlearn.aln"))
	{
		pro << "failed to read ALN!" << endl;
		return -1;
	}
	ALNNODE* pActiveLFN;
	// check the dynamic behavior of the pendulum
	double adblX[4] = {-1.3,0,0,0};	// start the pendulum off (angle, ang. vel.,torque u,Q_value)
	double dblRes1, dblRes2, dblRes3, dblMax,u;
	double adblY[2]; // the Y's are the values of adblX[0] and adblX[1] after one time-step
	for(int ii=0; ii<4000; ii++)
	{
		adblX[2] = -ulimit;
		dblRes1 = aln.QuickEval(adblX, &pActiveLFN);
		adblX[2] = 0;
		dblRes2 = aln.QuickEval(adblX, &pActiveLFN);
		adblX[2] = ulimit;
		dblRes3 = aln.QuickEval(adblX, &pActiveLFN);

		// pick the best control value
		dblMax = dblRes1;
		u = -ulimit;
		if(dblRes2 > dblMax)
		{
			dblMax = dblRes2;
			u = 0;   
		}
		if(dblRes3 > dblMax)
		{
			dblMax = dblRes3;
			u = ulimit;
		}
		double dblTime = ii * deltat;
		// choose the best torque and write a line to qmotion.txt
		of << dblTime << "\t" << adblX[0] << "\t" << adblX[1] <<"\t" << u << "\t" << dblMax << std::endl;
		adblX[2] = u; // Q_learning result
		//adblX[2] = 0; // MYTEST see how the pendulum works with zero torque applied;

		dynamics(adblX, adblY);
		adblX[0] = adblY[0];
		adblX[1] = adblY[1];
	}
	aln.Destroy();
	return 0;
}
