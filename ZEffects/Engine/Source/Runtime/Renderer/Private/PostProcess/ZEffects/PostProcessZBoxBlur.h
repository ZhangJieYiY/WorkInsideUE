// Created by ZhangJieyi

#pragma once

#include "ScreenPass.h"

struct FZBoxBlurInputs
{
	FScreenPassTexture SceneColor;
	uint32 Horizontal;
	float BlurRadius;
};

void AddZBoxBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZBoxBlurInputs Inputs);