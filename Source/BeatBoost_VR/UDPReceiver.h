#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "UDPReceiver.generated.h"

UCLASS()
class BEATBOOST_VR_API AUDPReceiver : public AActor
{
	GENERATED_BODY()

public:
	AUDPReceiver();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// --- Variables que Blueprint podrá leer ---
	UPROPERTY(BlueprintReadOnly, Category = "Giroscopio")
	float GyroX;

	UPROPERTY(BlueprintReadOnly, Category = "Giroscopio")
	float GyroY;

	UPROPERTY(BlueprintReadOnly, Category = "Giroscopio")
	float GyroZ;

private:
	FSocket* ListenSocket;
};