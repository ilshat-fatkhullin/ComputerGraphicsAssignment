#include "triangle_rasterization.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>


TriangleRasterization::TriangleRasterization(unsigned short width, unsigned short height, std::string obj_file) : Projection(width, height, obj_file)
{

}

TriangleRasterization::~TriangleRasterization()
{

}

void TriangleRasterization::DrawScene()
{
	parser->Parse();

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	float fov = M_PI * 1.0f / 3.0f;

	float yScale = 1.0f / tan(fov / 2.0f);
	float xScale = yScale / aspectRatio;

	float zNear = 1;
	float zFar = 10;

	float rotationAngle = 0.0f;

	float3 eyePosition{ 0, 0, 0 };
	float3 lookAtPosition{ 0, 0, 1 };
	float3 upDirection{ 0, 1, 0 };

	float3 zAxis = normalize(eyePosition - lookAtPosition);
	float3 xAxis = normalize(cross(upDirection, zAxis));
	float3 yAxis = cross(xAxis, zAxis);

	float4x4 rotationMatrix{
		{1, 0, 0, 0},
		{0, cos(rotationAngle), sin(rotationAngle), 0},
		{0, -sin(rotationAngle), cos(rotationAngle), 0},
		{0, 0, 0, 1},
	};

	float4x4 viewMatrix{
		{xAxis.x, xAxis.y, xAxis.z, -dot(xAxis, eyePosition)},
		{yAxis.x, yAxis.y, yAxis.z, -dot(yAxis, eyePosition)},
		{zAxis.x, zAxis.y, zAxis.z, -dot(zAxis, eyePosition)},
		{0, 0, 0, 1},
	};

	float4x4 translationMatrix{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 2, 1},
	};

	float4x4 projectionMatrix{
		{xScale, 0, 0, 0},
		{0, yScale, 0, 0},
		{0, 0, zFar / (zFar - zNear), -zNear * zFar / (zFar - zNear)},
		{0, 0, 1, 0},
	};

	float4x4 resultMatrix = linalg::mul(projectionMatrix, viewMatrix, mul(translationMatrix, rotationMatrix));

	for (auto face : parser->GetFaces()) {
		float4 translated[3];
		for (unsigned short i = 0; i < 3; i++) {
			translated[i] = linalg::mul(resultMatrix, face.vertexes[i]);
			translated[i] /= translated[i].w;
			translated[i].x = x_center + x_center * translated[i].x;
			translated[i].y = y_center - y_center * translated[i].y;
		}
		TriangleRasterization::DrawTriangle(translated);
	}
}

void TriangleRasterization::DrawTriangle(float4 triangle[3])
{
	float minX = width;
	float maxX = 0;
	float minY = height;
	float maxY = 0;

	for (int i = 0; i < 3; i++) {
		if (triangle[i].x < minX)
		{
			minX = triangle[i].x;
		}
		if (triangle[i].x > maxX)
		{
			maxX = triangle[i].x;
		}
		if (triangle[i].y < minY)
		{
			minY = triangle[i].y;
		}
		if (triangle[i].y > maxY)
		{
			maxY = triangle[i].y;
		}
	}

	for (float x = minX; x <= maxX; x++)
		for (float y = minY; y <= maxY; y++)
		{
			float2 point = float2{ x, y };
			float zone0 = EdgeFunction(triangle[0].xy(), triangle[1].xy(), point);
			float zone1 = EdgeFunction(triangle[1].xy(), triangle[2].xy(), point);
			float zone2 = EdgeFunction(triangle[2].xy(), triangle[0].xy(), point);
			if (zone0 >= 0.0 && zone1 >= 0.0 && zone2 >= 0.0)
			{
				ColorSpace::SetPixel(x, y, color(255, 0, 0));
			}
		}

	DrawLine(static_cast<unsigned short>(triangle[0].x),
		static_cast<unsigned short>(triangle[0].y),
		static_cast<unsigned short>(triangle[1].x),
		static_cast<unsigned short>(triangle[1].y), color(255, 255, 255));

	DrawLine(static_cast<unsigned short>(triangle[1].x),
		static_cast<unsigned short>(triangle[1].y),
		static_cast<unsigned short>(triangle[2].x),
		static_cast<unsigned short>(triangle[2].y), color(255, 255, 255));

	DrawLine(static_cast<unsigned short>(triangle[2].x),
		static_cast<unsigned short>(triangle[2].y),
		static_cast<unsigned short>(triangle[0].x),
		static_cast<unsigned short>(triangle[0].y), color(255, 255, 255));
}

float TriangleRasterization::EdgeFunction(float2 a, float2 b, float2 c)
{
	float2 d = b - c;
	return (c.x - a.x) * d.y - (c.y - a.y) * d.x;
}


