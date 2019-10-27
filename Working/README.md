# Running the programs

First of all you have to build all the projects.  I use the community version of Visual Studio 2019.  You may have to build a second time to catch two failures because of build order. Also it seems the project files didn't push properly.  I'll tell you what you need in the project properties below.

After seeing how these programs work, you can try your own data files, and, if you like, make your own e.g. Python program to run the NANO library using the NANO DLL(in the X64/lib directory). I'll try to get some documentation to you asap.

## To try think.exe

Look at the properties of the think project in VS, particularly Debugging.  That is where the data file is named and the rest of the command line is defined.  The program takes input data from the directory called Working in NANO and produces output there.  One example input is VeryNoisyGaussianSine.txt which has a huge level of noise applied as the signal gets smaller. The goal of the program is to try to deal with a variable level of noise in data. The output is in the file ExtendedTR.txt which you can examine in a spreadsheet.  In MS Excel you open that file, highlight the rightmost two columns and insert a line chart which show the noisy data input and the NANO output ( which extends the input data by one column). For the sine-cosine problem which has two inputs, you insert a scatterplot that shows how close the noisy data is fitted.

Here is a typical command line for think.exe which contains some arguments:
"NoisySinCos20000_2.txt" "3" "20" "-90" "-1"  These are  "Data_file_name nDim nMaxEpochs dblMSEorF SKIPCOL"   The last argument causes the program not to learn using the data in the given column of the input data file.  Skipping column -1 does nothing, but you should omit column 1 in the VeryNoisyGaussianSine.txt example, or else you will get a fantastically good result. 

In addition you need to change the working directory for project think and for Q-Learning to: ..\..\..\Working\

## To try Q_learning.exe

This shows NANO performing Q learning for the inverted pendulum problem. Just run the program (say inside Visual Studio "without debugging" ). It does a few iterations of Q learning then asks you for an integer number of iterations you want to do.  You can give it 100, 1000 or more. I suggest 10000. At the start 1000 iterations takes on the order of a minute or two.  As the program learns the Q function, the ALN tree grows and may end up with hundreds or thousands of leaf nodes. Then each iteration takes longer.  Each leaf node computes a flat surface defining a small part of the function surface. After the specified number of iterations, the program asks again how many more you want.  If you input 0, then that finishes the program and you can open the file qmotion.txt in the directory "Working" using a spreadsheet like Excel.  Then you highlight the B column and insert a line chart which shows the angle of the pendulum over time.  The third column shows the angular velocity. Then comes the control given.  There are just three possible torque values which can be used: maximal clockwise torque (negative), maximal counter-clockwise torque and zero torque. Please try the given parameters first, and after building the program and starting the run specify 10000 iterations. It is hard to find good parameters to make this work, but in reality, you are dealing with a problem of solving ordinary differential equations using a larger time step than is desirable. This required artificial damping and that means you aren't simulating a real pendulum.  

For viewing the Qraster.txt file, I used a free program called QuickGrid. After installing and opening it, you use the menu to input the contents of a control file, e.g. mydata.txt below, then input the Qraster.txt file, then change to 3D display and add color. The Qraster_EXAMPLE_OUTPUT.txt file took 40005 iterations, but is quite beautiful.

Edit Grid Template -10 0.5063 10 -6.283 0.10084 3.1416 
File Input Metric data points 
Grid Automatic grid resolution ON


I hope someone will try NANO on a real live pendulum. That's what *online* in the name NANO means. Good luck! 
