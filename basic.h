/* 该头文件的接口都使用GPU */
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#include <CL/cl.h>
#include <CL/opencl.h>

cl_platform_id *get_platform();
cl_device_id *get_device(cl_platform_id *platform);
cl_context create_context(cl_platform_id * platform, cl_device_id * device);
cl_command_queue create_command_queue(cl_context context, cl_device_id *device);
cl_program create_program(char *srcFile, cl_context context);
cl_kernel create_kernel(cl_program program, cl_device_id * device, char *func_name);
void clean_up(cl_context context, cl_command_queue queue, cl_program program, cl_kernel kernel);