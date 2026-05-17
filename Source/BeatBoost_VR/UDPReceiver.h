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

	// --- Variables que Blueprint podr� leer ---
	UPROPERTY(BlueprintReadOnly, Category = "UDP")
	float AccelX = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "UDP")
	float AccelY = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "UDP")
	float AccelZ = 0.0f;

		// Variables de la mano DERECHA (¡NUEVAS!)
	UPROPERTY(BlueprintReadOnly, Category = "UDP")
	float RightAccelX;
	UPROPERTY(BlueprintReadOnly, Category = "UDP")
	float RightAccelY;
	UPROPERTY(BlueprintReadOnly, Category = "UDP")
	float RightAccelZ;

	// Función para mandar mensajes al ESP32
    UFUNCTION(BlueprintCallable, Category = "UDP")
    void MandarVibracion(FString IPObjetivo, int32 Puerto, FString Mensaje);

private:
	FSocket* ListenSocket;
};