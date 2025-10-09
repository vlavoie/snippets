/*
WIP json parser, this will leak a lot of memory do not use as is
Copyright (C) 2025  Vincent Lavoie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <common.hh>
#include <hash.hh>
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

union json_value;

struct json_array
{
  key Length;
  json_value *Values;
};

struct json_object;

union json_value {
  f32 Number;
  bool32 Boolean;
  bool32 Null;
  json_string String;
  json_array Array;
  json_object *Object;
};

struct json_object_slot
{
  hash::digest Hash;
  json_value Value;
};

struct json_object
{
  key Capacity;
  key Count;
  json_object_slot *Slots;
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

inline bool32 AnyExcept(const char *Input, input_reader *Reader, const char Exception)
{
  if (Input[Reader->Offset] != Exception)
  {
    Next(Input, Reader);
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

bool32 AcceptWhitespace(const char *Input, input_reader *Reader)
{
  bool32 Whitespace = false;
  while (Accept(Input, Reader, ' ') || Accept(Input, Reader, 0x0020) ||
         Accept(Input, Reader, 0x000A) || Accept(Input, Reader, 0x000D) ||
         Accept(Input, Reader, 0x0009))
  {
    Whitespace = true;
  }

  return Whitespace;
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

struct json_raw_string
{
  const key Length;
  const char *String;
};

json_raw_string AcceptString(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, '\"'))
  {
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return {.Length = 0, .String = 0x0};
  }

  const char *StringStart = Input + Reader->Offset;
  key Length = 0;

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
            return {.Length = 0, .String = 0x0};
          }
        }
      }
      else
      {
        Reader->Error = INPUT_READER_ERROR_PARSING;
        return {.Length = 0, .String = 0x0};
      }
    }
    else if (AnyExcept(Input, Reader, '\"'))
    {
      Length++;
    }
    else if (Accept(Input, Reader, '\0'))
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
      return {.Length = 0, .String = 0x0};
    }
  }

  return {.Length = Length, .String = StringStart};
}

json_string ParseString(const char *Input, input_reader *Reader)
{
  json_raw_string RawString = AcceptString(Input, Reader);

  if (Reader->Error != INPUT_READER_ERROR_NONE)
  {
    return {
        .Length = 0,
        .Buffer = 0x0,
    };
  }

  json_string Result;
  Result.Length = RawString.Length;
  Result.Buffer = SysAllocate(char, RawString.Length + 1);
  Result.Buffer[RawString.Length] = '\0';

  for (key CharIndex = 0; CharIndex < RawString.Length; CharIndex++)
  {
    Result.Buffer[CharIndex] = RawString.String[CharIndex];
  }

  return Result;
}

json_array ParseArray(const char *Input, input_reader *Reader);
json_object *ParseObject(const char *Input, input_reader *Reader);

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
  case '[':
    Value.Array = ParseArray(Input, Reader);
    break;
  case '{':
    Value.Object = ParseObject(Input, Reader);
    break;
  default:
    Reader->Error = INPUT_READER_ERROR_PARSING;
    break;
  }

  return Value;
}

json_value ParseValue(const char *Input, input_reader *Reader)
{
  AcceptWhitespace(Input, Reader);
  json_value Value = ParseTrimmedValue(Input, Reader);
  AcceptWhitespace(Input, Reader);

  return Value;
}

json_array ParseArray(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, '['))
  {
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return {
        .Length = 0,
        .Values = 0x0,
    };
  }

  AcceptWhitespace(Input, Reader);
  bool32 FirstLoop = true;

  json_array Result;
  Result.Length = 0;

  input_reader Backtrack;
  Backtrack.Offset = Reader->Offset;
  Backtrack.Error = Reader->Error;

  while (!Accept(Input, &Backtrack, ']'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(Input, &Backtrack, ','))
    {
    }
    else
    {
      Backtrack.Error = INPUT_READER_ERROR_PARSING;
      return {
          .Length = 0,
          .Values = 0x0,
      };
    }

    ParseValue(Input, &Backtrack);
    AcceptWhitespace(Input, &Backtrack);

    Result.Length++;
    FirstLoop = false;
  }

  Result.Values = SysAllocate(json_value, Result.Length);
  key Index = 0;
  FirstLoop = true;

  while (!Accept(Input, Reader, ']'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(Input, Reader, ','))
    {
    }
    else
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
    }

    Result.Values[Index] = ParseValue(Input, Reader);
    AcceptWhitespace(Input, Reader);

    Index++;
    FirstLoop = false;
  }

  return Result;
}

inline json_object_slot *GetSlot(const json_object *JsonObject, const hash::digest Hash)
{
  key Index = Hash % JsonObject->Capacity;
  key Attempts = 0;

  json_object_slot *Slot = &JsonObject->Slots[Index];

  while (Slot->Hash != Hash)
  {
    Assert(Attempts < JsonObject->Capacity, "Could not acquire slot in JsonObject hashmap.");
    Attempts++;
    Index = (Index + 1) % JsonObject->Capacity;
    Slot = &JsonObject->Slots[Index];
  }

  return Slot;
}

inline json_object_slot *AssignEmptySlot(const json_object *JsonObject,
                                         const json_raw_string *RawString)
{
  hash::digest Hash = hash::Mix(RawString->String, RawString->Length);
  json_object_slot *Slot = GetSlot(JsonObject, 0);
  Slot->Hash = Hash;

  return Slot;
}

inline json_object_slot *GetSlot(const json_object *JsonObject, const char *StringKey)
{
  hash::digest Hash = hash::Mix(StringKey);
  return GetSlot(JsonObject, Hash);
}

json_object *ParseObject(const char *Input, input_reader *Reader)
{
  if (Accept(Input, Reader, '{'))
  {
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0x0;
  }

  input_reader Backtrack;
  Backtrack.Offset = Reader->Offset;
  Backtrack.Error = Reader->Error;

  bool32 FirstLoop = true;
  key ObjectCount = 0;

  while (!Accept(Input, &Backtrack, '}'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(Input, &Backtrack, ','))
    {
    }
    else
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
      return 0x0;
    }

    AcceptWhitespace(Input, &Backtrack);
    AcceptString(Input, &Backtrack);
    AcceptWhitespace(Input, &Backtrack);

    Accept(Input, &Backtrack, ':');
    ParseValue(Input, &Backtrack);
    ObjectCount++;
    FirstLoop = false;
  }

  if (Backtrack.Error != INPUT_READER_ERROR_NONE)
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0x0;
  }

  FirstLoop = true;
  json_object *Result = SysAllocate(json_object, 1);
  Result->Count = ObjectCount;
  Result->Capacity = key(f32(ObjectCount) * 1.5f); // Reduce risk of collisions
  Result->Slots = SysAllocate(json_object_slot, Result->Capacity);

  while (!Accept(Input, Reader, '}'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(Input, Reader, ','))
    {
    }
    else
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
    }

    AcceptWhitespace(Input, Reader);
    json_raw_string RawString = AcceptString(Input, Reader);
    AcceptWhitespace(Input, Reader);

    Accept(Input, Reader, ':');
    json_object_slot *Slot = AssignEmptySlot(Result, &RawString);
    Slot->Value = ParseValue(Input, Reader);

    if (Reader->Error != INPUT_READER_ERROR_NONE)
    {
      return Result;
    }

    FirstLoop = false;
  }

  return Result;
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

  Reader = {.Offset = 0, .Error = 0};

  json_array Array = ParseValue("[123, [], \"A string\", true, null]", &Reader).Array;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    printf("Successfully parsed array: %f, %s\n", Array.Values[0].Number,
           Array.Values[2].String.Buffer);
  }
  else
  {
    printf("Failed to parse array.\n");
  }

  Reader = {.Offset = 0, .Error = 0};

  json_object *Object = ParseValue("{ \"First\": [123, 1.45], \"Another\": false, \"A number\" : "
                                   "456, \"SubObject\": { \"A property\": \"substring\"}}",
                                   &Reader)
                            .Object;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    json_object *SubObject = GetSlot(Object, "SubObject")->Value.Object;
    f32 ObjectNumber = GetSlot(Object, "A number")->Value.Number;
    json_string ObjectString = GetSlot(SubObject, "A property")->Value.String;

    printf("Successfully parsed object: %f, %s\n", ObjectNumber, ObjectString.Buffer);
  }
  else
  {
    printf("Failed to parse object.\n");
  }

  return 0;
}
