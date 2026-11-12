#include "Film/JamMiniJson.h"

namespace
{
	constexpr int32 MaxChars = 32768;
	constexpr int32 MaxDepth = 12;
	constexpr int32 MaxElements = 512;

	struct FReader
	{
		const FString& S;
		int32 Pos = 0;
		int32 Elements = 0;

		explicit FReader(const FString& InS) : S(InS) {}

		void SkipWs()
		{
			while (Pos < S.Len() && (S[Pos] == ' ' || S[Pos] == '\t' || S[Pos] == '\n' || S[Pos] == '\r')) { ++Pos; }
		}

		bool Eat(TCHAR C)
		{
			SkipWs();
			if (Pos < S.Len() && S[Pos] == C) { ++Pos; return true; }
			return false;
		}

		bool Literal(const TCHAR* Word)
		{
			const int32 N = FCString::Strlen(Word);
			if (Pos + N > S.Len() || FCString::Strncmp(*S + Pos, Word, N) != 0) { return false; }
			Pos += N;
			return true;
		}

		static int32 Hex(TCHAR C)
		{
			if (C >= '0' && C <= '9') { return C - '0'; }
			if (C >= 'a' && C <= 'f') { return 10 + C - 'a'; }
			if (C >= 'A' && C <= 'F') { return 10 + C - 'A'; }
			return -1;
		}

		bool ReadString(FString& Out)
		{
			if (Pos >= S.Len() || S[Pos] != '"') { return false; }
			++Pos;
			Out.Reset();
			while (Pos < S.Len())
			{
				const TCHAR C = S[Pos++];
				if (C == '"') { return true; }
				if (C < 0x20) { return false; } // raw control characters are not legal JSON
				if (C != '\\') { Out.AppendChar(C); continue; }
				if (Pos >= S.Len()) { return false; }
				const TCHAR E = S[Pos++];
				switch (E)
				{
				case '"': Out.AppendChar('"'); break;
				case '\\': Out.AppendChar('\\'); break;
				case '/': Out.AppendChar('/'); break;
				case 'b': Out.AppendChar('\b'); break;
				case 'f': Out.AppendChar('\f'); break;
				case 'n': Out.AppendChar('\n'); break;
				case 'r': Out.AppendChar('\r'); break;
				case 't': Out.AppendChar('\t'); break;
				case 'u':
				{
					if (Pos + 4 > S.Len()) { return false; }
					int32 V = 0;
					for (int32 i = 0; i < 4; ++i)
					{
						const int32 H = Hex(S[Pos++]);
						if (H < 0) { return false; }
						V = V * 16 + H;
					}
					Out.AppendChar((TCHAR)V); // surrogate pairs arrive as two escapes and join in the UTF-16 string
					break;
				}
				default: return false;
				}
			}
			return false; // unterminated
		}

		bool ReadNumber(double& Out)
		{
			const int32 Start = Pos;
			if (Pos < S.Len() && S[Pos] == '-') { ++Pos; }
			int32 Digits = 0;
			while (Pos < S.Len() && S[Pos] >= '0' && S[Pos] <= '9') { ++Pos; ++Digits; }
			if (Digits == 0) { return false; }
			if (Pos < S.Len() && S[Pos] == '.')
			{
				++Pos;
				int32 Frac = 0;
				while (Pos < S.Len() && S[Pos] >= '0' && S[Pos] <= '9') { ++Pos; ++Frac; }
				if (Frac == 0) { return false; }
			}
			if (Pos < S.Len() && (S[Pos] == 'e' || S[Pos] == 'E'))
			{
				++Pos;
				if (Pos < S.Len() && (S[Pos] == '+' || S[Pos] == '-')) { ++Pos; }
				int32 Exp = 0;
				while (Pos < S.Len() && S[Pos] >= '0' && S[Pos] <= '9') { ++Pos; ++Exp; }
				if (Exp == 0) { return false; }
			}
			Out = FCString::Atod(*S.Mid(Start, Pos - Start));
			return FMath::IsFinite(Out);
		}

		bool ReadValue(FJamJson& Out, int32 Depth)
		{
			if (Depth > MaxDepth || ++Elements > MaxElements) { return false; }
			SkipWs();
			if (Pos >= S.Len()) { return false; }
			const TCHAR C = S[Pos];
			if (C == '{')
			{
				++Pos;
				Out.Type = FJamJson::EType::Object;
				if (Eat('}')) { return true; }
				while (true)
				{
					SkipWs();
					FString Key;
					if (!ReadString(Key) || !Eat(':')) { return false; }
					FJamJson Child;
					if (!ReadValue(Child, Depth + 1)) { return false; }
					Out.Members.Emplace(MoveTemp(Key), MoveTemp(Child));
					if (Eat(',')) { continue; }
					return Eat('}');
				}
			}
			if (C == '[')
			{
				++Pos;
				Out.Type = FJamJson::EType::Array;
				if (Eat(']')) { return true; }
				while (true)
				{
					FJamJson Child;
					if (!ReadValue(Child, Depth + 1)) { return false; }
					Out.Array.Add(MoveTemp(Child));
					if (Eat(',')) { continue; }
					return Eat(']');
				}
			}
			if (C == '"') { Out.Type = FJamJson::EType::String; return ReadString(Out.String); }
			if (Literal(TEXT("true"))) { Out.Type = FJamJson::EType::Bool; Out.Bool = true; return true; }
			if (Literal(TEXT("false"))) { Out.Type = FJamJson::EType::Bool; Out.Bool = false; return true; }
			if (Literal(TEXT("null"))) { Out.Type = FJamJson::EType::Null; return true; }
			Out.Type = FJamJson::EType::Number;
			return ReadNumber(Out.Number);
		}
	};
}

bool FJamJson::Parse(const FString& Text, FJamJson& Out)
{
	Out = FJamJson();
	if (Text.Len() == 0 || Text.Len() > MaxChars) { return false; }
	FReader R(Text);
	FJamJson Value;
	if (!R.ReadValue(Value, 0)) { return false; }
	R.SkipWs();
	if (R.Pos != Text.Len()) { return false; } // trailing junk
	Out = MoveTemp(Value);
	return true;
}
