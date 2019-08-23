#include <stdio.h>
#include <random>
#include <stdlib.h>
#include <cstringt.h>
#include "CL/cl.h"
#include <iostream>
#include <math.h>
#include"../read_source.h"
#include <iostream>
#include <fstream>
using namespace std;

void printfSQMatrix(cl_float *nums, int n) 
{
	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < n; j++) 
		{
			printf("%13.8f", nums[i*n + j]);
		}
		printf("\n");
	}
}
float getRandom()
{
	static std::default_random_engine e;
	static std::uniform_real_distribution<> dis(-100, 100); // rage 0 - 100
	return roundf(dis(e)*100)/100;
}
int findMinIndex(double* nums, int n) 
{
	double smallest = nums[1];
	int index = 1;
	for (int i = 1; i <= n; i++) 
	{
		if(smallest>nums[i])
		{
			smallest = nums[i];
			index = i;
		}
	}
	return index;
}
void writeToCSV(cl_float * nums, int n, const char* name)
{
	ofstream outfile;
	outfile.open(name);
	for (int i = 0; i<n; i++)
	{
		outfile << nums[i*n+0];
		for (int j = 1; j<n; j++)
		{
			outfile << ", "<< nums[i*n+j];
		}
		outfile << endl;
	}
	outfile.close();
}
int main(int argc, char** argv)
{
	try {
		int nDO;
		printf("Please insert matrix size:\n");
		scanf("%d", &nDO);
		// creating pointer arrey for platformIDs 
		cl_platform_id * platforms = NULL;
		cl_platform_id * myPlatform = NULL;
		cl_device_id * myDevice = NULL;
		char * myDeviceName = NULL;
		char * myPlatformName = NULL;
		//creating memory for to obtain number of platforms 
		cl_uint num_platforms = 0;
		//Obtaining number of platforms
		cl_int errGetplatformsNum = clGetPlatformIDs(0, NULL, &num_platforms);
		//checking for errors of Obtaining number of platforms
		if (errGetplatformsNum != CL_SUCCESS)
		{
			printf("Obtaining number of platforms was unsuccessful.\n");
		}
		//Obtaining platform IDs
		platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id)*num_platforms);
		cl_int errGetplatforms = clGetPlatformIDs(num_platforms, platforms, 0);
		//checking for errors of Obtaining platform IDs
		if (errGetplatforms != CL_SUCCESS)
		{
			printf("Obtaining platform IDs was unsuccessful.\n");
		}

		//looping through the platforms of this machine.
		for (int i = 0; i < num_platforms; i++)
		{
			char * platformName = NULL;
			size_t nameSize = 0;
			//Obtaining platform Name Sizes
			cl_int errPlatformNameSize = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &nameSize);
			//checking for errors of Obtaining platform Name Sizes
			if (errPlatformNameSize != CL_SUCCESS)
			{
				printf("Obtaining platform name sizes was unsuccessful.\n");
			}
			//Obtaining platform Names
			platformName = (char *)malloc(sizeof(char)*nameSize);
			cl_int errPlatformName = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, nameSize, platformName, 0);
			//checking for errors of Obtaining platform Names
			if (errPlatformName != CL_SUCCESS)
			{
				printf("Obtaining platform names was unsuccessful.\n");
			}
			printf("Platform number %d:", i + 1);
			printf("%s\n", platformName);
			//Obtaining device ID numbers
			cl_device_id *devices = NULL;
			cl_uint numDevices = 0;
			cl_int errNumberOfDevices = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
			//checking for errors of Obtaining platform Names
			if (errNumberOfDevices != CL_SUCCESS)
			{
				if (errNumberOfDevices != CL_DEVICE_NOT_FOUND)
					printf("Obtaining device size was unsuccessful.\n");
			}
			//Obtaining device IDs
			if (numDevices > 0)
			{

				devices = (cl_device_id *)malloc(sizeof(cl_device_id)*numDevices);
				cl_int errDeviceIDs = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, numDevices, devices, 0);
				//checking for errors of Obtaining device IDs
				if (errDeviceIDs != CL_SUCCESS)
				{
					printf("Obtaining device IDs was unsuccessful.\n");
				}
				for (int ii = 0; ii < numDevices; ii++)
				{
					//Obtaining device names
					size_t deviceNameSize = 0;
					cl_int errDeviceNameSize = clGetDeviceInfo(devices[ii], CL_DEVICE_NAME, 0, NULL, &deviceNameSize);
					if (errDeviceNameSize != CL_SUCCESS)
					{
						printf("Obtaining device name size was unsuccessful.\n");
					}
					char * deviceName = (char *)malloc(sizeof(char)*deviceNameSize);
					cl_int errDeviceName = clGetDeviceInfo(devices[ii], CL_DEVICE_NAME, deviceNameSize, deviceName, NULL);
					if (errDeviceName != CL_SUCCESS)
					{
						printf("Obtaining device name was unsuccessful.\n");
					}

					printf("\t -Device number %d: ", ii + 1);
					printf("%s\n", deviceName);
					//if (strcmp(deviceName, "Intel(R) Iris(R) Plus Graphics 640") == 0 && strcmp(platformName, "Intel(R) OpenCL") == 0)
					//{
					myPlatform = &platforms[0];
					myDevice = &devices[0];
					myPlatformName = (char *)malloc(sizeof(char)*nameSize);
					myPlatformName = platformName;
					myDeviceName = (char *)malloc(sizeof(char)*deviceNameSize);
					myDeviceName = deviceName;
					//}

				}
			}

		}
		printf("\n\nSelected platform name is: ");
		printf("%s\n", myPlatformName);
		printf("Selected device name is: ");
		printf("%s\n", myDeviceName);
		
		//Creating context
		cl_int errCreateContext = 0;
		cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,(cl_context_properties)myPlatform[0],0 };
		cl_context myContext = clCreateContext(NULL,1, myDevice, NULL, NULL, &errCreateContext);
		if (errCreateContext != CL_SUCCESS)
			printf("Creating context was unsuccessful.\n");
		else
			printf("Creating context was successful.\n");
		//reading kernel file 
		size_t* fileSize = (size_t *)malloc(sizeof(size_t *) * 1);
		char* programSource = read_source("GaussJordanMain.cl", fileSize);
		cl_int errProgramWithSource = 0;
		//creating program
		cl_program myProgram = clCreateProgramWithSource(myContext, 1, (const char**)&programSource, NULL, &errProgramWithSource);
		if (errProgramWithSource != CL_SUCCESS)
		{
			if (errProgramWithSource == CL_INVALID_CONTEXT)
				printf("Invalid context.\n");
		}
		//Building program
		cl_int errBuildProgram = clBuildProgram(myProgram, 0, NULL, "-cl-std=CL2.0", NULL, NULL);
		if (errBuildProgram != CL_SUCCESS)
		{
			size_t len = 0;
			char buffer[2048];
			cl_int err = clGetProgramBuildInfo(myProgram, *myDevice, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			if (err == CL_SUCCESS)
				printf("%s\n", buffer);
			printf("Program was build unsuccessful.\n");
		}
		else
			printf("Program was built successfully.\n");
		//creating command queu
		cl_int errCommandQueCreation = 0;
		cl_command_queue_properties properties[] = { CL_QUEUE_PROPERTIES,  (cl_command_queue_properties)CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
			CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT,0 };
		cl_command_queue myCommandQue = clCreateCommandQueueWithProperties(myContext, myDevice[0],properties, &errCommandQueCreation);

		if (errCommandQueCreation != CL_SUCCESS)
			printf("Creating command queue with properties was unsuccessful.\n");
		else
			printf("Creating command queue with properties was successful.\n");
		//___________________________________________________________________________________________________________________________________________________________
		//creating kernel optimizing division row portion
		printf("\nOptimizing division row portion.\n");
		cl_int erCreateKernelDO = 0;
		cl_kernel mykernelDO = clCreateKernel(myProgram, "GaussJordanDiv", &erCreateKernelDO);

		if (erCreateKernelDO != CL_SUCCESS)
			printf("Creating kernel was unsuccessful.\n");
		else
			printf("Creating kernel was successful.\n");
		//creaeing buffer
		int yDO = 0;
		cl_float * matrixBuffDO = (cl_float*)_aligned_malloc(sizeof(cl_float *) * nDO* nDO, 4096);
		cl_float * IBuffDO = (cl_float*)_aligned_malloc(sizeof(cl_float *) * nDO* nDO, 4096);
		//intializing buffer
		for (int i = 0; i <= nDO; i++)
		{
			for (int j = 0; j < nDO; j++)
			{
				matrixBuffDO[i*nDO + j] = (cl_float)getRandom();
				if (i == j)
					IBuffDO[i*nDO + j] = (cl_float)getRandom();
				else
					IBuffDO[i*nDO + j] = (cl_float)getRandom();
			}
		}

		cl_int errClCreateBuffer1DO = 0;
		cl_mem inputMatrixDO = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float)*nDO*nDO, matrixBuffDO, &errClCreateBuffer1DO);
		cl_int errClCreateBuffer2DO = 0;
		cl_mem inputIDO = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float)*nDO*nDO, IBuffDO, &errClCreateBuffer2DO);
		//const cl_mem inputBuffer5 = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float16 *) * numOfinputs* numOfinputs, buffer5, &errClCreateBuffer5);
		if (errClCreateBuffer1DO != CL_SUCCESS || errClCreateBuffer2DO != CL_SUCCESS)
			printf("Creating buffers was unsuccessful.\n");
		else
			printf("Creating buffers was successful.\n");
	
		//setting kernel args
		cl_int errSetArg1DO = clSetKernelArg(mykernelDO, 0, sizeof(cl_mem), &inputMatrixDO);
		cl_int errSetArg2DO = clSetKernelArg(mykernelDO, 1, sizeof(cl_mem), &inputIDO);
		cl_int errSetArg3DO = clSetKernelArg(mykernelDO, 2, sizeof(int), &nDO);
		cl_int errSetArg4DO = clSetKernelArg(mykernelDO, 3, sizeof(int), &yDO);
		if (errSetArg1DO != CL_SUCCESS || errSetArg2DO != CL_SUCCESS || errSetArg3DO != CL_SUCCESS || errSetArg4DO != CL_SUCCESS)
			printf("Passing kernel args was unsuccessful.\n");
		else
			printf("Passing kernel args was successful.\n");
		
		cl_event eventDO;
		cl_ulong startTEventDO, endTEventDO;
		size_t retBytesEventDO;
		double *timesEventDO = new double[500+1];
		for (int i = 1; i <= 500; i++)
		{
			//running kernel
			cl_int errCommandQueCreationDO;
			cl_command_queue hostCommandQueDO = clCreateCommandQueue(myContext, myDevice[0], CL_QUEUE_PROFILING_ENABLE, &errCommandQueCreationDO);
			cl_int errSetArg4DO = clSetKernelArg(mykernelDO, 4, sizeof(int), &i);
			size_t global_work_size[] = { nDO/i+1,0,0 };
			cl_int errEnqueueNDRangeKernelDO = clEnqueueNDRangeKernel(hostCommandQueDO, mykernelDO, 1, NULL, global_work_size, NULL, 0, NULL, &eventDO);
			clFinish(hostCommandQueDO);
			clWaitForEvents(1, &eventDO);
			clGetEventProfilingInfo(eventDO, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTEventDO, &retBytesEventDO);
			clGetEventProfilingInfo(eventDO, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTEventDO, &retBytesEventDO);
			timesEventDO[i] = (double)((endTEventDO - startTEventDO) / 1000000.0);
			//printf("%Lf %d\n", timesEventDO[i], i);
		}
		int minIndexDO=findMinIndex(timesEventDO, 500);
		printf("Minimum division time %Lfms was achieved by dividing %d numbers in each instance of the kernel.\n", timesEventDO[minIndexDO], minIndexDO);
		//____________________________________________________________________________________________________________________________________________________________
		//creating kernel optimizing elimination row portion
		printf("\nOptimizing elimination matrix portion.\n");
		cl_int erCreateKernelEO = 0;
		cl_kernel mykernelEO = clCreateKernel(myProgram, "GaussJordanElim", &erCreateKernelEO);
		int nEO = nDO;
		int yEO = 0;
		if (erCreateKernelEO != CL_SUCCESS)
			printf("Creating kernel was unsuccessful.\n");
		else
			printf("Creating kernel was successful.\n");
		//setting kernel args
		cl_int errSetArg1EO = clSetKernelArg(mykernelEO, 0, sizeof(cl_mem), &inputMatrixDO);
		cl_int errSetArg2EO = clSetKernelArg(mykernelEO, 1, sizeof(cl_mem), &inputIDO);
		cl_int errSetArg3EO = clSetKernelArg(mykernelEO, 2, sizeof(int), &nEO);
		cl_int errSetArg4EO = clSetKernelArg(mykernelEO, 3, sizeof(int), &yEO);
		if (errSetArg1EO != CL_SUCCESS || errSetArg2EO != CL_SUCCESS || errSetArg3EO != CL_SUCCESS || errSetArg4EO != CL_SUCCESS)
			printf("Passing kernel args was unsuccessful.\n");
		else
			printf("Passing kernel args was successful.\n");

		cl_event eventEO;
		cl_ulong startTEventEO, endTEventEO;
		size_t retBytesEventEO;
		double *timesEventEO = new double[100+1];
		for (int i = 1; i <= 100; i++)
		{
			//running kernel
			cl_int errCommandQueCreationEO;
			cl_command_queue hostCommandQueEO = clCreateCommandQueue(myContext, myDevice[0], CL_QUEUE_PROFILING_ENABLE, &errCommandQueCreationEO);
			cl_int errSetArg5EO = clSetKernelArg(mykernelEO, 4, sizeof(int), &i);
			size_t global_work_size[] = { nEO / i + 1, nEO / i + 1,0 };
			cl_int errEnqueueNDRangeKernelDO = clEnqueueNDRangeKernel(hostCommandQueEO, mykernelEO, 2, NULL, global_work_size, NULL, 0, NULL, &eventEO);
			clFinish(hostCommandQueEO);
			clWaitForEvents(1, &eventEO);
			clGetEventProfilingInfo(eventEO, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTEventEO, &retBytesEventEO);
			clGetEventProfilingInfo(eventEO, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTEventEO, &retBytesEventEO);
			timesEventEO[i] = (double)((endTEventEO - startTEventEO) / 1000000.0);
			//printf("%Lf %d\n", timesEventDO[i], i);
		}
		int minIndexEO = findMinIndex(timesEventEO, 100);
		printf("Minimum elimination time %Lfms was achieved by eliminating %d by %d matrix in each instance of the kernel.\n", timesEventEO[minIndexEO], minIndexEO, minIndexEO);
		//____________________________________________________________________________________________________________________________________________________________
		//creating kernel optimizing elimination pivot column portion
		printf("\nOptimizing elimination pivot column portion.\n");
		cl_int erCreateKernelEPO = 0;
		cl_kernel mykernelEPO = clCreateKernel(myProgram, "GaussJordanElimPiv", &erCreateKernelEO);
		int nEPO = nDO;
		int yEPO = nEPO;
		if (erCreateKernelEPO != CL_SUCCESS)
			printf("Creating kernel was unsuccessful.\n");
		else
			printf("Creating kernel was successful.\n");
		//setting kernel args
		cl_int errSetArg1EPO = clSetKernelArg(mykernelEPO, 0, sizeof(cl_mem), &inputMatrixDO);
		cl_int errSetArg2EPO = clSetKernelArg(mykernelEPO, 1, sizeof(cl_mem), &inputIDO);
		cl_int errSetArg3EPO = clSetKernelArg(mykernelEPO, 2, sizeof(int), &nEPO);
		cl_int errSetArg4EPO = clSetKernelArg(mykernelEPO, 3, sizeof(int), &yEPO);
		if (errSetArg1EPO != CL_SUCCESS || errSetArg2EPO != CL_SUCCESS || errSetArg3EPO != CL_SUCCESS || errSetArg4EPO != CL_SUCCESS)
			printf("Passing kernel args was unsuccessful.\n");
		else
			printf("Passing kernel args was successful.\n");

		cl_event eventEPO;
		cl_ulong startTEventEPO, endTEventEPO;
		size_t retBytesEventEPO;
		double *timesEventEPO = new double[500 + 1];
		for (int i = 1; i <= 500; i++)
		{
			//running kernel
			cl_int errCommandQueCreationEPO;
			cl_command_queue hostCommandQueEPO = clCreateCommandQueue(myContext, myDevice[0], CL_QUEUE_PROFILING_ENABLE, &errCommandQueCreationEPO);
			cl_int errSetArg5EPO = clSetKernelArg(mykernelEPO, 4, sizeof(int), &i);
			size_t global_work_size[] = { nEPO / i + 1,0,0 };
			cl_int errEnqueueNDRangeKernelEPO = clEnqueueNDRangeKernel(hostCommandQueEPO, mykernelEPO, 1, NULL, global_work_size, NULL, 0, NULL, &eventEPO);
			clFinish(hostCommandQueEPO);
			clWaitForEvents(1, &eventEPO);
			clGetEventProfilingInfo(eventEPO, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTEventEPO, &retBytesEventEPO);
			clGetEventProfilingInfo(eventEPO, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTEventEPO, &retBytesEventEPO);
			timesEventEPO[i] = (double)((endTEventEPO - startTEventEPO) / 1000000.0);
			//printf("%Lf %d\n", timesEventDO[i], i);
		}
		int minIndexEPO = findMinIndex(timesEventEPO, 500);
		printf("Minimum pivot column elimination time %Lfms was achieved by eliminating %d numbers in each instance of the kernel.\n", timesEventEPO[minIndexEPO], minIndexEPO);
		//____________________________________________________________________________________________________________________________________________________________
		//creating kernel
		printf("\nInversion.\n");
		cl_int erCreateKernel = 0;
		cl_kernel mykernel = clCreateKernel(myProgram, "GaussJordanMain", &erCreateKernel);

		if (erCreateKernel != CL_SUCCESS)
			printf("Creating kernel was unsuccessful.\n");
		else
			printf("Creating kernel was successful.\n");
		//creaeing buffer
		int n = nDO;
		cl_float * matrixBuff = (cl_float*)_aligned_malloc(sizeof(cl_float *) * n* n,4096);
		cl_float * IBuff = (cl_float*)_aligned_malloc(sizeof(cl_float *) * n* n,4096);
		//intializing buffer

		/*cl_float matrixTemp[25] = { 
		2.0f,3.0f,4.0f,5.0f,11.0f,
		6.0f,7.0f,9.0f,12.0f,8.0f,
		10.0f,13.0f,14.0f,15.0f,16.0f,
		17.0f,18.0f,19.0f,20.0f,21.0f,
		22.0f,23.0f,24.0f,25.0f,26.0f };*/

		/*cl_float matrixTemp[100] = { 
		0.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f,  
		10.0f, 0.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 5.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 10.0f, 5.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 5.0f, 10.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 5.0f, 10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 5.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 5.0f, 10.0f,
		10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 5.0f };*/

		/*cl_float matrixTemp[100] = {
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, };*/

		// creating an identity matrix
		for (int i = 0; i <= n; i++)
		{
			for (int j = 0; j < n; j++) 
			{
				//matrixBuff[i*n+j] = (cl_float)matrixTemp[i*n + j];
				matrixBuff[i*n + j]= (cl_float)getRandom();
				if (i==j)
					IBuff[i*n+j] = (cl_float)1;
				else 
					IBuff[i*n + j] = (cl_float)0;
			}
		}
		//void * voidBuffer4 = buffer4;
		if (n <= 15) 
		{
			printf("Input matrix:\n");
			printfSQMatrix(matrixBuff, n);
			printf("Identity matrix:\n");
			printfSQMatrix(IBuff, n);
			writeToCSV(matrixBuff, n, "InputMatrix.csv");
		}
		else 
		{ 
			printf("Matrices are too large to printf on screen. See the CSV file in project folder.\n"); 
			writeToCSV(matrixBuff, n, "InputMatrix.csv");
		}
		cl_int errClCreateBuffer1 = 0;
		cl_mem inputMatrix = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float)*n*n, matrixBuff, &errClCreateBuffer1);
		cl_int errClCreateBuffer2 = 0;
		cl_mem inputI = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float)*n*n , IBuff, &errClCreateBuffer2);
		//const cl_mem inputBuffer5 = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float16 *) * numOfinputs* numOfinputs, buffer5, &errClCreateBuffer5);
		if (errClCreateBuffer1 != CL_SUCCESS || errClCreateBuffer2 != CL_SUCCESS)
			printf("Creating buffers was unsuccessful.\n");
		else
			printf("Creating buffers was successful.\n");
		cl_command_queue hostCommandQue = clCreateCommandQueue(myContext, myDevice[0], CL_QUEUE_PROFILING_ENABLE, &errCommandQueCreation);
		if (errCommandQueCreation != CL_SUCCESS)
			printf("Creating command queue with properties was unsuccessful.\n");
		else
			printf("Creating command queue with properties was successful.\n");
		//setting kernel args
		cl_int errSetArg1 = clSetKernelArg(mykernel, 0, sizeof(cl_mem), &inputMatrix);
		cl_int errSetArg2 = clSetKernelArg(mykernel, 1, sizeof(cl_mem), &inputI);
		cl_int errSetArg3 = clSetKernelArg(mykernel, 2, sizeof(int), &n);
		cl_int errSetArg4 = clSetKernelArg(mykernel, 3, sizeof(cl_command_queue), &myCommandQue);
		cl_int errSetArg5 = clSetKernelArg(mykernel, 4, sizeof(int), &minIndexDO);
		cl_int errSetArg6 = clSetKernelArg(mykernel, 5, sizeof(int), &minIndexEO);
		cl_int errSetArg7 = clSetKernelArg(mykernel, 6, sizeof(int), &minIndexEPO);
		if (errSetArg1 != CL_SUCCESS || errSetArg2 != CL_SUCCESS || errSetArg3 != CL_SUCCESS || errSetArg4 != CL_SUCCESS || errSetArg5 != CL_SUCCESS || errSetArg6 != CL_SUCCESS || errSetArg7 != CL_SUCCESS)
			printf("Passing kernel args was unsuccessful.\n");
		else
			printf("Passing kernel args was successful.\n");
		//running kernel
		size_t global_work_size[] = { 1,0,0 };
		size_t local_work_size[] = { 2, 2, 0 };
		cl_event eventINV;
		cl_ulong startTEventINV, endTEventINV;
		size_t retBytesEventINV;
		double timesEventINV;
		cl_int errEnqueueNDRangeKernel = clEnqueueNDRangeKernel(hostCommandQue, mykernel, 1, NULL, global_work_size, NULL, 0, NULL, &eventINV);
		clFinish(hostCommandQue);
		clWaitForEvents(1, &eventINV);
		clGetEventProfilingInfo(eventDO, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTEventINV, &retBytesEventINV);
		clGetEventProfilingInfo(eventDO, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTEventINV, &retBytesEventINV);
		timesEventINV = (double)((endTEventINV - startTEventINV) / 1000000.0);
		if (errEnqueueNDRangeKernel != CL_SUCCESS )
			printf("Kernel execution unsuccessful.\n");
		else
			printf("Kernel execution successful.\n");
			printf("Inversion kernel execution took %Lfms.\n", timesEventINV);
		//reading return value of the kernel
		cl_map_flags MapFlags(CL_MAP_READ);
		cl_int errClEnqueuMapBuffer = 0;
		cl_float *  ret = (cl_float *)clEnqueueMapBuffer(hostCommandQue, inputMatrix, CL_TRUE, MapFlags, 0, sizeof(cl_float*) * n*n, 0, NULL, NULL, &errClEnqueuMapBuffer);
		if (errClEnqueuMapBuffer != CL_SUCCESS)
			printf("Reading return buffer unsuccessful. \n");
		else
			printf("Reading return buffer successful. \n");
		printf("Outputting results matrix.\n");
		cl_float *  ret2 = (cl_float *)clEnqueueMapBuffer(hostCommandQue, inputI, CL_TRUE, MapFlags, 0, sizeof(cl_float*) * n*n, 0, NULL, NULL, &errClEnqueuMapBuffer);
		if (errClEnqueuMapBuffer != CL_SUCCESS)
			printf("Reading return buffer unsuccessful. \n");
		else
			printf("Reading return buffer successful. \n");

		if(n<=15)
		{
			printf("Inverted input matrix:\n");
			printfSQMatrix(ret, n);
			printf("Inverted identity matrix : \n");
			printfSQMatrix(ret2, n);
			writeToCSV(ret2, n, "InvertedMatrix.csv");
		}
		else
		{ 
			printf("Matrices are too large to printf on screen. See the CSV file in project folder.\n"); 
			writeToCSV(ret2, n, "InvertedMatrix.csv");
		}
		system("PAUSE");
		_aligned_free(matrixBuff);
		_aligned_free(IBuff);
		_aligned_free(matrixBuffDO);
		_aligned_free(IBuffDO);
		return 0;
	}

	catch (const std::exception& e)
	{
		printf(e.what());
		system("PAUSE");
		return 0;
	}
}