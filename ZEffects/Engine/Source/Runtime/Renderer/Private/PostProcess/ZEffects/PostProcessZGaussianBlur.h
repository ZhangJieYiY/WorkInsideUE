// Created by ZhangJieyi

#pragma once

#include "ScreenPass.h"

struct FZGaussianBlurInputs
{
	FScreenPassTexture SceneColor;
	uint32 Horizontal;
	float BlurRadius;
};

void AddZGaussianBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZGaussianBlurInputs Inputs);