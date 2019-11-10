README
**********
This application has been tested on a Windows 10 system.

###########################################
#USING THE C++ SHAPE RETRIEVAL APPLICATION#
###########################################

1.Ensure you are in the (aptly named) ConsoleApplication1 folder within the solution.
2.Run 3D Model Query System.exe.
3.Paste a path to an .off file. The system automatically replaces '\' with '/'. 
(The entire PSB database can be found in the InputDB folder)
4.After the processing and matching has been done, press 'n' to load the next model in the result queue.
5.For additional queries, the application must be restarted.

Notes:
- The processed and normalized database of 3D models is in the ShapeDB folder.
- Output folder contains several statistics extracted from the shapes, as well as the feature table for quick use.
- In main.cpp, the method prepareMesh wraps the whole normalization pipeline. The framework would crash when computing
normals after changing the number of vertices/faces, so an additional input.off is created with the normalized file,
which is then opened again and has its normals computed.
- Weights can be modified in the main method by changing the weights array.
- A new database of processed shapes can be created using the createDatabase function in main, however this takes
a very long time and may need to be manually debugged if something does not work.
- The method read from the PlyReader class can read PLY files, which can be saved as OFF using WriteFileOFF from the 
OFFConverter class. There is also a method for directly converting an OFF file to a PLY file, but it is not used.

############################################
##USING THE PYTHON T-SNE VISUALIZATION APP##
############################################

1.Open the t-SNE folder in a Python IDE.
2.Make sure the following are installed:
    -numpy
    -pandas
    -matplotlib
    -seaborn
3.Configure main entry file to be tsne_wrapper.py
4.Run the application.
5.You will be asked whether you want to reload the previous t-SNE plot.
    -Yy: the application will plot "tsneresult.txt" from the t-SNE folder. See notes.
    -Nn: the application will run the t-SNE algorithm on the feature space again using the perplexity at the top
         of the script. This will take a few minutes. The result will be displayed and saved as "tsneresult.txt".

Notes:
- Pressing the slider button in the bottom left will enable plot resizing, as well as the possibility of making 
the legend visible and the plot itself bigger.
- Pressing 'x' will enable labels showing shape name and class for ALL points. As there are almost 2k points, it in
advisable to only press 'x' after zooming in on the plot, otherwise it will take a minute to load them all and the
result will be hard to read. Pressing 'x' again hides the labels.
- The t-SNE folder contains a few saved t-SNE results with their perplexity and error in the name. To view any of them,
make a copy in the same folder and rename it to "tsneresult.txt", then run the app andsee step 5 above on how to view 
it.
