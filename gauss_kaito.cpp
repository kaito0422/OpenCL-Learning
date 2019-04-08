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

	/* ��ͼƬ��� */
	cl_mem my_image;	// ���ڱ��������ͼ��
	int width, height;	
	cl_mem image_out;	// ���ڱ��������ͼ��

	// create output image object
	cl_image_format clImageFormat;
	clImageFormat.image_channel_order = CL_RGBA;	// RGBA�е�RGB����ԭɫ��A��ʾAlpha��Ϊ͸����
	clImageFormat.image_channel_data_type = CL_UNORM_INT8;	// ��ͼ��ͨ������8λ����ӳ�䵽[0.0, 1.0], Unsigned NORMalize
	/* �ô�ͨ������RGBA4��ͨ����һ�����ص���8λ��ʾ */			/* ������kernel�ļ��е���ɫ��ΧҲ����[0.0��1.0] */

	my_image = LoadImage(context, "lin.jpg", width, height);	// ����һ��ͼƬ�����ݵ��ڴ����
	image_out = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &clImageFormat, width, height, 0, NULL, NULL);

	cl_sampler sampler;
	/* ���������� */
	sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST, NULL);
	                                  /* �ǹ��  ���곬������ǯ�Ƶ���Ե    ������� */

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &my_image);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &image_out);
	clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
	clSetKernelArg(kernel, 3, sizeof(int), &width);
	clSetKernelArg(kernel, 4, sizeof(int), &height);

	size_t local_size[] = { 16, 16 };
	size_t global_size[] = { RoundUp(local_size[0], width), RoundUp(local_size[1], height) };
							/* ���global��С����local��С�������������ȡһ��local�Ĵ�С */

	/* kernel�Ŷ� */
	errNum = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);

	/* ���屣�������buffer */ 
	char *buffer = new char[width * height * 4];
	size_t origin[3] = { 0,0,0 };	// ��[0, 0]���꿪ʼ��ȡ����3λӦ���Ǳ�ʾ��ȣ���0��1
	size_t region[3] = { width, height, 1 };	// ��ȡwidth��ȣ�height�߶�
	clEnqueueReadImage(queue, image_out, CL_TRUE, origin, region, 0, 0, buffer, 0, NULL, NULL);

	SaveImage("kaito.png", buffer, width, height);	/* ��buffer���������FreeImage�����ͼƬ��ʽ */

	clean_up(context, queue, program, kernel);

	system("pause");
	return 0;
}

cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height)
{
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName, 0);	// ��ȡͼƬ�ļ�����
	FIBITMAP *image = FreeImage_Load(format, fileName);	// ����һ��ͼƬ����ʽΪformat��ʽ
	
	FIBITMAP *temp = image;
	image = FreeImage_ConvertTo32Bits(temp);	// ��ԭ�����ؽ�����ͼƬת����32λ��ͼ
	FreeImage_Unload(temp);

	width = FreeImage_GetWidth(image);
	height = FreeImage_GetHeight(image);

	char *buffer = new char[width * height * 4];
	memcpy(buffer, FreeImage_GetBits(image), width * height * 4);	// ��ͼ������ݿ������ַ�������

	FreeImage_Unload(image);

	// create opencl image
	cl_image_format clImageFormat;	// �˴���ʽ����Ҫ�����ͼ��ĸ�ʽ����һ��
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