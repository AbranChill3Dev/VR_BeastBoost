#include "UDPReceiver.h"

AUDPReceiver::AUDPReceiver()
{
	PrimaryActorTick.bCanEverTick = true;
	ListenSocket = nullptr;

	// Iniciamos las variables en 0
	AccelX = 0.0f;
	AccelY = 0.0f;
	AccelZ = 0.0f;
}

void AUDPReceiver::BeginPlay()
{
	Super::BeginPlay();

	FIPv4Endpoint Endpoint(FIPv4Address::Any, 8001);
	ListenSocket = FUdpSocketBuilder(TEXT("LectorUDP"))
		.AsNonBlocking().AsReusable().BoundToEndpoint(Endpoint).WithReceiveBufferSize(2 * 1024 * 1024);

	if (ListenSocket) {
		UE_LOG(LogTemp, Warning, TEXT("¡EXITO! Escuchando UDP..."));
	}
}

void AUDPReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (ListenSocket) {
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
	}
}

void AUDPReceiver::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!ListenSocket) return;

	TSharedRef<FInternetAddr> Sender = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	uint32 Size;

	while (ListenSocket->HasPendingData(Size))
	{
		TArray<uint8> ReceivedData;
		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
		int32 BytesRead = 0;
		ListenSocket->RecvFrom(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *Sender);

		if (BytesRead > 0)
		{
			ReceivedData.Add(0);
			FString Mensaje = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

			// --- Recortar los números ---
			TArray<FString> Partes;
			Mensaje.ParseIntoArray(Partes, TEXT(" "), true);

			if (Partes.Num() >= 3)
			{
				// CAMBIO AQUÍ: Usamos las nuevas variables y buscamos AX, AY, AZ
				AccelX = FCString::Atof(*Partes[0].Replace(TEXT("AX:"), TEXT("")));
				AccelY = FCString::Atof(*Partes[1].Replace(TEXT("AY:"), TEXT("")));
				AccelZ = FCString::Atof(*Partes[2].Replace(TEXT("AZ:"), TEXT("")));

				// Logs para ver la aceleración cruda en la consola de Unreal
				UE_LOG(LogTemp, Warning, TEXT("Accel X: %f Y: %f Z: %f"), AccelX, AccelY, AccelZ);
			}
		}
	}
}