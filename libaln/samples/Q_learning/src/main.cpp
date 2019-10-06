// main.cpp
// Pendulum Swing-Up -- Q-learning test using NANO
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

extern int Qlearn();      // The training result for an ALN is saved in Qlearn.aln
extern int dataoutput();  // Creates data for graphical display of the state-action Q function
extern int swing();       // Reads the ALN from Qlearn.aln and outputs qmotion.txt to show the swing
// A line graph of the left-hand column of qmotion.txt MS Excel traces the pendulum motion

int main()
{
  Qlearn();
  dataoutput();
  swing();
  return 0;
}
