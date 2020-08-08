# EdgeDetector
Multithreaded edge detection program written in C. (serial version also included)

Takes in a png image and outputs a bitmap image of the edges detected from the input based on the threshold provided. <br />
Lower threshold means higher sensitivity. <br />

OpenMP and netpbm are dependancies. <br />

### How To Run 
To compile parralel version: gcc -o edgeDetection -fopenmp EdgeDetection.c  <br />
To run parallel version: ./edgeDetection inputFile outputFile (threshhold 0-255) (# of threads)<br />
Example: ./edgeDetection coins.png coinEdges.pbm 40 4 <br />

To compile serial version: gcc -o serialEdgeDetection SerialEdgeDetection.c  <br />
To run parallel version: ./serialEdgeDetection inputFile outputFile (threshhold 0-255) <br />
Example: ./serialEdgeDetection coins.png coinEdges.pbm 40 4 <br />

### Sample Input/Output
![bridge example](./Images/bridge-example.png?raw=true "Bridge Example")
(threshold 35)