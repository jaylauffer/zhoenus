// Copyright 2026 Jay Lauffer

#include "ZhoenusModelCall.h"

#include "Dom/JsonObject.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#if PLATFORM_MAC || PLATFORM_LINUX
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace ZhoenusModelCall
{
	FString DefaultSocketPath()
	{
		const FString Home = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));
		return FPaths::Combine(Home, TEXT(".loadngo/node/gpt-oss.sock"));
	}

	namespace
	{
		FResult Failed(const TCHAR* Status, const FString& Why, double Started)
		{
			FResult Result;
			Result.Status = Status;
			Result.Text = Why;
			Result.Seconds = FPlatformTime::Seconds() - Started;
			return Result;
		}

		/** The call as one JSON line. */
		FString CallLine(const FString& Instructions, const FString& Message, int32 DeadlineMs)
		{
			TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
			if (!Instructions.IsEmpty())
			{
				Body->SetStringField(TEXT("instructions"), Instructions);
			}
			Body->SetStringField(TEXT("message"), Message);
			TSharedRef<FJsonObject> Call = MakeShared<FJsonObject>();
			Call->SetNumberField(TEXT("call_id"), 1);
			Call->SetStringField(TEXT("capability"), TEXT("chat"));
			Call->SetNumberField(TEXT("deadline_ms"), DeadlineMs);
			Call->SetObjectField(TEXT("body"), Body);
			FString Line;
			TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
				TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Line);
			FJsonSerializer::Serialize(Call, Writer);
			return Line + TEXT("\n");
		}
	}

	FResult Ask(const FString& SocketPath, const FString& Instructions, const FString& Message, int32 DeadlineMs)
	{
		const double Started = FPlatformTime::Seconds();
#if PLATFORM_MAC || PLATFORM_LINUX
		const FTCHARToUTF8 Path(*SocketPath);
		sockaddr_un Address = {};
		if (Path.Length() + 1 > static_cast<int32>(sizeof(Address.sun_path)))
		{
			return Failed(TEXT("unreachable"), TEXT("the socket path is too long"), Started);
		}
		Address.sun_family = AF_UNIX;
		FMemory::Memcpy(Address.sun_path, Path.Get(), Path.Length());

		const int Socket = socket(AF_UNIX, SOCK_STREAM, 0);
		if (Socket < 0)
		{
			return Failed(TEXT("unreachable"), FString::Printf(TEXT("socket failed (errno %d)"), errno), Started);
		}
		// The reply may take up to the deadline; the socket waits two seconds more.
		const int32 WaitMs = FMath::Max(DeadlineMs, 0) + 2000;
		timeval Wait;
		Wait.tv_sec = WaitMs / 1000;
		Wait.tv_usec = (WaitMs % 1000) * 1000;
		setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, &Wait, sizeof(Wait));
		setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, &Wait, sizeof(Wait));
#if PLATFORM_MAC
		// A server that hangs up must not raise SIGPIPE in the game.
		const int NoSigPipe = 1;
		setsockopt(Socket, SOL_SOCKET, SO_NOSIGPIPE, &NoSigPipe, sizeof(NoSigPipe));
		const int SendFlags = 0;
#else
		const int SendFlags = MSG_NOSIGNAL;
#endif
		if (connect(Socket, reinterpret_cast<const sockaddr*>(&Address), sizeof(Address)) != 0)
		{
			const int Error = errno;
			close(Socket);
			return Failed(TEXT("unreachable"), FString::Printf(TEXT("no model at %s (errno %d)"), *SocketPath, Error), Started);
		}

		const FTCHARToUTF8 Line(*CallLine(Instructions, Message, DeadlineMs));
		int32 Sent = 0;
		while (Sent < Line.Length())
		{
			const ssize_t Count = send(Socket, Line.Get() + Sent, Line.Length() - Sent, SendFlags);
			if (Count <= 0)
			{
				close(Socket);
				return Failed(TEXT("unreachable"), TEXT("the model hung up before the call was sent"), Started);
			}
			Sent += static_cast<int32>(Count);
		}

		TArray<ANSICHAR> Reply;
		ANSICHAR Buffer[4096];
		while (!Reply.Contains('\n'))
		{
			const ssize_t Count = recv(Socket, Buffer, sizeof(Buffer), 0);
			if (Count <= 0)
			{
				break;
			}
			Reply.Append(Buffer, static_cast<int32>(Count));
		}
		close(Socket);
		if (!Reply.Contains('\n'))
		{
			return Failed(TEXT("expired"), TEXT("no reply before the deadline"), Started);
		}
		Reply.Add('\0');

		TSharedPtr<FJsonObject> Parsed;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(UTF8_TO_TCHAR(Reply.GetData()));
		if (!FJsonSerializer::Deserialize(Reader, Parsed) || !Parsed.IsValid())
		{
			return Failed(TEXT("error"), TEXT("the reply is not JSON"), Started);
		}
		FResult Result;
		Result.Status = Parsed->GetStringField(TEXT("status"));
		const TSharedPtr<FJsonObject>* Body = nullptr;
		if (Parsed->TryGetObjectField(TEXT("body"), Body) && Body != nullptr && Body->IsValid())
		{
			FString Text;
			if ((*Body)->TryGetStringField(Result.IsOk() ? TEXT("text") : TEXT("error"), Text))
			{
				Result.Text = Text;
			}
		}
		Result.Seconds = FPlatformTime::Seconds() - Started;
		return Result;
#else
		return Failed(TEXT("unsupported"), TEXT("no Unix sockets on this platform"), Started);
#endif
	}
}
