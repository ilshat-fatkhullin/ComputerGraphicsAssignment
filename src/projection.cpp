#include "projection.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>


Projection::Projection(unsigned short width, unsigned short height, std::string obj_file) : ReadObj(width, height, obj_file)
{
	x_center = width / 2.0;
	y_center = height / 2.0;
}

Projection::~Projection()
{
}

void Projection::DrawScene()
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

	float4x4 viewMatrix {
		{xAxis.x, xAxis.y, xAxis.z, -dot(xAxis, eyePosition)},
		{yAxis.x, yAxis.y, yAxis.z, -dot(yAxis, eyePosition)},
		{zAxis.x, zAxis.y, zAxis.z, -dot(zAxis, eyePosition)},
		{0, 0, 0, 1},
	};

	float4x4 translationMatrix {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 2, 1},
	};

	float4x4 projectionMatrix {
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
		}
		DrawTriangle(translated, color(255, 255, 255));
	}
}

void Projection::DrawTriangle(float4 triangle[3], color color)
{
	DrawLine(static_cast<unsigned short>(x_center + x_center * triangle[0].x),
		static_cast<unsigned short>(y_center - y_center * triangle[0].y),
		static_cast<unsigned short>(x_center + x_center * triangle[1].x),
		static_cast<unsigned short>(y_center - y_center * triangle[1].y), color);

	DrawLine(static_cast<unsigned short>(x_center + x_center * triangle[1].x),
		static_cast<unsigned short>(y_center - y_center * triangle[1].y),
		static_cast<unsigned short>(x_center + x_center * triangle[2].x),
		static_cast<unsigned short>(y_center - y_center * triangle[2].y), color);

	DrawLine(static_cast<unsigned short>(x_center + x_center * triangle[2].x),
		static_cast<unsigned short>(y_center - y_center * triangle[2].y),
		static_cast<unsigned short>(x_center + x_center * triangle[0].x),
		static_cast<unsigned short>(y_center - y_center * triangle[0].y), color);
}

