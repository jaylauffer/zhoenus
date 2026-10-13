// Copyright 2026 Jay Lauffer
#pragma once

#include "CoreMinimal.h"

/**
 * Calls loadngo's gpt_oss_serve on this machine: one JSON line per call over its Unix
 * socket (~/.loadngo/node/gpt-oss.sock), no token, no HTTP. See loadngo
 * docs/GPT_OSS.md, "Serving". Mac and Linux only; elsewhere every call is "unsupported".
 */
namespace ZhoenusModelCall
{
	struct FResult
	{
		/** "ok", "expired", "busy" or "error" from the server; "unreachable" or
		 * "unsupported" when no call was made. */
		FString Status;
		/** The answer when Status is "ok"; otherwise why not. */
		FString Text;
		double Seconds = 0.0;

		bool IsOk() const { return Status == TEXT("ok"); }
	};

	/** ~/.loadngo/node/gpt-oss.sock */
	FString DefaultSocketPath();

	/**
	 * Asks the model. Blocks until the reply or the deadline (plus two seconds for the
	 * reply to travel), so call it off the game thread.
	 */
	FResult Ask(const FString& SocketPath, const FString& Instructions, const FString& Message, int32 DeadlineMs);
}
