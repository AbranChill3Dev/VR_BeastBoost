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

    // ?? Aceleración cruda (por si la necesitas en Blueprint para debug) ??
    UPROPERTY(BlueprintReadOnly, Category = "UDP|Raw")
    float AccelX = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "UDP|Raw")
    float AccelY = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "UDP|Raw")
    float AccelZ = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "UDP|Raw")
    float RightAccelX = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "UDP|Raw")
    float RightAccelY = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category = "UDP|Raw")
    float RightAccelZ = 0.0f;

    // ?? Posición acumulada — ESTO es lo que usarás en Blueprint ??
    UPROPERTY(BlueprintReadOnly, Category = "UDP|Haptic")
    FVector LeftHandPos = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "UDP|Haptic")
    FVector RightHandPos = FVector::ZeroVector;

    // ?? Parámetros ajustables desde el Editor sin recompilar ??

    /** Valores del sensor por debajo de esto se tratan como 0 (anti-temblor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic")
    float DeadZone = 800.0f;

    /** Suavizado de movimiento. 0.1 = muy suave, 0.9 = casi sin suavizado */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic")
    float SmoothFactor = 0.2f;

    /** Convierte unidades del sensor a centímetros en Unreal */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic")
    float SensorScale = 0.0015f;

    /** Límite máximo de desplazamiento desde el origen (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic")
    float MaxDisplacement = 25.0f;

    // ?? Control de ejes ??

    /** Eje del sensor que mapea a IZQUIERDA-DERECHA en Unreal (0=X, 1=Y, 2=Z) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    int32 EjeHorizontal = 0;

    /** Eje del sensor que mapea a ARRIBA-ABAJO en Unreal (0=X, 1=Y, 2=Z) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    int32 EjeVertical = 1;

    /** Invierte el eje horizontal si el movimiento va al revés */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    bool bInvertirHorizontal = false;

    /** Invierte el eje vertical si el movimiento va al revés */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    bool bInvertirVertical = false;

    // Para la mano derecha por separado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    int32 RightEjeHorizontal = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    int32 RightEjeVertical = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    bool bRightInvertirHorizontal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UDP|Haptic|Ejes")
    bool bRightInvertirVertical = false;

    UFUNCTION(BlueprintCallable, Category = "UDP")
    void MandarVibracion(FString IPObjetivo, int32 Puerto, FString Mensaje);

    /** Llama esto desde Blueprint para resetear las manos al origen */
    UFUNCTION(BlueprintCallable, Category = "UDP|Haptic")
    void ResetHandPositions();

private:
    FSocket* ListenSocket;

    void ParsePacket(const FString& Mensaje);

    FORCEINLINE float ApplyDeadZone(float Value) const
    {
        return (FMath::Abs(Value) < DeadZone) ? 0.0f : Value;
    }

    /** Dado un array de 3 floats del sensor, regresa un FVector 2D mapeado correctamente */
    FVector MapearEjes(const float Sensor[3], int32 EjeH, int32 EjeV,
        bool bInvH, bool bInvV) const;
};