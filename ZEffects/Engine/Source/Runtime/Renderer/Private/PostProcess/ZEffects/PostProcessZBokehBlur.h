// Created by ZhangJieyi

#pragma once

#include "ScreenPass.h"
#include "Math/UnrealMathUtility.h"

struct FZBokehBlurInputs
{
	FScreenPassTexture SceneColor;
	float BlurRadius;
	int Iteration;
};

namespace ZBokehblur
{
	// Precompute rotations
	const float c = FMath::Cos(2.39996323f);
	const float s = FMath::Sin(2.39996323f);
	const FVector4 GoldenRot(c, s, -s, c);
}

void AddZBokehBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZBokehBlurInputs Inputs);