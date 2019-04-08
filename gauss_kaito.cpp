#include "basic.h"
#include <iomanip>
#include "FreeImage.h"
using namespace std;

#pragma comment(lib,"FreeImage.lib") 
#pragma warning( disable : 4996 )

cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height);
size_t RoundUp(int groupSize, int globalSize);
bool SaveImage(char *fileName, char *buffer, int width, int height);

int main()
{
	cl_int errNum;
	cl_platform_id *platform;
	cl_device_id *device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;

	platform = get_platform();
	device = get_device(platform);
	context = create_context(platform, device);
	queue = create_command_queue(context, device);
	program = create_program("kernel.cl", context);
	kernel = create_kernel(program, device, "gauss_filter");

	/* 与图片相关 */
	cl_mem my_image;	// 用于保存输入的图像
	int width, height;	
	cl_mem image_out;	// 用于保存输出的图像

	// create output image object
	cl_image_format clImageFormat;
	clImageFormat.image_channel_order = CL_RGBA;	// RGBA中的RGB即三原色，A表示Alpha，为透明度
	clImageFormat.image_channel_data_type = CL_UNORM_INT8;	// 把图像通道数据8位整数映射到[0.0, 1.0], Unsigned NORMalize
	/* 该处通道就是RGBA4个通道，一个像素点用8位表示 */			/* 所以在kernel文件中的颜色范围也是在[0.0，1.0] */

	my_image = LoadImage(context, "lin.jpg", width, height);	// 加载一副图片的内容到内存对象
	image_out = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &clImageFormat, width, height, 0, NULL, NULL);

	cl_sampler sampler;
	/* 创建采样器 */
	sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, NULL);
	                                  /* 非规格化  坐标超出部分钳制到边缘    最近采样 */

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &my_image);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &image_out);
	clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
	clSetKernelArg(kernel, 3, sizeof(int), &width);
	clSetKernelArg(kernel, 4, sizeof(int), &height);

	size_t local_size[] = { 16, 16 };
	size_t global_size[] = { RoundUp(local_size[0], width), RoundUp(local_size[1], height) };
							/* 如果global大小不是local大小的整数倍，则多取一组local的大小 */

	/* kernel排队 */
	errNum = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);

	/* 定义保存输出的buffer */ 
	char *buffer = new char[width * height * 4];
	size_t origin[3] = { 0,0,0 };	// 从[0, 0]坐标开始读取，第3位应该是表示深度，从0到1
	size_t region[3] = { width, height, 1 };	// 读取width宽度，height高度
	clEnqueueReadImage(queue, image_out, CL_TRUE, origin, region, 0, 0, buffer, 0, NULL, NULL);

	SaveImage("kaito.png", buffer, width, height);	/* 将buffer里的数据用FreeImage保存成图片格式 */

	clean_up(context, queue, program, kernel);

	system("pause");
	return 0;
}

cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height)
{
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName, 0);	// 获取图片文件类型
	FIBITMAP *image = FreeImage_Load(format, fileName);	// 加载一张图片，格式为format格式
	
	FIBITMAP *temp = image;
	image = FreeImage_ConvertTo32Bits(temp);	// 把原本加载进来的图片转换成32位的图
	FreeImage_Unload(temp);

	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	char *buffer = new char[width * height * 4];
	memcpy(buffer, FreeImage_GetBits(image), width * height * 4);	// 把图像的数据拷贝到字符缓冲区

	FreeImage_Unload(image);

	// create opencl image
	cl_image_format clImageFormat;	// 此处格式配置要和输出图像的格式配置一致
	clImageFormat.image_channel_order = CL_RGBA;
	clImageFormat.image_channel_data_type = CL_UNORM_INT8;

	cl_int errNum;
	cl_mem clImage;
	clImage = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&clImageFormat, width, height, 0, buffer, &errNum);

	return clImage;
}

size_t RoundUp(int groupSize, int globalSize)
{
	int r = globalSize % groupSize;
	if (r == 0)
	{
		return globalSize;
	}
	else
	{
		return globalSize - r + groupSize;
	}
}

bool SaveImage(char *fileName, char *buffer, int width, int height)
{
	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(fileName);
	FIBITMAP *image = FreeImage_ConvertFromRawBits((BYTE*)buffer, width,
		height, width * 4, 32,
		0xFF000000, 0x00FF0000, 0x0000FF00);
	return (FreeImage_Save(format, image, fileName) == TRUE) ? true : false;
}