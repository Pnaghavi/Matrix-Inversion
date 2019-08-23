# Matrix-Inversion
## Introduction 
In this report I am going to compare speed of parallelized algorithm of matrix inversion using OpenCL with existing sequential based algorithm. Running these computations could take several hours depending on the size of the matrix and how much accuracy is desired from the analysis. The focus of this report is to analyze the potential benefits of OpenCL using the GPU to parallelize matrix inversion of various sizes and comparing that with the conventional sequential method to invert matrices. 
## Implementation 
There are several methods to invert matrices but after research Gauss Jordan method was selected as the preferred method in this application because it is the most easily parallelizable method. Other methods have been developed to invert matrices with slightly smaller operational intensity but unfortunately, they are slower when parallelized. 
### Mathematical Representation 
This method requires two matrices to be appended, the original matrix, and the identity matrix after that the two steps can be done. For example, see the matrix below:
![matrix1](/images/matrix1.PNG)
Every row operation has to be done on the working matrix. Gauss Jordan matrix inversion algorithm requires two main steps per row of the matrix. The first step requires matrix row elements to be divided by the diagonal element of the matrix. At the end the diagonal element is one. When step one is completed the working matrix will be:
![matrix2](/images/matrix2.PNG)
In two steps must be conducted row by row sequentially but this step for each column happens on all the elements of the matrix except for the row number that is equal to the current row or the pivot row. In this step every item is subtracted by the same element on the pivot row multiplied by the pivot element on the item’s row. The result will be after the first pivot column. 
![matrix3](/images/matrix3.PNG)
Same two steps is repeated for every row then the result will be identity matrix on the right-hand side and inverted matrix on the left-hand side.
![matrix4](/images/matrix4.PNG)

### Theoretical Analysis  
In this section the arithmetic intensity of matrix inversion is analysed to assess the floating-point performance as a function of machine peak performance.  
![complexity](/images/complexity.PNG)

### Roofline Performance Model
Roofline model below is showing floating-point performance as a function of machine peak performance, machine peak bandwidth, and arithmetic intensity of matrix inversion. These curves demonstrate performance bound under which matrix inversion kernel is possible. The graph below suggests that this kernel is bound by the memory bandwidth of the GPU rather than the processors peak performance. Therefor to improve this algorithm the number of memory reads, and writes have to decrease.  This graph was constructed based on 	Intel(R) Iris(R) Plus Graphics 640 parameters.    
![roofline](/images/roofline.PNG)

### Empirical Analysis  
#### Optimizing NDrange Configuration and Performance profiling      
In this section the focus was mainly on finding out the most efficient global and local sizes. Since there are four kernels called for each row of the matrix, they each have to be optimized, but the divide diagonal kernel is a single item kernel and there is no possible way to improv it. However, the divide and elimination pivot kernels are single dimension kernel and its global size and local size can be optimized. In addition, the elimination kernel is a two-dimensional kernel n3 flops so optimizing this matrix is critical. Initially I began by optimizing the single dimension kernels global size by dividing my global dimension and looping inside the kernel to do would have been multiple instances of the kernel inside one kernel while timing it, using device side timing, to find the fastest arrangement. After that used the same method but in two dimensions for the elimination kernel eliminating a square matrix in each kernel while also using device side timing to find the best arrangement. Finally, I used KCL and most common arrangement outputted from my optimization to find out the most optimal local sizes. The most optimal local sizes are:
Division kernel 8,0,0
Elimination kernel: 8,4,0
Pivot Elimination kernel: 2,0,0
Most optimal global sizes for various matrix sizes are: 
![kernelSize](/images/kernelSize.PNG)

#### Serial and Parallel Execution Time comparison    
In this section the main focus of the project is realized by comparing the time measured to complete matrix inversion of various matrix sizes of 100, 1000, 2000, 5000, and 10000 both in serial and parallel and speedup of each matrix size. The overall parallel and serial time was done using windows performance counters as there are several kernels that has to run for one matrix inversion.  
![speedup](/images/speedup.PNG)
![speedUpGraph](/images/speedUpGraph.PNG)
#### Vtune Analysis of performance
For Vtune analysis was only done for matrices of size 1000 given the most common global sizes and local sizes. The Vtune analysis consists of both overview report and compute basic report.       
![EU-Utilization](/images/EU-Utilization.PNG)

FPU utilization 5.4% elapsed time from the overview report. 
FPU utilization 5.7% elapsed time from the compute basics report. 
In terms of memory usage, no local memory, typed, and untyped memory was used was used by the kernels. GPU memory bandwidth is listed below. 
![memory Bandwidth](/images/memory-Bandwidth.PNG)

Looking back at the roofline graph this algorithm theoretically should be memory bandwidth limited but it seems like the bottleneck of this algorithm is due to fact that each kernel has to run sequentially to provide correctly inverted matrix while the instances of the kernels can run in parallel. As it can be seen in the table above kernels are often stalled or idle except for the elimination kernel. In addition, the kernels only operate for about a second and a half, but the entire program takes about 6 seconds to run to select devices read and build kernels and create buffers. Therefore, it is apparent that this size matrix is too small for the benefits of parallelization however after 3000 or larger matrix size this algorithm becomes very useful.
## Additional Findings
During the implementation initially, the code was implemented using device side queueing. Where the host would call a main kernel and the main kernel was in charge of creating all the division and elimination kernels, as well as creating the required sequential calling of the kernels to maintain the correct matrix row operations. This implementation was successful providing fast and correct results until the matrix size exceeded 59, at which point all work would halt. This was rather unpredictable the kernel would run without errors and there was no warning in the Kronos documentations about this. After further research I discovered the device side queueing has had this issue when the number of child kernels passes a limit. It must be emphasized that with matrix of size 59 the code calls 236 child kernels and this number seems to be a rather significant number of child kernels which seems to be the main cause of this issue.                           
## Conclusion 
It is apparent that parallel programming using OpenCL and GPU results in significant improvements over sequential programming, nevertheless it must be emphasized that there is significant overhead involved with parallel programming which is a minor issue when the matrix is large enough.
