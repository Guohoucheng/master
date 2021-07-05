// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "TextActor.h"
#include "ActorWebSocket.generated.h"

UCLASS()
class PROJECT01_API AActorWebSocket : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActorWebSocket();

	const FString ServerURL = "ws://127.0.0.1:23335";
	const FString ServerProtocol = "ws";
	
	TSharedPtr<IWebSocket> Socket = nullptr;
	//TArray64<uint8>& PNGDataEx;

	//TArray<TSharedPtr<FJsonValue>> PngRawData;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	void OnConnected();
	void OnConnectionError(const FString& Error);
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnMessage(const FString& Message); // 接收消息时
	void OnMessageSent(const FString& MessageString); // 发送消息时

	void JsonParse(const FString InMessage);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	void MySend(UTextureRenderTarget2D* TexR);

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	class ATextActor* TextactorEx;

	UFUNCTION(BlueprintCallable)
	bool GetRawDataForText(UTextureRenderTarget2D* TexR);

	UFUNCTION(BlueprintCallable)
	void MyLoadFileToArray(const FString& FilePath, const FString& FileName);

}; 
