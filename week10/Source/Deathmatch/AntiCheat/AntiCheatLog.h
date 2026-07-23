// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 *  Dedicated anti-cheat log category.
 *
 *  Rationale (see Week 10 audit): routing every rejection through LogTemp/Warning floods the
 *  server log and makes real problems hard to spot. A dedicated category can be filtered
 *  ("Log LogAntiCheat Verbose", "Log LogAntiCheat off") and its verbosity tuned independently
 *  of the rest of the game. Rejections are logged at Warning so they stand out in a capture;
 *  routine/expected drops can be lowered to Verbose per deployment.
 */
DECLARE_LOG_CATEGORY_EXTERN(LogAntiCheat, Log, All);

/**
 *  Emit a single, structured server-side rejection record.
 *
 *  Every rejection carries the same four fields the audit asks for:
 *    - timestamp        (UTC wall clock + authoritative server world-seconds)
 *    - player identifier (name + PlayerId, resolved from whatever actor we are handed)
 *    - action name      (e.g. "Fire", "Reload", "Move")
 *    - reason           (human-readable cause)
 *
 *  Offender may be a Controller, a Pawn, or a PlayerState; we resolve a stable identity from
 *  any of them. Server-only by contract — callers must already hold authority.
 */
void LogAntiCheatRejection(const UObject* WorldContext, const AActor* Offender, const FName Action, const FString& Reason);
