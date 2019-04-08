kernel void gauss_filter(__read_only image2d_t srcImg, __write_only image2d_t dstImg,
	sampler_t sampler, int width, int height)
{
	// gauss kernel is:
	// 1 2 1
	// 2 4 2
	// 1 2 1
	float kernelWeights[49] = { 1.0f, 2.0f, 1.0f,
								2.0f, 4.0f, 2.0f,
								1.0f, 2.0f, 1.0f, };

	int2 startImageCoord = (int2) (get_global_id(0) - 1, get_global_id(1) - 1);
	int2 endImageCoord = (int2) (get_global_id(0) + 1, get_global_id(1) + 1);
	int2 outImageCoord = (int2) (get_global_id(0), get_global_id(1));

	if (outImageCoord.x < width && outImageCoord.y < height)	// ��ǰҪ����ĵ��������ͼ��Χ��
	{
		/* �ù����������þ��ǰѰ����õ����ڵ���Χ9���㣬��һ��Ȩ��ȡ��ɫ */
		int weight = 0;	// ����ȡ��˹������ÿ�����Ȩ��
		float4 outColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
		for (int y = startImageCoord.y; y <= endImageCoord.y; y++)
		{
			for (int x = startImageCoord.x; x <= endImageCoord.x; x++)
			{
				outColor += (read_imagef(srcImg, sampler, (int2)(x, y)) * (kernelWeights[weight] / 16.0f));
				/* ����16����Ϊ���������Ϊ16 */

				weight += 1;
			}
		}

		// write the output value to image
		write_imagef(dstImg, outImageCoord, outColor);	// ��outColor�����ɫ���浽dstImg�ڴ�����outImageCoordλ��
	}
}

kernel void kaito_filter(__read_only image2d_t srcImg, __write_only image2d_t dstImg,
	sampler_t sampler, int width, int height)
{
	// gauss kernel is:
	// 1 2 1
	// 2 4 2
	// 1 2 1
	float kernelWeights[49] = { 64.0f, 32.0f, 64.0f, 48.0f, 64.0f, 32.0f, 64.0f,
							    32.0f, 64.0f, 48.0f, 16.0f, 48.0f, 64.0f, 32.0f,
							    64.0f, 48.0f, 16.0f, 32.0f, 16.0f, 48.0f, 64.0f,
								48.0f, 16.0f, 32.0f, 64.0f, 32.0f, 16.0f, 48.0f,
								64.0f, 48.0f, 16.0f, 32.0f, 16.0f, 48.0f, 64.0f,
								32.0f, 64.0f, 48.0f, 16.0f, 48.0f, 64.0f, 32.0f,
								64.0f, 32.0f, 64.0f, 48.0f, 64.0f, 32.0f, 64.0f, };

	float sum = 0.0f;
	for (int i = 0; i < 49; i++)
	{
		kernelWeights[i] = i;
		sum += kernelWeights[i];
	}

	int2 startImageCoord = (int2) (get_global_id(0) - 3, get_global_id(1) - 3);
	int2 endImageCoord = (int2) (get_global_id(0) + 3, get_global_id(1) + 3);
	int2 outImageCoord = (int2) (get_global_id(0), get_global_id(1));

	if (outImageCoord.x < width && outImageCoord.y < height)	// ��ǰҪ����ĵ��������ͼ��Χ��
	{
		/* �ù����������þ��ǰѰ����õ����ڵ���Χ9���㣬��һ��Ȩ��ȡ��ɫ */
		int weight = 0;	// ����ȡ��˹������ÿ�����Ȩ��
		float4 outColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
		for (int y = startImageCoord.y; y <= endImageCoord.y; y++)
		{
			for (int x = startImageCoord.x; x <= endImageCoord.x; x++)
			{
				outColor += (read_imagef(srcImg, sampler, (int2)(x, y)) * (kernelWeights[weight] / sum));
				/* ����16����Ϊ���������Ϊ16 */

				weight += 1;
			}
		}


		if ((outColor.x - outColor.y < 0.0002f) || (outColor.y - outColor.z < 0.0002f) || (outColor.x - outColor.z < 0.0002f))
		{
			outColor.x *= 0.2f;		/* blue */
			outColor.y *= 0.2f;		/* green */ 
			outColor.z *= 0.8f;		/* red */
		}

		// write the output value to image
		write_imagef(dstImg, outImageCoord, outColor);
	}
}