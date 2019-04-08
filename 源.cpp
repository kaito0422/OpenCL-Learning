#include "basic.h"
#include <iomanip>
#include <FreeImage.h>
#pragma comment(lib,"FreeImage.lib") 

#pragma warning( disable : 4996 )

size_t RoundUp(int groupSize, int globalSize);
cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height);

int main()
{
	cl_platform_id *platform;
	platform = get_platform();

	cl_device_id *device;
	device = get_device(platform);

	cl_context context;
	context = create_context(platform, device);
	
	cl_command_queue queue;
	queue = create_command_queue(context, device);

	cl_program program;
	program = create_program("kernel.cl", context);

	cl_kernel kernel;
	kernel = create_kernel(program, device, "gauss_filter");

	/*************************************************************************/
	
	/* 创建输出对象 */

	cl_mem imageObjects[2] = { 0, 0 };
	int width, height;
	cl_int errNum;

	imageObjects[0] = LoadImage(context, "lily_collins.png", width, height);

	// Create ouput image object
	cl_image_format clImageFormat;
	clImageFormat.image_channel_order = CL_RGBA;
	clImageFormat.image_channel_data_type = CL_UNORM_INT8;
	imageObjects[1] = clCreateImage2D(context,
		CL_MEM_WRITE_ONLY,
		&clImageFormat,
		width,
		height,
		0,
		NULL,
		&errNum);

	cl_sampler sampler;

	/* 创建一个采样器，非规格化，把坐标钳制图像边缘，最近采样 */
	sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, NULL);

	errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &imageObjects[1]);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
	errNum |= clSetKernelArg(kernel, 3, sizeof(int), &width);
	errNum |= clSetKernelArg(kernel, 4, sizeof(int), &height);

	size_t localWorkSize[] = { 16, 16 };
	size_t globalWorkSize[] = { RoundUp(localWorkSize[0], width), RoundUp(localWorkSize[1], height) };

	errNum = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

	/*************************************************************************/

	system("pause");
	return 0;
}

/* 装载一张图片，并生成对应的图像的内存对象 */
cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height)
{
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName, 0);
	FIBITMAP* image = FreeImage_Load(format, fileName);

	// convert to 32-bit iamge
	FIBITMAP* temp = image;
	image = FreeImage_ConvertTo32Bits(image);
	FreeImage_Unload(temp);

	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	char *buffer = new char[width * height * 4];	// 4字节 32bits
	memcpy(buffer, FreeImage_GetBits(image), width * height * 4);

	FreeImage_Unload(image);

	// create opencl image
	cl_image_format clImageFormat;
	clImageFormat.image_channel_order = CL_RGBA;
	clImageFormat.image_channel_data_type = CL_UNORM_INT8;

	cl_int errNum;
	cl_mem clImage;
	clImage = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clImageFormat,
		width, height, 0, buffer, &errNum);
	
	return clImage;
}

//  Round up to the nearest multiple of the group size
size_t RoundUp(int groupSize, int globalSize)
{
	int r = globalSize % groupSize;
	if (r == 0)
	{
		return globalSize;
	}
	else
	{
		return globalSize + groupSize - r;
	}
}