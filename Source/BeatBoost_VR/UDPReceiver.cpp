#include "UDPReceiver.h"

AUDPReceiver::AUDPReceiver()
{
    PrimaryActorTick.bCanEverTick = true;
    ListenSocket = nullptr;
}

// ─────────────────────────────────────────
//  BeginPlay / EndPlay
// ─────────────────────────────────────────

void AUDPReceiver::BeginPlay()
{
    Super::BeginPlay();

    FIPv4Endpoint Endpoint(FIPv4Address::Any, 8001);
    ListenSocket = FUdpSocketBuilder(TEXT("LectorUDP"))
        .AsNonBlocking()
        .AsReusable()
        .BoundToEndpoint(Endpoint)
        .WithReceiveBufferSize(2 * 1024 * 1024);

    if (ListenSocket)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UDP] Escuchando en puerto 8001"));
    }
    else
        UE_LOG(LogTemp, Error, TEXT("[UDP] ERROR: No se pudo abrir el socket"));
}

void AUDPReceiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }
}

// ─────────────────────────────────────────
//  Parsing del mensaje UDP
// ─────────────────────────────────────────

void AUDPReceiver::ParsePacket(const FString& Mensaje)
{
    TArray<FString> Partes;
    Mensaje.ParseIntoArray(Partes, TEXT(" "), true);
    if (Partes.Num() < 3) return;

    // Extrae el número después del prefijo "AX:", "RX:", etc.
    auto ExtractFloat = [](const FString& Part, const TCHAR* Prefix) -> float
        {
            FString Val = Part;
            Val.RemoveFromStart(Prefix);
            return FCString::Atof(*Val);
        };

    // Mano IZQUIERDA — prefijo AX / AY / AZ
    if (Partes[0].StartsWith(TEXT("AX:")))
    {
        AccelX = ExtractFloat(Partes[0], TEXT("AX:"));
        AccelY = ExtractFloat(Partes[1], TEXT("AY:"));
        AccelZ = ExtractFloat(Partes[2], TEXT("AZ:"));
    }
    // Mano DERECHA — prefijo RX / RY / RZ
    else if (Partes[0].StartsWith(TEXT("RX:")))
    {
        RightAccelX = ExtractFloat(Partes[0], TEXT("RX:"));
        RightAccelY = ExtractFloat(Partes[1], TEXT("RY:"));
        RightAccelZ = ExtractFloat(Partes[2], TEXT("RZ:"));
    }
}

FVector AUDPReceiver::MapearEjes(const float Sensor[3], int32 EjeH, int32 EjeV,
    bool bInvH, bool bInvV) const
{
    // Clampear índices para evitar out-of-bounds
    EjeH = FMath::Clamp(EjeH, 0, 2);
    EjeV = FMath::Clamp(EjeV, 0, 2);

    float Horizontal = ApplyDeadZone(Sensor[EjeH]) * (bInvH ? -1.0f : 1.0f);
    float Vertical = ApplyDeadZone(Sensor[EjeV]) * (bInvV ? -1.0f : 1.0f);

    // Y = horizontal (izq-der en Unreal), Z = vertical (arriba-abajo en Unreal)
    // X = profundidad — siempre 0, no queremos movimiento en profundidad
    return FVector(0.0f, Horizontal, Vertical);
}

// ─────────────────────────────────────────
//  Tick — leer UDP e integrar posición
// ─────────────────────────────────────────

void AUDPReceiver::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!ListenSocket) return;

    TSharedRef<FInternetAddr> Sender =
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    uint32 Size;

    // Vaciamos todo el buffer pendiente en este tick
    while (ListenSocket->HasPendingData(Size))
    {
        TArray<uint8> ReceivedData;
        ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
        int32 BytesRead = 0;
        ListenSocket->RecvFrom(ReceivedData.GetData(), ReceivedData.Num(), BytesRead, *Sender);

        if (BytesRead > 0)
        {
            ReceivedData.Add(0); // null-terminator para el string
            FString Mensaje = FString(ANSI_TO_TCHAR(
                reinterpret_cast<const char*>(ReceivedData.GetData())));
            ParsePacket(Mensaje);
        }
    }

    // ── Integrar mano IZQUIERDA ──
    {
        const float SensorLeft[3] = { AccelX, AccelY, AccelZ };
        FVector Delta = MapearEjes(SensorLeft,
            EjeHorizontal, EjeVertical,
            bInvertirHorizontal, bInvertirVertical);

        FVector TargetPos = LeftHandPos + Delta * SensorScale;
        TargetPos = TargetPos.GetClampedToSize(0.0f, MaxDisplacement);
        LeftHandPos = FMath::Lerp(LeftHandPos, TargetPos, SmoothFactor);
    }

    // ── Integrar mano DERECHA ──
    {
        const float SensorRight[3] = { RightAccelX, RightAccelY, RightAccelZ };
        FVector Delta = MapearEjes(SensorRight,
            RightEjeHorizontal, RightEjeVertical,
            bRightInvertirHorizontal, bRightInvertirVertical);

        FVector TargetPos = RightHandPos + Delta * SensorScale;
        TargetPos = TargetPos.GetClampedToSize(0.0f, MaxDisplacement);
        RightHandPos = FMath::Lerp(RightHandPos, TargetPos, SmoothFactor);
    }
}

// ─────────────────────────────────────────
//  Utilidades
// ─────────────────────────────────────────

void AUDPReceiver::ResetHandPositions()
{
    LeftHandPos = FVector::ZeroVector;
    RightHandPos = FVector::ZeroVector;
}

void AUDPReceiver::MandarVibracion(FString IPObjetivo, int32 Puerto, FString Mensaje)
{
    FSocket* SocketSender = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)
        ->CreateSocket(NAME_DGram, TEXT("UDPSender"), false);
    if (!SocketSender) return;

    FIPv4Address IP;
    FIPv4Address::Parse(IPObjetivo, IP);

    TSharedRef<FInternetAddr> Addr =
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    Addr->SetIp(IP.Value);
    Addr->SetPort(Puerto);

    int32 BytesSent = 0;
    FTCHARToUTF8 Convert(*Mensaje);
    SocketSender->SendTo(
        reinterpret_cast<const uint8*>(Convert.Get()),
        Convert.Length(), BytesSent, *Addr);

    SocketSender->Close();
    ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SocketSender);
}