// Created by ZhangJieyi

#include "PostProcessZGaussianBlur.h"
#include "PostProcess/PostProcessCombineLUTs.h"
#include "CanvasTypes.h"
#include "RenderTargetTemp.h"
#include "UnrealEngine.h"

class FZGaussianBlurPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZGaussianBlurPS);
	SHADER_USE_PARAMETER_STRUCT(FZGaussianBlurPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewportInfo)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		SHADER_PARAMETER(uint32, Horizontal)
		SHADER_PARAMETER(float, BlurRadius)
		RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};

IMPLEMENT_GLOBAL_SHADER(FZGaussianBlurPS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZGaussianBlur.usf", "MainPS", SF_Pixel);

void AddZGaussianBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZGaussianBlurInputs Inputs)
{
	check(Inputs.SceneColor.IsValid());

	if (Inputs.Horizontal)
	{
		RDG_EVENT_SCOPE(GraphBuilder, "ZGaussianBlur_H");
	}
	else
	{
		RDG_EVENT_SCOPE(GraphBuilder, "ZGaussianBlur_V");
	}

	FRDGTextureRef SceneColorTexture = Inputs.SceneColor.Texture;
	const FScreenPassTextureViewport Viewport(Inputs.SceneColor);

	FZGaussianBlurPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZGaussianBlurPS::FParameters>();
	PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(Viewport);
	PassParameters->InputTexture = SceneColorTexture;
	PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParameters->Horizontal = Inputs.Horizontal;
	PassParameters->BlurRadius = Inputs.BlurRadius;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(SceneColorTexture, ERenderTargetLoadAction::ELoad);/*Output.GetRenderTargetBinding()*/;

	TShaderMapRef<FZGaussianBlurPS> PixelShader(View.ShaderMap);

	AddDrawScreenPass(GraphBuilder, {}, View, Viewport, Viewport, PixelShader, PassParameters);
}