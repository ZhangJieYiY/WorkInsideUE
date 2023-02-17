// Created by ZhangJieyi

#include "PostProcessZSimpleDraw.h"
#include "PostProcess/PostProcessCombineLUTs.h"
#include "CanvasTypes.h"
#include "RenderTargetTemp.h"
#include "UnrealEngine.h"

class FZSimpleDrawPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZSimpleDrawPS);
	SHADER_USE_PARAMETER_STRUCT(FZSimpleDrawPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewportInfo)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};

IMPLEMENT_GLOBAL_SHADER(FZSimpleDrawPS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZSimpleDraw.usf", "MainPS", SF_Pixel);

void AddZSimpleDrawPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZSimpleDrawInputs Inputs)
{
	check(Inputs.SourceTexture);
	check(Inputs.TargetTexture);
	RDG_EVENT_SCOPE(GraphBuilder, "ZSimpleDraw");
	const FScreenPassTextureViewport Viewport(Inputs.TargetTexture);

	FZSimpleDrawPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZSimpleDrawPS::FParameters>();
	PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(Viewport);
	PassParameters->InputTexture = Inputs.SourceTexture;
	PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(Inputs.TargetTexture, ERenderTargetLoadAction::ELoad);

	TShaderMapRef<FZSimpleDrawPS> PixelShader(View.ShaderMap);

	AddDrawScreenPass(GraphBuilder, {}, View, Viewport, Viewport, PixelShader, PassParameters);
}