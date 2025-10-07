#include <common.hh>
#include <math2d.hh>

#include <cstdio>

struct input_reader
{
  key Offset;
  key Error;
};

enum input_reader_error
{
  INPUT_READER_ERROR_NONE = 0,
  INPUT_READER_ERROR_NUMBER = 1,
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

inline i32 AcceptDigit(const char *Input, input_reader *Reader)
{
  i32 StartIndex = Reader->Offset;

  if (!Accept(Input, Reader, '0', '9'))
  {
    Reader->Error = INPUT_READER_ERROR_NUMBER;
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

inline f32 AcceptFraction(const char *Input, input_reader *Reader)
{
  i32 StartIndex = Reader->Offset;

  if (!Accept(Input, Reader, '0', '9'))
  {
    Reader->Error = INPUT_READER_ERROR_NUMBER;
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

inline f32 AcceptNumber(const char *Input, input_reader *Reader)
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
    Number += f32(AcceptDigit(Input, Reader));
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_NUMBER;
    return 0.0f;
  }

  if (Accept(Input, Reader, '.'))
  {
    Number += AcceptFraction(Input, Reader);
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

    Number = Pow(Number, AcceptDigit(Input, Reader) * ExponentScalar);
  }

  return Number * Scalar;
}

inline bool32 AcceptWhitespace(const char *Input, input_reader *Reader)
{
  return Accept(Input, Reader, ' ') || Accept(Input, Reader, '\t') || Accept(Input, Reader, '\n') ||
         Accept(Input, Reader, '\r');
}

inline f32 AcceptValue(const char *Input, input_reader *Reader)
{
  while (AcceptWhitespace(Input, Reader))
  {
  }

  f32 Value = AcceptNumber(Input, Reader);

  while (AcceptWhitespace(Input, Reader))
  {
  }

  return Value;
}

i32 main(const i32 Argc, const char *Argv[])
{

  for (key Index = 1; Index < Argc; Index++)
  {
    input_reader Reader{.Offset = 0, .Error = 0};
    f32 Number = AcceptValue(Argv[Index], &Reader);

    if (Reader.Error == INPUT_READER_ERROR_NONE)
    {
      printf("Successfully parsed number: %f\n", Number);
    }
    else
    {
      printf("Failed to parse number.");
    }
  }

  return 0;
}
