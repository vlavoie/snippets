#include <common.hh>
#include <math2d.hh>

#include <cstdio>

struct input_reader
{
  key Offset;
  key Error;
};

struct json_string
{
  key Length;
  char *Buffer;
};

union json_value {
  f32 Number;
  bool32 Boolean;
  bool32 Null;
  json_string String;
};

#define JSON_NULL_VALUE INT32_MAX

enum input_reader_error
{
  INPUT_READER_ERROR_NONE = 0,
  INPUT_READER_ERROR_PARSING = 1,
};

inline char Next(const char *Input, input_reader *Reader)
{
  return Input[Reader->Offset++];
}

inline bool32 Accept(const char *Input, input_reader *Reader, const char Expected)
{
  if (Input[Reader->Offset] == Expected)
  {
    Next(Input, Reader);
    return true;
  }

  return false;
}

inline bool32 Accept(const char *Input, input_reader *Reader, const char Lower, const char Upper)
{
  char Value = Input[Reader->Offset];

  if (Value >= Lower && Value <= Upper)
  {
    Next(Input, Reader);
    return true;
  }

  return false;
}

inline bool32 Expect(const char *Input, input_reader *Reader, const char Expected)
{
  if (Input[Reader->Offset] == Expected)
  {
    return true;
  }

  return false;
}

inline bool32 Expect(const char *Input, input_reader *Reader, const char Lower, const char Upper)
{
  char Value = Input[Reader->Offset];

  if (Value >= Lower && Value <= Upper)
  {
    return true;
  }

  return false;
}

inline char Peek(const char *Input, input_reader *Reader)
{
  return Input[Reader->Offset];
}

i32 ParseDigit(const char *Input, input_reader *Reader)
{
  i32 StartIndex = Reader->Offset;

  if (!Accept(Input, Reader, '0', '9'))
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0;
  }

  while (Accept(Input, Reader, '0', '9'))
  {
  }

  i32 Digit = 0;
  i32 Scalar = 1;

  for (i32 Index = Reader->Offset - 1; Index >= StartIndex; Index--)
  {
    Digit += (Input[Index] - '0') * Scalar;
    Scalar *= 10;
  }

  return Digit;
}

f32 ParseFraction(const char *Input, input_reader *Reader)
{
  i32 StartIndex = Reader->Offset;

  if (!Accept(Input, Reader, '0', '9'))
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0;
  }

  while (Accept(Input, Reader, '0', '9'))
  {
  }

  f32 Fraction = 0;
  f32 Scalar = 0.1f;

  for (i32 Index = StartIndex; Index < Reader->Offset; Index++)
  {
    Fraction += f32(Input[Index] - '0') * Scalar;
    Scalar *= 0.1f;
  }

  return Fraction;
}

f32 ParseNumber(const char *Input, input_reader *Reader)
{
  f32 Number = 0.0f;
  f32 Scalar = 1.0f;

  if (Accept(Input, Reader, '-'))
  {
    Scalar *= -1.0f;
  }

  if (Accept(Input, Reader, '0'))
  {
  }
  else if (Expect(Input, Reader, '1', '9'))
  {
    Number += f32(ParseDigit(Input, Reader));
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0.0f;
  }

  if (Accept(Input, Reader, '.'))
  {
    Number += ParseFraction(Input, Reader);
  }

  if (Accept(Input, Reader, 'e') || Accept(Input, Reader, 'E'))
  {
    f32 ExponentScalar = 1.0f;

    if (Accept(Input, Reader, '-'))
    {
      ExponentScalar *= -1.0f;
    }
    else if (Accept(Input, Reader, '+'))
    {
    }

    Number = Pow(Number, ParseDigit(Input, Reader) * ExponentScalar);
  }

  return Number * Scalar;
}

bool32 ParseWhitespace(const char *Input, input_reader *Reader)
{
  return Accept(Input, Reader, ' ') || Accept(Input, Reader, 0x0020) ||
         Accept(Input, Reader, 0x000A) || Accept(Input, Reader, 0x000D) ||
         Accept(Input, Reader, 0x0009);
}

bool32 ParseBoolean(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, 't') && Accept(Input, Reader, 'r') && Accept(Input, Reader, 'u') &&
      Accept(Input, Reader, 'e'))
  {
    return true;
  }
  else if (Accept(Input, Reader, 'f') && Accept(Input, Reader, 'a') && Accept(Input, Reader, 'l') &&
           Accept(Input, Reader, 's') && Accept(Input, Reader, 'e'))
  {
    return false;
  }

  Reader->Error = INPUT_READER_ERROR_PARSING;
  return false;
}

bool32 ParseNull(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, 'n') && Accept(Input, Reader, 'u') && Accept(Input, Reader, 'l') &&
      Accept(Input, Reader, 'l'))
  {
    return JSON_NULL_VALUE;
  }

  Reader->Error = INPUT_READER_ERROR_PARSING;
  return 0;
}

inline bool32 AcceptHex(const char *Input, input_reader *Reader)
{
  char Head = Input[Reader->Offset];

  if ((Head >= '0' && Head <= '9') || (Head >= 'a' && Head <= 'f') || (Head >= 'A' && Head <= 'F'))
  {
    Next(Input, Reader);
    return true;
  }

  return false;
}

json_string ParseString(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, '\"'))
  {
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return {.Length = 0, .Buffer = 0x0};
  }

  key Length = 0;
  key StartIndex = Reader->Offset;

  while (!Accept(Input, Reader, '\"'))
  {
    if (Accept(Input, Reader, '\\'))
    {
      Length++;

      if (Expect(Input, Reader, '\"') || Expect(Input, Reader, '\\') ||
          Expect(Input, Reader, '/') || Expect(Input, Reader, 'b') || Expect(Input, Reader, 'f') ||
          Expect(Input, Reader, 'n') || Expect(Input, Reader, 'r') || Expect(Input, Reader, 't'))
      {
        Next(Input, Reader);
        Length++;
      }
      else if (Accept(Input, Reader, 'u'))
      {
        Length++;

        for (key Index = 0; Index < 4; Index++)
        {
          if (AcceptHex(Input, Reader))
          {
            Length++;
          }
          else
          {
            Reader->Error = INPUT_READER_ERROR_PARSING;
            return {.Length = 0, .Buffer = 0x0};
          }
        }
      }
      else
      {
        Reader->Error = INPUT_READER_ERROR_PARSING;
        return {.Length = 0, .Buffer = 0x0};
      }
    }
    else if (!Accept(Input, Reader, '\"'))
    {
      Next(Input, Reader);
      Length++;
    }
    else if (Accept(Input, Reader, '\0'))
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
      return {.Length = 0, .Buffer = 0x0};
    }
  }

  json_string Result;
  Result.Length = Length;
  Result.Buffer = SysAllocate(char, Length);

  for (key CharIndex = 0; CharIndex < Length; CharIndex++)
  {
    Result.Buffer[CharIndex] = Input[StartIndex + CharIndex];
  }

  return Result;
}

json_value ParseTrimmedValue(const char *Input, input_reader *Reader)
{
  json_value Value;

  char Match = Input[Reader->Offset];

  switch (Match)
  {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '-':
    Value.Number = ParseNumber(Input, Reader);
    break;
  case 'n':
    Value.Null = ParseNull(Input, Reader);
    break;
  case 't':
  case 'f':
    Value.Boolean = ParseBoolean(Input, Reader);
    break;
  case '\"':
    Value.String = ParseString(Input, Reader);
    break;
  default:
    Reader->Error = INPUT_READER_ERROR_PARSING;
    break;
  }

  return Value;
}

json_value ParseValue(const char *Input, input_reader *Reader)
{
  while (ParseWhitespace(Input, Reader))
  {
  }

  json_value Value = ParseTrimmedValue(Input, Reader);

  while (ParseWhitespace(Input, Reader))
  {
  }

  return Value;
}

i32 main(const i32 Argc, const char *Argv[])
{
  input_reader Reader = {.Offset = 0, .Error = 0};

  bool32 Boolean = ParseValue("\r\t     true", &Reader).Boolean;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed Boolean: %d (true)\n", Boolean);
  }
  else
  {
    printf("Failed to parse boolean.\n");
  }

  Reader = {.Offset = 0, .Error = 0};

  f32 Number = ParseValue("-3.14159e2", &Reader).Number;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed number: %f\n", Number);
  }
  else
  {
    printf("Failed to parse number.\n");
  }

  Reader = {.Offset = 0, .Error = 0};

  Boolean = ParseValue("false", &Reader).Boolean;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed Boolean: %d (false)\n", Boolean);
  }
  else
  {
    printf("Failed to parse Boolean.\n");
  }

  Reader = {.Offset = 0, .Error = 0};

  ParseValue("\rnull", &Reader);

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed Null\n");
  }
  else
  {
    printf("Failed to parse Null.\n");
  }

  Reader = {.Offset = 0, .Error = 0};

  json_string String = ParseValue("\"A \\u2346\\bstring\"", &Reader).String;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed string: %s\n", String.Buffer);
  }
  else
  {
    printf("Failed to parse string.\n");
  }
  return 0;
}
