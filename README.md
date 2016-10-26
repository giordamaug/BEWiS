# BEWiS
Background Estimation by Weightless Neural Networks

Authors: Maurizio Giordano and Massimo De Gregorio
Institution:  Consiglio Nazionale delle Rierche (Italy)

----------------------
Description
----------------------

BEWiS is a background modeling approach for videos based on a weightless neural system, 
namely WiSARD, with the aim of exploiting its features of being highly adaptive and 
noise–tolerance at runtime.
In BEWiS, the changing pixel colors in a video are processed by a an incremental 
learning neural network with a limited-in-time memory-retention mechanism that allow the
proposed system to absorb small variations of the learned model (background) 
in the steady state of operation as well as to  fastly adapt to background 
changes during the video timeline.

----------------------
Citation Details
----------------------
  
Please cite the following journal article when using this source code:

 M. De Gregorio, M. Giordano.
 Background Modeling by Weightless Neural Networks.
 in "New Trends in Image Analysis and Processing - ICIAP 2015 Workshops", 
 Volume 9281 of the series Lecture Notes in Computer Science, pp 493-501.
 Springer Verlag, http://dx.doi.org/10.1007/978-3-319-23222-5_60 

----------------------
License
----------------------
  
The source code is provided without any warranty of fitness for any purpose.
You can redistribute it and/or modify it under the terms of the
GNU General Public License (GPL) as published by the Free Software Foundation,
either version 3 of the License or (at your option) any later version.
A copy of the GPL license is provided in the "GPL.txt" file.

----------------------
Instructions and Notes
----------------------

To run the code the following libraries must be installed:

1. OpenCV 3.0 (later versions may also work)

2. CMake  3.0  (later version may also work)

3. C++ Compiler (tested only with GCC 5.x or later versions)

<b>Compile source (Linux, Mac OSX):</b>

$ cmake .

$ make

$ bewis  -i \<inputidr\>




