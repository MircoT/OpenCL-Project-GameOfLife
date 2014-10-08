#include <stdio.h>
#include <stdlib.h>
#include "timer.hpp"
#include <assert.h>
#include <stdio.h>
#include "game_of_life.hpp"

#ifdef __APPLE__
	#include <OpenCL/cl.h>
#else
	#include <CL/cl.h>
#endif

#define ROWS 4096
#define COLUMNS 4096
#define VECTOR_SIZE ROWS * COLUMNS


// Support function to print OpenCL Devices
void printDevices()
{
	int i, j;
	char* value;
	size_t valueSize;
	cl_uint platformCount;
	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_uint maxComputeUnits;
    
	// get all platforms
	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL);
    
	for (i = 0; i < platformCount; i++) {
        
		// get all devices
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
		devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);
        
		// for each device print critical attributes
		for (j = 0; j < deviceCount; j++) {
            
            
			// print device name
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
			printf("%d. Device: %s\n", j + 1, value);
			free(value);
            
			// print hardware device version
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
			printf(" %d.%d Hardware version: %s\n", j + 1, 1, value);
			free(value);
            
			// print software driver version
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
			printf(" %d.%d Software version: %s\n", j + 1, 2, value);
			free(value);
            
			// print c version supported by compiler for device
			clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
			printf(" %d.%d OpenCL C version: %s\n", j + 1, 3, value);
			free(value);
            
			// print parallel compute units
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                            sizeof(maxComputeUnits), &maxComputeUnits, NULL);
			printf(" %d.%d Parallel compute units: %d\n", j + 1, 4, maxComputeUnits);
            
			printf("\n-----\n");
            
		}
        
		free(devices);
        
	}
    
	free(platforms);
    
}

int main(void) {
    
	FILE* programHandle;
	size_t programSize, kernelSourceSize;
	char *programBuffer, *kernelSource;
    
	// get size of kernel source
	programHandle = fopen("kernel.cl", "r");
	if (programHandle == NULL)
	{
		printf("Can't open kernel file!\n");
		return 1;
	}
	fseek(programHandle, 0, SEEK_END);
	programSize = ftell(programHandle);
	rewind(programHandle);
    
	// read kernel source into buffer
	programBuffer = (char*) malloc(programSize + 1);
	programBuffer[programSize] = '\0';
	fread(programBuffer, sizeof(char), programSize, programHandle);
	fclose(programHandle);
    
    // Create game of life board and initialize it
	GameOfLife *test = new GameOfLife(COLUMNS, ROWS);
    
	test->setItem(1, 1, 1);
	test->setItem(2, 1, 1);
	test->setItem(3, 1, 1);
    
	test->setItem(14, 0, 1);
	test->setItem(15, 1, 1);
	test->setItem(13, 2, 1);
	test->setItem(14, 2, 1);
	test->setItem(15, 2, 1);
    
	//test->print();
    
	printDevices();
    
	// Get platform and device information
	cl_platform_id * platforms = NULL;
	cl_uint num_platforms;
    
	//Set up the Platform
	cl_int clStatus = clGetPlatformIDs(0, NULL, &num_platforms);
    assert(clStatus == 0);
	platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id)*num_platforms);
	clStatus = clGetPlatformIDs(num_platforms, platforms, NULL);
    assert(clStatus == 0);
    
	//Get the devices list and choose the device you want to run on
	cl_device_id *device_list = NULL;
	cl_uint num_devices;
	clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
	device_list = (cl_device_id *) malloc(sizeof(cl_device_id)*num_devices);
	clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices, device_list, NULL);
    assert(clStatus == 0);
    
	// Create one OpenCL context for each device in the platform
	cl_context context;
	context = clCreateContext(NULL, num_devices, device_list, NULL, NULL, &clStatus);
	assert(clStatus == 0);
    
	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue(context, device_list[0], 0, &clStatus);
	assert(clStatus == 0);
    
	// Create memory buffers on the device for matrix and result
	cl_mem matrix = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(int), NULL, &clStatus);
	assert(clStatus == 0);
    
	cl_mem result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(int), NULL, &clStatus);
	assert(clStatus == 0);
    
	// Copy the Buffer matrix to the device
	clStatus = clEnqueueWriteBuffer(command_queue, matrix, CL_TRUE, 0, VECTOR_SIZE * sizeof(int), test->getRow(0), 0, NULL, NULL);
	assert(clStatus == 0);
    
	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &programBuffer, &programSize, &clStatus);
	assert(clStatus == 0);
    
	// read kernel source back in from program to check
	clGetProgramInfo(program, CL_PROGRAM_SOURCE, 0, NULL, &kernelSourceSize);
	kernelSource = (char*) malloc(kernelSourceSize);
	clGetProgramInfo(program, CL_PROGRAM_SOURCE, kernelSourceSize, kernelSource, NULL);
    // Print the kernel source
	//printf("\nKernel source:\n\n%s\n", kernelSource);
	free(kernelSource);
    
	// Build the program
	clStatus = clBuildProgram(program, 1, device_list, NULL, NULL, NULL);
    
	//assert(clStatus==0);
	if (clStatus != CL_SUCCESS) {
		size_t logSize;
		cl_build_status status;
		char *programLog;
        
		// check build error and build status first
		clGetProgramBuildInfo(program, device_list[0], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
        
		// check build log
		clGetProgramBuildInfo(program, device_list[0],
                              CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		programLog = (char*) calloc(logSize + 1, sizeof(char));
		clGetProgramBuildInfo(program, device_list[0], CL_PROGRAM_BUILD_LOG, logSize + 1, programLog, NULL);
		printf("Build failed; error=%d, status=%d, programLog:\n\n%s\n", clStatus, status, programLog);
		free(programLog);
		return 1;
	}
    
	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "gameOfLifeKernel", &clStatus);
    assert(clStatus == 0);
    
    int rows = ROWS;
    int columns = COLUMNS;

	// Set the arguments of the kernel
    clStatus = clSetKernelArg(kernel, 0, sizeof(int *), (void *) &rows);
	assert(clStatus == 0);
    clStatus = clSetKernelArg(kernel, 1, sizeof(int *), (void *) &columns);
	assert(clStatus == 0);
	clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &matrix);
	assert(clStatus == 0);
	clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *) &result);
	assert(clStatus == 0);
    
	// Execute the OpenCL kernel on the list
	size_t global_size = VECTOR_SIZE; // Process the entire lists
	size_t local_size = 1; // Process one item at a time
    
    // ----- NORMAL STEP -----
    
    Timer::bit64 start, end;
    
	start = Timer::getTime();
	test->step();
	end = Timer::getTime();
    
	//test->print();
	std::cout << "Time elapsed normal step: " << end - start << std::endl;
    
	// -----------------------
    
    
	start = Timer::getTime();  // START -----
	clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	assert(clStatus == 0);
    
	// Read the cl memory result on device to the host variable result
	clStatus = clEnqueueReadBuffer(command_queue, result, CL_TRUE, 0, VECTOR_SIZE * sizeof(int), test->getRow(0), 0, NULL, NULL);
	assert(clStatus == 0);
    
	// Clean up and wait for all the comands to complete.
	clStatus = clFlush(command_queue);
	assert(clStatus == 0);
	clStatus = clFinish(command_queue); // to make sure the kernel completed
	assert(clStatus == 0);
    
	end = Timer::getTime();  // END -----
    
	//test->print();
	std::cout << "Time elapsed with OpenCL: " << end - start << std::endl;
    
	// Finally release all OpenCL allocated objects and host buffers
	clStatus = clReleaseKernel(kernel);
	assert(clStatus == 0);
	clStatus = clReleaseProgram(program);
	assert(clStatus == 0);
	clStatus = clReleaseMemObject(matrix);
	assert(clStatus == 0);
	clStatus = clReleaseMemObject(result);
	assert(clStatus == 0);
	clStatus = clReleaseCommandQueue(command_queue);
	assert(clStatus == 0);
	clStatus = clReleaseContext(context);
	assert(clStatus == 0);
	free(platforms);
	free(device_list);
	delete test;
    

	std::cout << "Press Enter to continue...";
    std::cin.get();
	return 0;
    
}