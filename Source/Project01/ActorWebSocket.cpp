// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorWebSocket.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ObjectThumbnail.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Containers/Array.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"

// Sets default values
AActorWebSocket::AActorWebSocket()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AActorWebSocket::BeginPlay()
{
	Super::BeginPlay();
	FModuleManager::Get().LoadModuleChecked("WebSockets");

	Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol);

	Socket->OnConnected().AddUObject(this, &AActorWebSocket::OnConnected);
	Socket->OnConnectionError().AddUObject(this, &AActorWebSocket::OnConnectionError);
	Socket->OnClosed().AddUObject(this, &AActorWebSocket::OnClosed);
	Socket->OnMessage().AddUObject(this, &AActorWebSocket::OnMessage);//接收
	Socket->OnMessageSent().AddUObject(this, &AActorWebSocket::OnMessageSent);//发送

	Socket->Connect();
	//FTimerHandle TimerHandle;
	//GetWorldTimerManager().SetTimer(TimerHandle, this, &AActorWebSocket::MySend, 1, true, 1);
}

void AActorWebSocket::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Socket->Close();
}

// Called every frame
void AActorWebSocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AActorWebSocket::GetRawDataForText(UTextureRenderTarget2D* TexR)
{
	bool bSuccess = false;

	check(TexR != nullptr);
	FRenderTarget* RenderTarget = TexR->GameThread_GetRenderTargetResource();
	FIntPoint Size = RenderTarget->GetSizeXY();
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

	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	TSharedPtr<IImageWrapper> PNGImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	PNGImageWrapper->SetRaw(RawData.GetData(), RawData.GetAllocatedSize(), Size.X, Size.Y, ERGBFormat::BGRA, 8);

	const TArray64<uint8>& PNGData = PNGImageWrapper->GetCompressed(100);
	//Ar.Serialize((void*)PNGData.GetData(), PNGData.GetAllocatedSize());

	return bReadSuccess;
}

void AActorWebSocket::MyLoadFileToArray(const FString& FilePath, const FString& FileName)
{
	TArray64<uint8> BinaryMessage;
	FString TotalFileName = FPaths::Combine(*FilePath, *FileName);
	
	FFileHelper::LoadFileToArray(BinaryMessage, *TotalFileName);//从PNG文件里读取UINT8
	//FFileHelper::SaveArrayToFile(BinaryMessage, TEXT("C:/Users/v_houchguo/Desktop/textpng.txt"));//导入到文件
	if (Socket->IsConnected())
	{
		//Socket->Send(Jsonstr);

		Socket->Send(BinaryMessage.GetData(), BinaryMessage.Num(), true);//发送给WEB
		UE_LOG(LogTemp, Error, TEXT("sConnected succssesssssss"));


	}

}

void AActorWebSocket::MySend(UTextureRenderTarget2D* TexR)
{
	bool bSuccess = false;

	check(TexR != nullptr);
	FRenderTarget* RenderTarget = TexR->GameThread_GetRenderTargetResource();
	FIntPoint Size = RenderTarget->GetSizeXY();
	EPixelFormat Format = TexR->GetFormat();
	FBufferArchive Ar;
	int32 ImageBytes = CalculateImageBytes(TexR->SizeX, TexR->SizeY, 0, Format);
	TArray64<uint8> RawData;
	const FString& FilePath= "E:/Unreal Projects/Project01/Content/Content/PNG";
	const FString& FileName= "text.png";
	FString TotalFileName = FPaths::Combine(*FilePath, *FileName);
	FText PathError;
	FPaths::ValidatePath(TotalFileName, &PathError);
	FArchive* ArEX = IFileManager::Get().CreateFileWriter(*TotalFileName);
	RawData.AddUninitialized(ImageBytes);

	bool bReadSuccess = false;
	switch (Format)
	{
	case PF_FloatRGBA:
	{
		TArray<FFloat16Color> FloatColors;
		bReadSuccess = RenderTarget->ReadFloat16Pixels(FloatColors);
		FMemory::Memcpy(RawData.GetData(), FloatColors.GetData(), ImageBytes);

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
	IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	TSharedPtr<IImageWrapper> PNGImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	PNGImageWrapper->SetRaw(RawData.GetData(), RawData.GetAllocatedSize(), Size.X, Size.Y, ERGBFormat::BGRA, 8);
	//bReadSuccess = PNGImageWrapper->GetRaw(ERGBFormat::BGRA, 8, OutRawData);
	
	const TArray64<uint8>& PNGData = PNGImageWrapper->GetCompressed(100);
	
	Ar.Serialize((void*)PNGData.GetData(), PNGData.GetAllocatedSize());		
	ArEX->Serialize(const_cast<uint8*>(Ar.GetData()), Ar.Num());

	TSharedPtr<FJsonObject> RootObj(new FJsonObject);
	RootObj->SetBoolField("IsExecute", true);
	RootObj->SetStringField("Type", "Test");
	TSharedPtr<FJsonValue> Value(new FJsonValueNumber(GetGameTimeSinceCreation()));
	RootObj->SetField("Value", Value);
	FString Jsonstr;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&Jsonstr);
	FJsonSerializer::Serialize(RootObj.ToSharedRef(), JsonWriter);
	const TArray64<uint8> BinaryMessage = { 'H', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e', ' ', '!' };
	if (Socket->IsConnected())
	{
	    //Socket->Send(Jsonstr);
		
		Socket->Send(Ar.GetData(), Ar.GetAllocatedSize(),true);
		UE_LOG(LogTemp, Error, TEXT("sConnected succsse"));
		

	}
	UE_LOG(LogTemp, Warning, TEXT("%s Result:%d"), *FString(__FUNCTION__), BinaryMessage.GetAllocatedSize());
}



void AActorWebSocket::OnConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(__FUNCTION__));
}

void AActorWebSocket::OnConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("%s Error:%s"), *FString(__FUNCTION__), *Error);
}

void AActorWebSocket::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Warning, TEXT("%s StatusCode:%d Reason:%s bWasClean:%d"),
		*FString(__FUNCTION__), StatusCode, *Reason, bWasClean);
}

void AActorWebSocket::OnMessage(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s Message:%s"), *FString(__FUNCTION__), *Message);
	JsonParse(Message);
}

void AActorWebSocket::OnMessageSent(const FString& MessageString)
{
	UE_LOG(LogTemp, Warning, TEXT("%s MessageString:%s"), *FString(__FUNCTION__), *MessageString);
}

void AActorWebSocket::JsonParse(const FString InMessage)
{
	TSharedPtr<FJsonObject> RootObj=MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InMessage);

	if (FJsonSerializer::Deserialize(JsonReader, RootObj)) // 通过JsonReader读取RootObj
	{
		FString Topic = RootObj->GetStringField("Topic"); // Topic
		UE_LOG(LogTemp, Warning, TEXT("%s Topic:%s"), *FString(__FUNCTION__), *Topic);

		TSharedPtr<FJsonObject> DataObj = MakeShareable(new FJsonObject());
		DataObj = RootObj->GetObjectField("Data"); // Data

		FString Key = DataObj->GetStringField("Key"); // Key
		UE_LOG(LogTemp, Warning, TEXT("%s Key:%s"), *FString(__FUNCTION__), *Key);

		if (Key.Equals("Time"))
		{
			FString Time = DataObj->GetStringField("Value"); // Value type is String
			UE_LOG(LogTemp, Warning, TEXT("%s Time:%s"), *FString(__FUNCTION__), *Time);
		}
		else if (Key.Equals("Something"))
		{
			TArray<TSharedPtr<FJsonValue>> Value = DataObj->GetArrayField("Value"); // Value type is Array

			bool BoolValue = Value[0]->AsBool();
			int IntValue = Value[1]->AsNumber();
			float FloatValue = Value[2]->AsNumber();
			FString StringValue = Value[3]->AsString();

			UE_LOG(LogTemp, Warning, TEXT("%s BoolValue:%d"), *FString(__FUNCTION__), BoolValue);
			UE_LOG(LogTemp, Warning, TEXT("%s IntValue:%d"), *FString(__FUNCTION__), IntValue);
			UE_LOG(LogTemp, Warning, TEXT("%s FloatValue:%f"), *FString(__FUNCTION__), FloatValue);
			UE_LOG(LogTemp, Warning, TEXT("%s StringValue:%s"), *FString(__FUNCTION__), *StringValue);
		}
	}
}


