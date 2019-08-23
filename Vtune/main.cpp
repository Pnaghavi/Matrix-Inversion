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

void printfSQMatrix(cl_float *nums, int n, bool par)
{
	if (par)
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
	else 
	{
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				printf("%13.8f", nums[i * 2 * n + j + n]);
			}
			printf("\n");
		}
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
void writeToCSV(cl_float * nums, int n, const char* name, bool par)
{
	ofstream outfile;
	outfile.open(name);
	if(par)
		for (int i = 0; i<n; i++)
		{
			outfile << nums[i*n+0];
			for (int j = 1; j<n; j++)
			{
				outfile << ", "<< nums[i*n+j];
			}
			outfile << endl;
		}
	else
		for (int i = 0; i<n; i++)
		{
			outfile << nums[i * 2 * n + n + 0];
			for (int j = 1; j<n; j++)
			{
				outfile << ", " << nums[i * 2 * n + j + n];
			}
			outfile << endl;
		}
	outfile.close();
}
void sequentialMatrixInversion(float * a, int n) 
{
	long i, j, k;
	float d;
	for (i = n-1; i>0; i--)
	{
		if (a[(i - 1)*2*n]<a[i*2*n])
			for (j = 0; j < n * 2; j++)
			{
				d = a[i*2*n+j];
				a[i*2*n+j] = a[(i - 1)*2*n+j];
				a[(i - 1)*2*n+j] = d;
			}
	}
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n * 2; j++)
			if (j != i)
			{
				d = a[j*2*n+i] / a[i*2*n+i];
				for (k = 0; k <= n * 2; k++)
					a[j*2*n+k] -= a[i*2*n+k] * d;
			}
	}

	for (i = 0; i < n; i++)
	{
		d = a[i*2*n+i];
		for (j = 0; j < n * 2; j++)
			a[i*2*n+j] = a[i*2*n+j] / d;
	}
	return;
}

int main(int argc, char** argv)
{
	try {
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
			//printf("Platform number %d:", i + 1);
			//printf("%s\n", platformName);
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

					//printf("\t -Device number %d: ", ii + 1);
					//printf("%s\n", deviceName);
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
		//printf("\n\nSelected platform name is: ");
		//printf("%s\n", myPlatformName);
		//printf("Selected device name is: ");
		//printf("%s\n", myDeviceName);
		
		//Creating context
		cl_int errCreateContext = 0;
		cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM,(cl_context_properties)myPlatform[0],0 };
		cl_context myContext = clCreateContext(NULL,1, myDevice, NULL, NULL, &errCreateContext);
		if (errCreateContext != CL_SUCCESS)
			printf("Creating context was unsuccessful.\n");
		//else
		//	printf("Creating context was successful.\n");
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
		//else
			//printf("Program was built successfully.\n");
		//creating command queu
		cl_int errCommandQueCreation = 0;
		cl_command_queue_properties properties[] = { CL_QUEUE_PROPERTIES,  (cl_command_queue_properties)CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
			CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT,0 };
		cl_command_queue myCommandQue = clCreateCommandQueueWithProperties(myContext, myDevice[0],properties, &errCommandQueCreation);

		if (errCommandQueCreation != CL_SUCCESS)
			printf("Creating command queue with properties was unsuccessful.\n");
		//else
		//	printf("Creating command queue with properties was successful.\n");
		//____________________________________________________________________________________________________________________________________________________________
		//creating kernel
		//printf("\nInversion.\n");
		//creaeing buffer
		int n = 1000;
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
				if (i == j)
				{
					IBuff[i*n + j] = (cl_float)1;
				}
				else 
				{
					IBuff[i*n + j] = (cl_float)0;
				}
			}
		}
		//void * voidBuffer4 = buffer4;
	/*	if (n <= 15) 
		{
			printf("Input matrix:\n");
			printfSQMatrix(matrixBuff, n,true);
			printf("Identity matrix:\n");
			printfSQMatrix(IBuff, n,true);
			writeToCSV(matrixBuff, n, "InputMatrix.csv",true);
		}
		else 
		{ 
			printf("Matrices are too large to printf on screen. See the CSV file in project folder.\n");
			if(n<5000)
				writeToCSV(matrixBuff, n, "InputMatrix.csv", true);
		}*/
		cl_int errClCreateBuffer1 = 0;
		cl_mem inputMatrix = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float)*n*n, matrixBuff, &errClCreateBuffer1);
		cl_int errClCreateBuffer2 = 0;
		cl_mem inputI = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float)*n*n , IBuff, &errClCreateBuffer2);
		//const cl_mem inputBuffer5 = clCreateBuffer(myContext, CL_MEM_USE_HOST_PTR, sizeof(cl_float16 *) * numOfinputs* numOfinputs, buffer5, &errClCreateBuffer5);
		if (errClCreateBuffer1 != CL_SUCCESS || errClCreateBuffer2 != CL_SUCCESS)
			printf("Creating buffers was unsuccessful.\n");
		//else
		//	printf("Creating buffers was successful.\n");


		//______________________Div____________________________________________________________
		cl_int erCreateKernelDiv = 0;
		cl_kernel kernelDiv = clCreateKernel(myProgram, "GaussJordanDiv", &erCreateKernelDiv);

		if (erCreateKernelDiv != CL_SUCCESS)
			printf("Creating div kernel was unsuccessful.\n");
		//else
		//	printf("Creating div kernel was successful.\n");
		//setting kernel args
		cl_int errSetArg1Div = clSetKernelArg(kernelDiv, 0, sizeof(cl_mem), &inputMatrix);
		cl_int errSetArg2Div = clSetKernelArg(kernelDiv, 1, sizeof(cl_mem), &inputI);
		cl_int errSetArg3Div = clSetKernelArg(kernelDiv, 2, sizeof(int), &n);
		if (errSetArg1Div != CL_SUCCESS || errSetArg2Div != CL_SUCCESS || errSetArg3Div != CL_SUCCESS)
			printf("Passing div kernel args was unsuccessful.\n");
		//else
		//	printf("Passing div kernel args was successful.\n");
		//______________________DiagDiv____________________________________________________________
		cl_int erCreateKernelDiagDiv = 0;
		cl_kernel kernelDiagDiv = clCreateKernel(myProgram, "GaussJordanDiagDiv", &erCreateKernelDiagDiv);

		if (erCreateKernelDiagDiv != CL_SUCCESS)
			printf("Creating diagdiv kernel was unsuccessful.\n");
		//else
		//	printf("Creating diagdiv kernel was successful.\n");
		//setting kernel args
		cl_int errSetArg1DiagDiv = clSetKernelArg(kernelDiagDiv, 0, sizeof(cl_mem), &inputMatrix);
		cl_int errSetArg2DiagDiv = clSetKernelArg(kernelDiagDiv, 1, sizeof(cl_mem), &inputI);
		cl_int errSetArg3DiagDiv = clSetKernelArg(kernelDiagDiv, 2, sizeof(int), &n);
		if (errSetArg1DiagDiv != CL_SUCCESS || errSetArg2DiagDiv != CL_SUCCESS || errSetArg3DiagDiv != CL_SUCCESS)
			printf("Passing diagdiv kernel args was unsuccessful.\n");
		//else
		//	printf("Passing duagdiv kernel args was successful.\n");
		//______________________elim____________________________________________________________
		cl_int erCreateKernelElim = 0;
		cl_kernel kernelElim = clCreateKernel(myProgram, "GaussJordanElim", &erCreateKernelElim);

		if (erCreateKernelElim != CL_SUCCESS)
			printf("Creating diagdiv kernel was unsuccessful.\n");
		//else
		//	printf("Creating diagdiv kernel was successful.\n");
		//setting kernel args
		cl_int errSetArg1Elim = clSetKernelArg(kernelElim, 0, sizeof(cl_mem), &inputMatrix);
		cl_int errSetArg2Elim = clSetKernelArg(kernelElim, 1, sizeof(cl_mem), &inputI);
		cl_int errSetArg3Elim = clSetKernelArg(kernelElim, 2, sizeof(int), &n);
		//cl_int errSetArg5 = clSetKernelArg(kernelDiv, 4, sizeof(int), &minIndexDO);
		if (errSetArg1Elim != CL_SUCCESS || errSetArg2Elim != CL_SUCCESS || errSetArg3Elim != CL_SUCCESS)
			printf("Passing elim kernel args was unsuccessful.\n");
		//else
		//	printf("Passing elim kernel args was successful.\n");
		//______________________elimPivot____________________________________________________________
		cl_int erCreateKernelElimP = 0;
		cl_kernel kernelElimP = clCreateKernel(myProgram, "GaussJordanElimPiv", &erCreateKernelElimP);

		if (erCreateKernelElimP != CL_SUCCESS)
			printf("Creating diagdiv kernel was unsuccessful.\n");
		//else
		//	printf("Creating diagdiv kernel was successful.\n");
		//setting kernel args
		cl_int errSetArg1ElimP = clSetKernelArg(kernelElimP, 0, sizeof(cl_mem), &inputMatrix);
		cl_int errSetArg2ElimP = clSetKernelArg(kernelElimP, 1, sizeof(cl_mem), &inputI);
		cl_int errSetArg3ElimP = clSetKernelArg(kernelElimP, 2, sizeof(int), &n);
		//cl_int errSetArg5 = clSetKernelArg(kernelDiv, 4, sizeof(int), &minIndexDO);
		if (errSetArg1ElimP != CL_SUCCESS || errSetArg2ElimP != CL_SUCCESS || errSetArg3ElimP != CL_SUCCESS)
			printf("Passing elim pivot kernel args was unsuccessful.\n");
		//else
		//	printf("Passing elim pivot kernel args was successful.\n");
		//running kernels
		//_______________________________________________________________________________________________
		cl_map_flags MapFlags(CL_MAP_READ);
		cl_int erCommandQue = 0;
		cl_command_queue commandQue = clCreateCommandQueue(myContext, myDevice[0], CL_QUEUE_PROFILING_ENABLE, &erCommandQue);
		/*LARGE_INTEGER liiiL;
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		double PCFreq = 0.0;
		PCFreq = double(li.QuadPart) / 1000.0;
		__int64 CounterStart = 0;
		QueryPerformanceFrequency(&li);
		PCFreq = double(li.QuadPart) / 1000.0;
		QueryPerformanceCounter(&liiiL);
		CounterStart = liiiL.QuadPart;*/
		for(int i=0;i<n;i++)
		{
			//div_____________________________________________________________________________
			int minIndexDO = 4;
			size_t global_work_size_div[] = { n/minIndexDO+1,0,0 };
			size_t local_work_size_div[] = { 4,0,0 };
			clSetKernelArg(kernelDiv, 3, sizeof(int), &i);
			clSetKernelArg(kernelDiv, 4, sizeof(int), &minIndexDO);
			clEnqueueNDRangeKernel(commandQue, kernelDiv, 1, NULL, global_work_size_div, local_work_size_div, 0, NULL, NULL);
			clFinish(commandQue);
			//diagDiv_________________________________________________________________________
			size_t global_work_size_diagdiv[] = { 1,0,0 };
			clSetKernelArg(kernelDiagDiv, 3, sizeof(int), &i);
			clEnqueueNDRangeKernel(commandQue, kernelDiagDiv, 1, NULL, global_work_size_diagdiv, NULL, 0, NULL, NULL);
			clFinish(commandQue);
			//eLim_____________________________________________________________________________
			int minIndexEO = 1;
			size_t global_work_size_elim[] = { n / minIndexEO+1, n / minIndexEO+1 ,0 };
			size_t local_work_size_elim[] = { 4,8,0 };
			clSetKernelArg(kernelElim, 3, sizeof(int), &i);
			clSetKernelArg(kernelElim, 4, sizeof(int), &minIndexEO);
			cl_int erKernelElim=clEnqueueNDRangeKernel(commandQue, kernelElim, 2, NULL, global_work_size_elim, local_work_size_elim, 0, NULL, NULL);
			clFinish(commandQue);
			//eLimP_____________________________________________________________________________
			int minIndexEPO = 6;
			size_t global_work_size_elim_p[] = { n / minIndexEPO + 1,0,0 };
			size_t local_work_size_ElimP[] = { 4,0,0 };
			clSetKernelArg(kernelElimP, 3, sizeof(int), &i);
			clSetKernelArg(kernelElimP, 4, sizeof(int), &minIndexEPO);
			clEnqueueNDRangeKernel(commandQue, kernelElimP, 1, NULL, global_work_size_elim_p, local_work_size_ElimP, 0, NULL, NULL);
			clFinish(commandQue);
		}
		if(n<=15)
		{
			//reading return value of the kernel
			cl_map_flags MapFlags(CL_MAP_READ);
			cl_int errClEnqueuMapBuffer = 0;
			cl_float *  ret = (cl_float *)clEnqueueMapBuffer(commandQue, inputMatrix, CL_TRUE, MapFlags, 0, sizeof(cl_float*) * n*n, 0, NULL, NULL, &errClEnqueuMapBuffer);
			cl_float *  ret2 = (cl_float *)clEnqueueMapBuffer(commandQue, inputI, CL_TRUE, MapFlags, 0, sizeof(cl_float*) * n*n, 0, NULL, NULL, &errClEnqueuMapBuffer);
			//printf("Inverted input matrix:\n");
			//printfSQMatrix(ret, n,true);
			//printf("Inverted identity matrix : \n");
			//printfSQMatrix(ret2, n,true);
			//writeToCSV(ret2, n, "InvertedMatrix.csv",true);
		}
		else
		{ 
			//cl_map_flags MapFlags(CL_MAP_READ);
			//cl_int errClEnqueuMapBuffer = 0;
			//cl_float *  ret2 = (cl_float *)clEnqueueMapBuffer(commandQue, inputI, CL_TRUE, MapFlags, 0, sizeof(cl_float*) * n*n, 0, NULL, NULL, &errClEnqueuMapBuffer);
			//printf("Matrices are too large to printf on screen. See the CSV file in project folder.\n"); 
			//if(n<5000)
			//	writeToCSV(ret2, n, "InvertedMatrix.csv",true);
		}

		//system("PAUSE");
		_aligned_free(matrixBuff);
		_aligned_free(IBuff);
		return 0;
	}

	catch (const std::exception& e)
	{
		printf(e.what());
		system("PAUSE");
		return 0;
	}
}