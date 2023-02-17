// Created by ZhangJieyi

#include "PostProcessZDualKawaseBlur.h"
#include "PostProcess/PostProcessCombineLUTs.h"
#include "CanvasTypes.h"
#include "RenderTargetTemp.h"
#include "UnrealEngine.h"

class FZDualKawaseBlurDownSamplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZDualKawaseBlurDownSamplePS);
	SHADER_USE_PARAMETER_STRUCT(FZDualKawaseBlurDownSamplePS, FGlobalShader);

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

class FZDualKawaseBlurUpSamplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FZDualKawaseBlurUpSamplePS);
	SHADER_USE_PARAMETER_STRUCT(FZDualKawaseBlurUpSamplePS, FGlobalShader);

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

IMPLEMENT_GLOBAL_SHADER(FZDualKawaseBlurDownSamplePS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZDualKawaseBlur.usf", "DownSamplePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FZDualKawaseBlurUpSamplePS, "/Engine/Private/ZEffects/PostProcessing/Blurs/PostProcessZDualKawaseBlur.usf", "UpSamplePS", SF_Pixel);

void AddZDualKawaseBlurPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, FZDualKawaseBlurInputs Inputs)
{
	check(Inputs.SceneColor.IsValid());

	FRDGTextureRef OutputTexture = Inputs.SceneColor.Texture;
	const FScreenPassTextureViewport RawViewport(Inputs.SceneColor);

	// Data setup
	FRDGTextureRef PreviousBuffer = OutputTexture;
	const FRDGTextureDesc& RTDescription = OutputTexture->Desc;

	float ViewportScale = 1;
	for (int i = 1; i <= Inputs.DownUpTime; i++)
	{
		ViewportScale /= 2;
		FIntPoint ScaledExtent = GetScaledExtent(RawViewport.Extent, ViewportScale);
		FScreenPassTextureViewport NewViewport = FScreenPassTextureViewport(ScaledExtent);

		// Build texture
		FRDGTextureDesc BlurDesc = RTDescription;
		BlurDesc.Reset();
		BlurDesc.Extent = ScaledExtent;
		BlurDesc.NumMips = 1;
		BlurDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);

		const FString RTName = FString(TEXT("ZDualKawaseBlurDownScale")) + FString::Printf(TEXT("_%ix%i"), ScaledExtent.X, ScaledExtent.Y);
		FRDGTextureRef Buffer = GraphBuilder.CreateTexture(BlurDesc, *RTName);

		RDG_EVENT_SCOPE(GraphBuilder, "ZDualKawaseBlurDownSample");
		FZDualKawaseBlurDownSamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZDualKawaseBlurDownSamplePS::FParameters>();
		PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(NewViewport);
		PassParameters->InputTexture = PreviousBuffer;
		PassParameters->BlurRadius = Inputs.BlurRadius;
		PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParameters->RenderTargets[0] = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ELoad);

		TShaderMapRef<FZDualKawaseBlurDownSamplePS> PixelShader(View.ShaderMap);

		AddDrawScreenPass(GraphBuilder, {}, View, NewViewport, NewViewport, PixelShader, PassParameters);

		PreviousBuffer = Buffer;
	}

	for (int i = 1; i <= Inputs.DownUpTime; i++)
	{
		ViewportScale *= 2;
		FIntPoint ScaledExtent = GetScaledExtent(RawViewport.Extent, ViewportScale);
		FScreenPassTextureViewport NewViewport = FScreenPassTextureViewport(ScaledExtent);

		if (i == Inputs.DownUpTime)
		{
			RDG_EVENT_SCOPE(GraphBuilder, "ZDualKawaseBlurUpSample");
			FZDualKawaseBlurUpSamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZDualKawaseBlurUpSamplePS::FParameters>();
			PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(NewViewport);
			PassParameters->InputTexture = PreviousBuffer;
			PassParameters->BlurRadius = Inputs.BlurRadius;
			PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::ELoad);
			TShaderMapRef<FZDualKawaseBlurUpSamplePS> PixelShader(View.ShaderMap);
			AddDrawScreenPass(GraphBuilder, {}, View, NewViewport, NewViewport, PixelShader, PassParameters);
		}
		else
		{
			// Build texture
			FRDGTextureDesc BlurDesc = RTDescription;
			BlurDesc.Reset();
			BlurDesc.Extent = ScaledExtent;
			BlurDesc.NumMips = 1;
			BlurDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);

			const FString RTName = FString(TEXT("ZDualKawaseBlurUpScale")) + FString::Printf(TEXT("_%ix%i"), ScaledExtent.X, ScaledExtent.Y);
			FRDGTextureRef Buffer = GraphBuilder.CreateTexture(BlurDesc, *RTName);

			RDG_EVENT_SCOPE(GraphBuilder, "ZDualKawaseBlurUpSample");
			FZDualKawaseBlurUpSamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FZDualKawaseBlurUpSamplePS::FParameters>();
			PassParameters->ViewportInfo = GetScreenPassTextureViewportParameters(NewViewport);
			PassParameters->InputTexture = PreviousBuffer;
			PassParameters->BlurRadius = Inputs.BlurRadius;
			PassParameters->InputSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			PassParameters->RenderTargets[0] = FRenderTargetBinding(Buffer, ERenderTargetLoadAction::ELoad);
			PreviousBuffer = Buffer;
			TShaderMapRef<FZDualKawaseBlurUpSamplePS> PixelShader(View.ShaderMap);
			AddDrawScreenPass(GraphBuilder, {}, View, NewViewport, NewViewport, PixelShader, PassParameters);
		}

	}

}

