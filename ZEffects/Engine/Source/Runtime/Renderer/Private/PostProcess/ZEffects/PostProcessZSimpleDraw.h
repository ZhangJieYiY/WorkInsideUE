// Created by ZhangJieyi

#pragma once

#include "ScreenPass.h"

struct FZSimpleDrawInputs
{
	FRDGTextureRef SourceTexture;
	FRDGTextureRef TargetTexture;
};

void AddZSimpleDrawPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZSimpleDrawInputs Inputs);