// Created by ZhangJieyi

#include "PostProcessZBoxBlur.h"
#include "PostProcess/PostProcessCombineLUTs.h"
#include "CanvasTypes.h"
#include "RenderTargetTemp.h"
#include "UnrealEngine.h"

class FZBoxBlurPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZBoxBlurPS);
	SHADER_USE_PARAMETER_STRUCT(FZBoxBlurPS, FGlobalShader);

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

IMPLEMENT_GLOBAL_SHADER(FZBoxBlurPS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZBoxBlur.usf", "MainPS", SF_Pixel);

void AddZBoxBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZBoxBlurInputs Inputs)
{
	check(Inputs.SceneColor.IsValid());

	if (Inputs.Horizontal)
	{
		RDG_EVENT_SCOPE(GraphBuilder, "ZBoxBlur_H");
	}
	else
	{
		RDG_EVENT_SCOPE(GraphBuilder, "ZBoxBlur_V");
	}
	FRDGTextureRef OutputTexture = Inputs.SceneColor.Texture;
	const FScreenPassTextureViewport Viewport(Inputs.SceneColor);

	FZBoxBlurPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZBoxBlurPS::FParameters>();
	PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(Viewport);
	PassParameters->InputTexture = OutputTexture;
	PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParameters->Horizontal = Inputs.Horizontal;
	PassParameters->BlurRadius = Inputs.BlurRadius;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::ELoad);

	TShaderMapRef<FZBoxBlurPS> PixelShader(View.ShaderMap);

	AddDrawScreenPass(GraphBuilder, {}, View, Viewport, Viewport, PixelShader, PassParameters);
}