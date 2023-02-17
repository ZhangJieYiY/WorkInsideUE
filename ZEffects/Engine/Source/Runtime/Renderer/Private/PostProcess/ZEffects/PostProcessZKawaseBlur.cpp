// Created by ZhangJieyi

#include "PostProcessZKawaseBlur.h"
#include "PostProcess/PostProcessCombineLUTs.h"
#include "CanvasTypes.h"
#include "RenderTargetTemp.h"
#include "UnrealEngine.h"

class FZKawaseBlurPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZKawaseBlurPS);
	SHADER_USE_PARAMETER_STRUCT(FZKawaseBlurPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewportInfo)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		SHADER_PARAMETER(float, BlurRadius)
		RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};

IMPLEMENT_GLOBAL_SHADER(FZKawaseBlurPS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZKawaseBlur.usf", "MainPS", SF_Pixel);

void AddZKawaseBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZKawaseBlurInputs Inputs)
{
	check(Inputs.SceneColor.IsValid());
	RDG_EVENT_SCOPE(GraphBuilder, "ZKawaseBlur");
	FRDGTextureRef OutputTexture = Inputs.SceneColor.Texture;
	const FScreenPassTextureViewport Viewport(Inputs.SceneColor);

	FZKawaseBlurPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZKawaseBlurPS::FParameters>();
	PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(Viewport);
	PassParameters->InputTexture = OutputTexture;
	PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParameters->BlurRadius = Inputs.BlurRadius;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::ELoad);

	TShaderMapRef<FZKawaseBlurPS> PixelShader(View.ShaderMap);

	AddDrawScreenPass(GraphBuilder, {}, View, Viewport, Viewport, PixelShader, PassParameters);
}