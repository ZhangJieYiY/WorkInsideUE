// Created by ZhangJieyi

#include "PostProcessZBokehBlur.h"
#include "PostProcessZSimpleDraw.h"
#include "PostProcess/PostProcessCombineLUTs.h"
#include "CanvasTypes.h"
#include "RenderTargetTemp.h"
#include "UnrealEngine.h"

class FZBokehBlurPS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZBokehBlurPS);
	SHADER_USE_PARAMETER_STRUCT(FZBokehBlurPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewportInfo)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
		SHADER_PARAMETER(FVector4f, GoldenRot)
		SHADER_PARAMETER(float, BlurRadius)
		SHADER_PARAMETER(int, Iteration)
		RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};

IMPLEMENT_GLOBAL_SHADER(FZBokehBlurPS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZBokehBlur.usf", "MainPS", SF_Pixel);

void AddZBokehBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZBokehBlurInputs Inputs)
{
	check(Inputs.SceneColor.IsValid());
	RDG_EVENT_SCOPE(GraphBuilder, "ZBokehBlur");
	FRDGTextureRef OutputTexture = Inputs.SceneColor.Texture;
	const FScreenPassTextureViewport Viewport(Inputs.SceneColor);

	// Data setup
	FRDGTextureRef PreviousBuffer = OutputTexture;
	const FRDGTextureDesc& RTDescription = OutputTexture->Desc;
	// Build texture
	FRDGTextureDesc BlurDesc = RTDescription;
	BlurDesc.Reset();
	BlurDesc.Extent = Viewport.Extent;
	BlurDesc.NumMips = 1;
	BlurDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);

	const FString RTName = FString(TEXT("ZBokehBlurRT")) + FString::Printf(TEXT("_%ix%i"), Viewport.Extent.X, Viewport.Extent.Y);
	FRDGTextureRef Buffer = GraphBuilder.CreateTexture(BlurDesc, *RTName);

	FZBokehBlurPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZBokehBlurPS::FParameters>();
	PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(Viewport);
	PassParameters->InputTexture = OutputTexture;
	PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParameters->GoldenRot = (FVector4f)ZBokehblur::GoldenRot;
	PassParameters->BlurRadius = Inputs.BlurRadius;
	PassParameters->Iteration = Inputs.Iteration;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ELoad);

	TShaderMapRef<FZBokehBlurPS> PixelShader(View.ShaderMap);
	AddDrawScreenPass(GraphBuilder, {}, View, Viewport, Viewport, PixelShader, PassParameters);


	//Draw back to screen
	FZSimpleDrawInputs V_ZPassInputs;
	V_ZPassInputs.SourceTexture = Buffer;
	V_ZPassInputs.TargetTexture = OutputTexture;
	AddZSimpleDrawPass(GraphBuilder, View, V_ZPassInputs);
}