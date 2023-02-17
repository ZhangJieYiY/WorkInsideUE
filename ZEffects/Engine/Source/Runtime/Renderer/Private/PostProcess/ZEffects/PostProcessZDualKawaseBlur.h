// Created by ZhangJieyi

#pragma once

#include "ScreenPass.h"

struct FZDualKawaseBlurInputs
{
	FScreenPassTexture SceneColor;
	int DownUpTime = 4;
	float BlurRadius;
};

void AddZDualKawaseBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZDualKawaseBlurInputs Inputs);