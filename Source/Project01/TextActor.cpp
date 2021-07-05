// Fill out your copyright notice in the Description page of Project Settings.


#include "TextActor.h"
#include "Engine/Texture2D.h"
#include "Misc/ObjectThumbnail.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RHI.h"
// Sets default values
ATextActor::ATextActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATextActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ATextActor::GetRawDataForText(UTextureRenderTarget2D* TexR)
{
	FRenderTarget* RenderTarget = TexR->GameThread_GetRenderTargetResource();
	EPixelFormat Format = TexR->GetFormat();
	
	int32 ImageBytes = CalculateImageBytes(TexR->SizeX, TexR->SizeY, 0, Format);
	TArray64<uint8> RawData;
	UE_LOG(LogTemp, Warning, TEXT("%s Result:%d"), *FString(__FUNCTION__), ImageBytes);//2457600
	RawData.AddUninitialized(ImageBytes);

	bool bReadSuccess = false;
	switch (Format)
	{
	case PF_FloatRGBA:
	{
		TArray<FFloat16Color> FloatColors;
		bReadSuccess = RenderTarget->ReadFloat16Pixels(FloatColors);
		FMemory::Memcpy(RawData.GetData(), FloatColors.GetData(), ImageBytes);
		if (RawData.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("RawData empty"));
		}
		for(auto &data: RawData)
		
		{
			PngRawData.Add(MakeShared<FJsonValueNumber>(data));
			UE_LOG(LogTemp, Error, TEXT("PngRawData emptyXXXXX:"));
		}

		
		if (PngRawData.Num()==0)
		{
			UE_LOG(LogTemp, Error, TEXT("PngRawData empty"));
		}
	}
	break;
	case PF_B8G8R8A8:
		bReadSuccess = RenderTarget->ReadPixelsPtr((FColor*)RawData.GetData());
		break;
	}
	if (bReadSuccess == false)
	{
		RawData.Empty();
	}
	return bReadSuccess;
	/*bool bSuccess = false;
    
	bSuccess= GetRawData(TexRT, RawData);
	return bSuccess;*/
} 

