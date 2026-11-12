#pragma once

#include "CoreMinimal.h"

/**
 * A tiny strict JSON reader for the note service's responses. Private to the Film code: it exists because RULES
 * does not own MegaJam.Build.cs (no "Json" module) and so the clamp stays headless. Hostile input is expected:
 * size, depth and element counts are capped, and any error returns false with nothing half-parsed.
 */
struct FJamJson
{
	enum class EType : uint8 { Null, Bool, Number, String, Array, Object };

	EType Type = EType::Null;
	bool Bool = false;
	double Number = 0.0;
	FString String;
	TArray<FJamJson> Array;
	TArray<TPair<FString, FJamJson>> Members;

	const FJamJson* Find(const TCHAR* Key) const
	{
		if (Type != EType::Object) { return nullptr; }
		for (const TPair<FString, FJamJson>& M : Members)
		{
			if (M.Key == Key) { return &M.Value; }
		}
		return nullptr;
	}

	static bool Parse(const FString& Text, FJamJson& Out);
};
