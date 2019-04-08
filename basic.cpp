#include "basic.h"

cl_platform_id *get_platform()
{
	cl_int errNum;
	cl_uint platform_num;
	cl_uint device_num;
	cl_platform_id *platform;
	cl_device_id *deviceIDs;

	/* get the platform ID */
	errNum = clGetPlatformIDs(0, NULL, &platform_num);
	platform = (cl_platform_id *)malloc(sizeof(cl_platform_id)*platform_num);
	errNum = clGetPlatformIDs(platform_num, platform, NULL);
	if (errNum != CL_SUCCESS)
	{
		cout << "get platform failed" << endl;
		return NULL;
	}

	return platform;
}

cl_device_id *get_device(cl_platform_id *platform)
{
	cl_int errNum;
	cl_uint device_num;
	cl_device_id *deviceIDs;

	errNum = clGetDeviceIDs(platform[1], CL_DEVICE_TYPE_GPU, 0, NULL, &device_num);
	deviceIDs = (cl_device_id *)malloc(sizeof(cl_device_id)*device_num);
	errNum = clGetDeviceIDs(platform[1], CL_DEVICE_TYPE_GPU, 1, deviceIDs, NULL);
	if (errNum != CL_SUCCESS)
	{
		cout << "get device failed" << endl;
		return NULL;
	}

	return deviceIDs;
}

cl_context create_context(cl_platform_id * platform, cl_device_id * device)
{
	cl_context context;
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform[1],
		0
	};
	context = clCreateContext(properties, 1, device, NULL, NULL, NULL);
	if (context == NULL)
		cout << "create context failed" << endl;

	return context;
}

cl_command_queue create_command_queue(cl_context context, cl_device_id *device)
{
	cl_command_queue queue;
	queue = clCreateCommandQueue(context, device[0], 0, NULL);
	if (queue == NULL)
		cout << "create queue failed" << endl;

	return queue;
}

cl_program create_program(char *srcFile, cl_context context)
{
	ifstream kernelFile(srcFile, ios::in);
	ostringstream oss;
	oss << kernelFile.rdbuf();
	string srcStdStr = oss.str();
	const char * srcStr = srcStdStr.c_str();

	cl_program program;
	program = clCreateProgramWithSource(context, 1, &srcStr, NULL, NULL);
	if (program == NULL)
		cout << "create program failed" << endl;

	return program;
}

cl_kernel create_kernel(cl_program program, cl_device_id * device, char *func_name)
{
	cl_kernel kernel;
	clBuildProgram(program, 1, device, NULL, NULL, NULL);

	kernel = clCreateKernel(program, func_name, NULL);
	if (kernel == NULL)
		cout << "create kernel failed" << endl;

	return kernel;
}

void clean_up(cl_context context, cl_command_queue queue, cl_program program, 
	cl_kernel kernel)
{
	if (context != 0)
		clReleaseContext(context);
	if (queue != 0)
		clReleaseCommandQueue(queue);
	if (program != 0)
		clReleaseProgram(program);
	if (kernel != 0)
		clReleaseKernel(kernel);
}