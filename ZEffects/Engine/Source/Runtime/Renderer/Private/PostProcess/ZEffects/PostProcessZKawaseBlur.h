// Created by ZhangJieyi

#pragma once

#include "ScreenPass.h"

struct FZKawaseBlurInputs
{
	FScreenPassTexture SceneColor;
	float BlurRadius;
};

void AddZKawaseBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZKawaseBlurInputs Inputs);