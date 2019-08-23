# Matrix-Inversion
## Introduction 
Finite element method to calculate stress in isotropic materials under constant force is commonly used in machinery design. As a mechanical engineer gaining expertise in a company that designs and manufactures tunnel boring machines, TBMs, this project would be very useful, because this method is used to assess whether the structural design of sections of a TBM can withstand the forces that are applied to them. For example, in shielded tunnel boring machines, thickness and structural design of the shield is normally assessed with respect to hydrostatic water pressure at the ground level in which the machine is operating combined with the weight of the overburdening ground. Another example of this analysis for TBMs is for the front rotating part of the machine, cutterhead, which is in contact with the earth and takes some of the thrust force, which is the force that pushes the machine forward and is in the order of tens of thousands of kN depending on the size of the machine and ground conditions, as well as all of the cutterhead torque, which is in the order of tens of thousands of kNm also depending on the size of the machine and ground conditions. In addition to the major parts of the TBM under severe load most structures pulled behind the machine carrying equipment need to be evaluated to makes sure they can withstand the weight of equipment as well as the pulling force applied to them.\
In this report I am going to compare speed of parallelized algorithm of the most time-consuming part of finite element method using OpenCL with existing sequential based algorithm. The finite element method to calculate stress of isotropic materials is a heavily computational based process which mainly relies on matrix inversion. Running these computations could take several hours depending on the size of the object and how much accuracy is desired from the analysis. This method relies several steps; the first is creating a mesh of the object. Second step is to generate a stiffness matrix which is square matrix to size of the number of nodes on the mesh. Third step is inverting a matrix. Final step is final step is to multiply inverted matrix by the force column vector and a scalar value to obtain stress at every node. It is apparent that the main time-consuming portion of this computation is the matrix inversion. The focus of this report is to analyze the potential benefits of OpenCL using the GPU to parallelize matrix inversion of various sizes and comparing that with the conventional sequential method to invert matrices. 
## Implementation 
There are several methods to invert matrices but after research Gauss Jordan method was selected as the preferred method in this application because it is the most easily parallelizable method. Other methods have been developed to invert matrices with slightly smaller operational intensity but unfortunately, they are slower when parallelized. 
### Mathematical Representation 
This method requires two matrices to be appended the original matrix and the identity matrix after that the two steps can be done. For example, see the matrix below:
