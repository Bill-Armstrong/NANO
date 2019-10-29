# Running the programs

First of all you have to build all the projects.  I use the community version of Visual Studio 2019 and do a batch build of everything.
The first thing you need to change is the working directory for both project think and for project Q_Learning to: ..\\..\\..\\Working\\.  I'll tell you below what you need to change in addition before running the program think.exe.

After seeing how these programs work, you can try your own data files, and, if you like, make your own program in your own favorite language to run using the NANO library DLL(in the X64/lib directory). I'll try to get some documentation to you asap.

## To try think.exe

Look at the properties of the think project in VS, particularly Debugging.  That is where the data file is named and the rest of the command line is defined.  The program takes input data from the directory called Working in NANO and produces output there.  There are two easy example data files and one very challenging one. The example VeryNoisyGaussianSine20000.txt has a growing level of noise applied as the signal gets smaller. One goal of the NANO library is to try to deal with a variable level of noise in data. It does this by measuring the level of noise in the data and stopping training by stopping breaking of linear pieces when the training error gets smaller than the noise level. (More accurately, it does an F-test). The output of think.exe is in the file ExtendedTR.txt which you can examine in a spreadsheet.  In MS Excel you open that file, and for a function of one input, you highlight the rightmost two columns and insert a line chart which shows the noisy data input and the NANO output ( which extends the input data by one column). For the sine-cosine problem which has two inputs, you insert a scatterplot that shows how close the noisy data is fitted.

Set think as your startup project.  Here is a typical command line for think.exe which you can set in the Debugging property of the project. It contains arguments:
"NoisySinCos20000.txt 3 20 -55 1 2 3"  These are  "Data_file_name nDim (the number of columns input to the ALN, including the desired output), nMaxEpochs (the number of epochs, passes throgh all the data in the buffer, during the last of which pieces can break in two),  dblMSEorF (either the set level of noise or, when negative, a probability value for the F-test.)  What follows those four inputs are the column numbers of the data file which are input to NANO. The columns are numbered starting at 1.  The last column in the list contains the desired output, usually corrupted by noise.  You should omit column 2 in the VeryNoisyGaussianSine20000.txt example, or else you will get a fantastically good result because you have given away the answer.

## To try Q_learning.exe

This shows NANO performing Q learning for the inverted pendulum problem. Just run the program (say inside Visual Studio "without debugging" ). It does a few iterations of Q learning then asks you for an integer number of iterations you want to do.  You can give it 100, 1000 or more. At the start 1000 iterations takes on the order of a minute or two.  As the program learns the Q function, the ALN tree grows and may end up with hundreds or thousands of leaf nodes. Then each iteration takes longer.  Each leaf node computes a flat surface defining a small part of the continuous function approximant. After the specified number of iterations, the program asks again how many more you want.  If you input 0, then that finishes the program and you can open the file qmotion.txt in the directory "Working" using a spreadsheet like Excel.  Then you highlight the B column and insert a line chart which shows the angle of the pendulum over time.  The third column shows the angular velocity. Then comes the control given.  There are just three possible torque values which can be used: maximal clockwise torque (negative), maximal counter-clockwise torque and zero torque. Please try the given parameters first, and after building the program and starting the run specify 10000 iterations. It is hard to find good parameters to make this work, but in reality, you are dealing with a problem of solving ordinary differential equations using a larger time step than is desirable. This required artificial damping and that means you aren't simulating a real pendulum.  

For viewing the Qraster.txt file, I used a free program called QuickGrid. After installing and opening it, you use the menu to input the contents of a control file, e.g. mydata.txt shown below, then input the Qraster.txt file, change to 3D display and add color. The Qraster_EXAMPLE_OUTPUT.txt file took 40005 iterations, over a billion adaptations to an input pattern, the NANO tree grew to 4400 nodes, but the resulting Q function is quite beautiful in color.

    Edit Grid Template -10 0.5063 10 -6.283 0.10084 3.1416
    File Input Metric data points
    Grid Automatic grid resolution ON

I hope someone will try NANO on a real live pendulum. That's what *online* in the name NANO means. Good luck!

Bill Armstrong
