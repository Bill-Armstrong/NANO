# Running the programs

First of all you have to build all the projects.  I use the community version of Visual Studio 2019.

After seeing how these programs work, you can try your own data files, and make your own program, e.g. in Python, to run the NANO library using the NANO DLL(in the X64/lib directory). I'll try to get some documentation to help with that asap.

## To try think.exe

Look at the properties of the think project in VS, particularly Debugging.  That is where the data file is named and the rest of the command line is defined.  The program takes input data from the directory called Working in directory NANO and produces output there.  The file ExtendedTR.txt has an additional column with the learned ouput. For the sine-cosine problem, a function which has two inputs, you insert a scatterplot (using, say, MS Excel) from the right-hand two columns that shows how closely the noisy data is fitted.

One example input is VeryNoisy.txt which has a huge level of noise applied as the signal gets smaller. The goal of the program is to try to deal with a variable level of noise in data. The output is in the file ExtendedTR.txt which you can examine in a spreadsheet.  In MS Excel, for the VeryNoisy.txt input data, you open the file, highlight the rightmost two columns and insert a line chart which show the noisy data input and the NANO output ( which extends the input data by one column).

Project properties: Here is a typical command line for think.exe which contains some arguments for the program which can be placed in the Debugging > Command Arguments property:
"NoisySinCos20000_2.txt" "3" "20" "-90" "-1"  These are, in order,  "Data_file_name nDim nMaxEpochs dblMSEorF SKIPCOL". nDim is the number of inputs to the function to be learned plus one (for the single output). An epoch is one pass through the data in the buffer. In the last of nMaxEpochs in a call to Train, the hyperplanes which form pieces of the learned result can split in two. MSEorF shows the level of mean square training error of a piece above which it can split. If MSEorF is negative, the program does an F-test based on the estimated level of noise to decide on splitting.  The last argument causes the program not to learn using the data in the given column of the input data file.  Skipping column -1 does nothing, but you should omit column 1 in the VeryNoisy.txt example, or else you will get a fantastically good result because you have given away the answer. In addition you may need to change the working directory property for project think to: ..\\..\\..\\Working\\.

## To try Q_learning.exe

This shows NANO performing Q learning for the inverted pendulum problem. Just run the program (say inside Visual Studio "without debugging" ). It does a few iterations of Q learning then asks you for an integer number of iterations you want to do.  You can give it 100, 1000, 20000 or more. At the start 1000 iterations takes on the order of a minute or two.  As the program learns the Q function, the ALN tree grows and may end up with hundreds or thousands of leaf nodes. Then each iteration takes longer.  Each leaf node computes a flat surface defining a small part of the function surface. After the specified number of iterations, the program asks again how many more iterations you want.  If you input 0, then that finishes the program and you can open the file qmotion.txt in the directory "Working" using a spreadsheet like Excel.  Then you highlight the B column and insert a line chart which shows the angle of the pendulum over time.  The C column shows the angular velocity. Then comes the control given.  If you want to show the time in the first column, you can use scatterplots of the A column with others.  There are just three possible torque values which can be used: maximal clockwise torque (negative), maximal counter-clockwise torque and zero torque.

The qmotion_EXAMPLE_OUTPUT.txt result needed 21000 iterations, but I suggest you try 1000 additional iterations at first. Experimentation can show how to do the job with fewer iteations.

Project properties: change the working directory for this project to the same as above for think.
