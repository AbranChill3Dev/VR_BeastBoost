#include "UDPReceiver.h"

AUDPReceiver::AUDPReceiver()
{
	PrimaryActorTick.bCanEverTick = true;
	ListenSocket = nullptr;

	// Iniciamos las variables de la mano IZQUIERDA en 0
	AccelX = 0.0f;
	AccelY = 0.0f;
	AccelZ = 0.0f;

	// Iniciamos las variables de la mano DERECHA en 0
	RightAccelX = 0.0f;
	RightAccelY = 0.0f;
	RightAccelZ = 0.0f;
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
				// 1. ¿Es la placa de la mano IZQUIERDA? (Empieza con AX)
				if (Partes[0].Contains(TEXT("AX:")))
				{
					AccelX = FCString::Atof(*Partes[0].Replace(TEXT("AX:"), TEXT("")));
					AccelY = FCString::Atof(*Partes[1].Replace(TEXT("AY:"), TEXT("")));
					AccelZ = FCString::Atof(*Partes[2].Replace(TEXT("AZ:"), TEXT("")));
					
					// Descomentar si ocupan ver los logs en Unreal
					// UE_LOG(LogTemp, Warning, TEXT("IZQ - X: %f Y: %f Z: %f"), AccelX, AccelY, AccelZ);
				}
				// 2. ¿Es la placa de la mano DERECHA? (Empieza con RX)
				else if (Partes[0].Contains(TEXT("RX:")))
				{
					RightAccelX = FCString::Atof(*Partes[0].Replace(TEXT("RX:"), TEXT("")));
					RightAccelY = FCString::Atof(*Partes[1].Replace(TEXT("RY:"), TEXT("")));
					RightAccelZ = FCString::Atof(*Partes[2].Replace(TEXT("RZ:"), TEXT("")));
					
					// Descomentar si ocupan ver los logs en Unreal
					// UE_LOG(LogTemp, Warning, TEXT("DER - X: %f Y: %f Z: %f"), RightAccelX, RightAccelY, RightAccelZ);
				}
			}
		}
	}
}

void AUDPReceiver::MandarVibracion(FString IPObjetivo, int32 Puerto, FString Mensaje)
{
    // Crear un socket temporal para enviar el mensaje
    FSocket* SocketSender = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_DGram, TEXT("UDPSender"), false);
    if (!SocketSender) return;

    // Convertir la IP de texto a formato de red
    FIPv4Address IP;
    FIPv4Address::Parse(IPObjetivo, IP);
    TSharedRef<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    Addr->SetIp(IP.Value);
    Addr->SetPort(Puerto);

    // Convertir el texto a bytes y enviarlo
    int32 BytesSent = 0;
    FTCHARToUTF8 Convert(*Mensaje);
    SocketSender->SendTo((uint8*)Convert.Get(), Convert.Length(), BytesSent, *Addr);

    // Limpiar la memoria cerrando el socket
    SocketSender->Close();
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SocketSender);
}