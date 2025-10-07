#include <common.hh>
#include <math2d.hh>

#include <cstdio>

struct input_reader
{
  key Offset;
  key Error;
};

union json_value {
  f32 Number;
  bool32 Boolean;
  bool32 Null;
};

enum input_reader_error
{
  INPUT_READER_ERROR_NONE = 0,
  INPUT_READER_ERROR_PARSING = 1,
};

inline bool32 Accept(const char *Input, input_reader *Reader, const char Expected)
{
  if (Input[Reader->Offset] == Expected)
  {
    Reader->Offset++;
    return true;
  }

  return false;
}

inline bool32 Accept(const char *Input, input_reader *Reader, const char Lower, const char Upper)
{
  char Value = Input[Reader->Offset];

  if (Value >= Lower && Value <= Upper)
  {
    Reader->Offset++;
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

inline bool32 ParseWhitespace(const char *Input, input_reader *Reader)
{
  return Accept(Input, Reader, ' ') || Accept(Input, Reader, 0x0020) ||
         Accept(Input, Reader, 0x000A) || Accept(Input, Reader, 0x000D) ||
         Accept(Input, Reader, 0x0009);
}

inline bool32 ParseBoolean(const char *Input, input_reader *Reader)
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

inline bool32 ParseNull(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, 'n') && Accept(Input, Reader, 'u') && Accept(Input, Reader, 'l') &&
      Accept(Input, Reader, 'l'))
  {
    return 1;
  }

  Reader->Error = INPUT_READER_ERROR_PARSING;
  return 0;
}

inline json_value ParseTrimmedValue(const char *Input, input_reader *Reader)
{
  json_value Value;
  input_reader Backtrack;

  Backtrack.Offset = Reader->Offset;
  Backtrack.Error = INPUT_READER_ERROR_NONE;

  Value.Number = ParseNumber(Input, &Backtrack);

  if (Backtrack.Error == INPUT_READER_ERROR_PARSING)
  {
    Backtrack.Offset = Reader->Offset;
    Backtrack.Error = INPUT_READER_ERROR_NONE;
  }
  else
  {
    return Value;
  }

  Value.Boolean = ParseBoolean(Input, &Backtrack);

  if (Backtrack.Error == INPUT_READER_ERROR_PARSING)
  {
    Backtrack.Offset = Reader->Offset;
    Backtrack.Error = INPUT_READER_ERROR_NONE;
  }
  else
  {
    return Value;
  }

  Value.Null = ParseNull(Input, &Backtrack);
  Reader->Offset = Backtrack.Offset;
  Reader->Error = Backtrack.Error;

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

  bool32 Boolean = ParseValue(Argv[1], &Reader).Boolean;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed Boolean: %d (true)\n", Boolean);
  }
  else
  {
    printf("Failed to parse boolean.");
  }

  Reader = {.Offset = 0, .Error = 0};

  f32 Number = ParseValue(Argv[2], &Reader).Number;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed number: %f\n", Number);
  }
  else
  {
    printf("Failed to parse number.");
  }

  Reader = {.Offset = 0, .Error = 0};

  Boolean = ParseValue(Argv[3], &Reader).Boolean;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed Boolean: %d (false)\n", Boolean);
  }
  else
  {
    printf("Failed to parse Boolean.");
  }

  Reader = {.Offset = 0, .Error = 0};

  bool32 Null = ParseValue(Argv[4], &Reader).Null;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed Null\n");
  }
  else
  {
    printf("Failed to parse Null.");
  }
  return 0;
}
