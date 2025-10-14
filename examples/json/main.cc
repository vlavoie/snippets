/*
WIP json parser, do not use as is
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
#include <allocators/bump.hh>

#include <cstdio>

struct json_raw_string
{
  const key Length;
  const char *String;
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

struct input_reader
{
  key Offset;
  key Error;
  const char *Input;
};

enum json_value_type
{
  JSON_VALUE_TYPE_INVALID = 0,
  JSON_VALUE_TYPE_NUMBER = 1,
  JSON_VALUE_TYPE_BOOLEAN = 2,
  JSON_VALUE_TYPE_NULL = 3,
  JSON_VALUE_TYPE_STRING = 4,
  JSON_VALUE_TYPE_ARRAY = 5,
  JSON_VALUE_TYPE_OBJECT = 6,
};

struct json_value_header
{
  key Type;
  json_value Value;
};

#define JSON_NULL_VALUE INT32_MAX
#define JSON_ALLOCATION_SIZE (MEGABYTE)

enum input_reader_error
{
  INPUT_READER_ERROR_NONE = 0,
  INPUT_READER_ERROR_PARSING = 0b1,
  INPUT_READER_ERROR_ALLOCATION = 0b10,
};

inline char Next(input_reader *Reader)
{
  return Reader->Input[Reader->Offset++];
}

inline bool32 Accept(input_reader *Reader, const char Expected)
{
  if (Reader->Input[Reader->Offset] == Expected)
  {
    Next(Reader);
    return true;
  }

  return false;
}

inline bool32 Accept(input_reader *Reader, const char Lower, const char Upper)
{
  char Value = Reader->Input[Reader->Offset];

  if (Value >= Lower && Value <= Upper)
  {
    Next(Reader);
    return true;
  }

  return false;
}

inline bool32 Expect(input_reader *Reader, const char Expected)
{
  if (Reader->Input[Reader->Offset] == Expected)
  {
    return true;
  }

  return false;
}

inline bool32 Expect(input_reader *Reader, const char Lower, const char Upper)
{
  char Value = Reader->Input[Reader->Offset];

  if (Value >= Lower && Value <= Upper)
  {
    return true;
  }

  return false;
}

inline bool32 AnyExcept(input_reader *Reader, const char Exception)
{
  if (Reader->Input[Reader->Offset] != Exception)
  {
    Next(Reader);
    return true;
  }

  return false;
}

inline char Peek(input_reader *Reader)
{
  return Reader->Input[Reader->Offset];
}

i32 ParseDigit(input_reader *Reader)
{
  i32 StartIndex = Reader->Offset;

  if (!Accept(Reader, '0', '9'))
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0;
  }

  while (Accept(Reader, '0', '9'))
  {
  }

  i32 Digit = 0;
  i32 Scalar = 1;

  for (i32 Index = Reader->Offset - 1; Index >= StartIndex; Index--)
  {
    Digit += (Reader->Input[Index] - '0') * Scalar;
    Scalar *= 10;
  }

  return Digit;
}

f32 ParseFraction(input_reader *Reader)
{
  i32 StartIndex = Reader->Offset;

  if (!Accept(Reader, '0', '9'))
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0;
  }

  while (Accept(Reader, '0', '9'))
  {
  }

  f32 Fraction = 0;
  f32 Scalar = 0.1f;

  for (i32 Index = StartIndex; Index < Reader->Offset; Index++)
  {
    Fraction += f32(Reader->Input[Index] - '0') * Scalar;
    Scalar *= 0.1f;
  }

  return Fraction;
}

f32 ParseNumber(input_reader *Reader)
{
  f32 Number = 0.0f;
  f32 Scalar = 1.0f;

  if (Accept(Reader, '-'))
  {
    Scalar *= -1.0f;
  }

  if (Accept(Reader, '0'))
  {
  }
  else if (Expect(Reader, '1', '9'))
  {
    Number += f32(ParseDigit(Reader));
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return 0.0f;
  }

  if (Accept(Reader, '.'))
  {
    Number += ParseFraction(Reader);
  }

  if (Accept(Reader, 'e') || Accept(Reader, 'E'))
  {
    f32 ExponentScalar = 1.0f;

    if (Accept(Reader, '-'))
    {
      ExponentScalar *= -1.0f;
    }
    else if (Accept(Reader, '+'))
    {
    }

    Number = Pow(Number, ParseDigit(Reader) * ExponentScalar);
  }

  return Number * Scalar;
}

bool32 AcceptWhitespace(input_reader *Reader)
{
  bool32 Whitespace = false;
  while (Accept(Reader, ' ') || Accept(Reader, 0x0020) || Accept(Reader, 0x000A) ||
         Accept(Reader, 0x000D) || Accept(Reader, 0x0009))
  {
    Whitespace = true;
  }

  return Whitespace;
}

bool32 ParseBoolean(input_reader *Reader)
{
  if (Accept(Reader, 't') && Accept(Reader, 'r') && Accept(Reader, 'u') && Accept(Reader, 'e'))
  {
    return true;
  }
  else if (Accept(Reader, 'f') && Accept(Reader, 'a') && Accept(Reader, 'l') &&
           Accept(Reader, 's') && Accept(Reader, 'e'))
  {
    return false;
  }

  Reader->Error = INPUT_READER_ERROR_PARSING;
  return false;
}

bool32 ParseNull(input_reader *Reader)
{
  if (Accept(Reader, 'n') && Accept(Reader, 'u') && Accept(Reader, 'l') && Accept(Reader, 'l'))
  {
    return JSON_NULL_VALUE;
  }

  Reader->Error = INPUT_READER_ERROR_PARSING;
  return 0;
}

inline bool32 AcceptHex(input_reader *Reader)
{
  char Head = Reader->Input[Reader->Offset];

  if ((Head >= '0' && Head <= '9') || (Head >= 'a' && Head <= 'f') || (Head >= 'A' && Head <= 'F'))
  {
    Next(Reader);
    return true;
  }

  return false;
}

json_raw_string AcceptString(input_reader *Reader)
{
  if (Accept(Reader, '\"'))
  {
  }
  else
  {
    Reader->Error = INPUT_READER_ERROR_PARSING;
    return {.Length = 0, .String = 0x0};
  }

  const char *StringStart = Reader->Input + Reader->Offset;
  key Length = 0;

  while (!Accept(Reader, '\"'))
  {
    if (Accept(Reader, '\\'))
    {
      Length++;

      if (Expect(Reader, '\"') || Expect(Reader, '\\') || Expect(Reader, '/') ||
          Expect(Reader, 'b') || Expect(Reader, 'f') || Expect(Reader, 'n') ||
          Expect(Reader, 'r') || Expect(Reader, 't'))
      {
        Next(Reader);
        Length++;
      }
      else if (Accept(Reader, 'u'))
      {
        Length++;

        for (key Index = 0; Index < 4; Index++)
        {
          if (AcceptHex(Reader))
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
    else if (AnyExcept(Reader, '\"'))
    {
      Length++;
    }
    else if (Accept(Reader, '\0'))
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
      return {.Length = 0, .String = 0x0};
    }
  }

  return {.Length = Length, .String = StringStart};
}

json_string ParseString(allocator::bump *Allocator, input_reader *Reader)
{
  json_raw_string RawString = AcceptString(Reader);

  if (Reader->Error != INPUT_READER_ERROR_NONE)
  {
    return {
        .Length = 0,
        .Buffer = 0x0,
    };
  }

  json_string Result;
  Result.Length = RawString.Length;
  Result.Buffer = AllocateN(Allocator, char, RawString.Length + 1);

  if (Result.Buffer)
  {
    Result.Buffer[RawString.Length] = '\0';

    for (key CharIndex = 0; CharIndex < RawString.Length; CharIndex++)
    {
      Result.Buffer[CharIndex] = RawString.String[CharIndex];
    }
  }
  else
  {
    SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
  }

  return Result;
}

json_array ParseArray(allocator::bump *Allocator, input_reader *Reader);
json_object *ParseObject(allocator::bump *Allocator, input_reader *Reader);

json_value ParseTrimmedValue(allocator::bump *Allocator, input_reader *Reader)
{
  json_value Value;

  char Match = Reader->Input[Reader->Offset];

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
    Value.Number = ParseNumber(Reader);
    break;
  case 'n':
    Value.Null = ParseNull(Reader);
    break;
  case 't':
  case 'f':
    Value.Boolean = ParseBoolean(Reader);
    break;
  case '\"':
    Value.String = ParseString(Allocator, Reader);
    break;
  case '[':
    Value.Array = ParseArray(Allocator, Reader);
    break;
  case '{':
    Value.Object = ParseObject(Allocator, Reader);
    break;
  default:
    Reader->Error = INPUT_READER_ERROR_PARSING;
    break;
  }

  return Value;
}

json_value ParseValue(allocator::bump *Allocator, input_reader *Reader)
{
  AcceptWhitespace(Reader);
  json_value Value = ParseTrimmedValue(Allocator, Reader);
  AcceptWhitespace(Reader);

  return Value;
}

json_array ParseArray(allocator::bump *Allocator, input_reader *Reader)
{
  if (Accept(Reader, '['))
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

  AcceptWhitespace(Reader);
  bool32 FirstLoop = true;

  json_array Result;
  Result.Length = 0;

  input_reader Backtrack;
  Backtrack.Input = Reader->Input;
  Backtrack.Offset = Reader->Offset;
  Backtrack.Error = Reader->Error;

  while (!Accept(&Backtrack, ']'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(&Backtrack, ','))
    {
    }
    else
    {
      SetFlag(&Backtrack.Error, INPUT_READER_ERROR_PARSING);
      return {
          .Length = 0,
          .Values = 0x0,
      };
    }

    ParseValue(Allocator, &Backtrack);
    AcceptWhitespace(&Backtrack);

    Result.Length++;
    FirstLoop = false;
  }

  Result.Values = AllocateN(Allocator, json_value, Result.Length);
  key Index = 0;
  FirstLoop = true;

  while (!Accept(Reader, ']'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(Reader, ','))
    {
    }
    else
    {
      SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
    }

    if (Result.Values)
    {
      Result.Values[Index] = ParseValue(Allocator, Reader);
    }
    else
    {
      ParseValue(Allocator, Reader);
      SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
    }
    AcceptWhitespace(Reader);

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

json_object *ParseObject(allocator::bump *Allocator, input_reader *Reader)
{
  if (Accept(Reader, '{'))
  {
  }
  else
  {
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
    return 0x0;
  }

  input_reader Backtrack;
  Backtrack.Input = Reader->Input;
  Backtrack.Offset = Reader->Offset;
  Backtrack.Error = Reader->Error;

  bool32 FirstLoop = true;
  key ObjectCount = 0;

  while (!Accept(&Backtrack, '}'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(&Backtrack, ','))
    {
    }
    else
    {
      SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
      return 0x0;
    }

    AcceptWhitespace(&Backtrack);
    AcceptString(&Backtrack);
    AcceptWhitespace(&Backtrack);

    Accept(&Backtrack, ':');
    ParseValue(Allocator, &Backtrack);
    ObjectCount++;
    FirstLoop = false;
  }

  if (HasFlag(&Backtrack.Error, INPUT_READER_ERROR_PARSING))
  {
    return 0x0;
  }

  FirstLoop = true;
  json_object *Result = Allocate(Allocator, json_object);

  if (Result)
  {
    Result->Count = ObjectCount;
    Result->Capacity = key(f32(ObjectCount) * 1.5f); // Reduce risk of collisions
    Result->Slots = AllocateN(Allocator, json_object_slot, Result->Capacity);
  }
  else
  {
    SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
  }

  while (!Accept(Reader, '}'))
  {
    if (FirstLoop)
    {
    }
    else if (Accept(Reader, ','))
    {
    }
    else
    {
      Reader->Error = INPUT_READER_ERROR_PARSING;
    }

    AcceptWhitespace(Reader);
    json_raw_string RawString = AcceptString(Reader);
    AcceptWhitespace(Reader);

    Accept(Reader, ':');

    if (Result && Result->Slots)
    {
      json_object_slot *Slot = AssignEmptySlot(Result, &RawString);
      Slot->Value = ParseValue(Allocator, Reader);
    }
    else
    {
      SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
      ParseValue(Allocator, Reader);
    }

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
  input_reader Reader = {
      .Offset = 0,
      .Error = 0,
      .Input = "{ \"First\": [123, 1.45], \"Another\": false, \"A number\" : "
               "456, \"SubObject\": { \"A property\": \"substring\"}}",
  };

  allocator::bump *Allocator = allocator::CreateBump(JSON_ALLOCATION_SIZE);
  json_object *Object = ParseValue(Allocator, &Reader).Object;

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    json_object *SubObject = GetSlot(Object, "SubObject")->Value.Object;
    f32 Number = GetSlot(Object, "A number")->Value.Number;
    json_string SubObjectString = GetSlot(SubObject, "A property")->Value.String;
    json_array Array = GetSlot(Object, "First")->Value.Array;

    printf("Successfully parsed object: %f, %s, %f\n", Number, SubObjectString.Buffer,
           Array.Values[0].Number);
  }
  else
  {
    printf("Failed to parse object.\n");
  }

  allocator::Destroy(Allocator);

  return 0;
}
