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

#include <allocators/bump.hh>
#include <allocators/fake.hh>
#include <buffer.hh>
#include <common.hh>
#include <hash.hh>
#include <math2d.hh>

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
struct json_value_header;

struct json_array
{
  key Length;
  json_value_header *Values;
};

struct json_object;

union json_value {
  f32 Number;
  bool32 Boolean;
  bool32 Null;
  json_string *String;
  json_array *Array;
  json_object *Object;
};

struct json_value_header
{
  key Type;
  json_value Value;
};

struct json_object_slot
{
  hash::digest Hash;
  json_value_header Header;
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

#define JSON_NULL_VALUE INT32_MAX

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
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
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
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
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
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
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

  SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
  return false;
}

bool32 ParseNull(input_reader *Reader)
{
  if (Accept(Reader, 'n') && Accept(Reader, 'u') && Accept(Reader, 'l') && Accept(Reader, 'l'))
  {
    return JSON_NULL_VALUE;
  }

  SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
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
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
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
            SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
            return {.Length = 0, .String = 0x0};
          }
        }
      }
      else
      {
        SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
        return {.Length = 0, .String = 0x0};
      }
    }
    else if (AnyExcept(Reader, '\"'))
    {
      Length++;
    }
    else if (Accept(Reader, '\0'))
    {
      SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
      return {.Length = 0, .String = 0x0};
    }
  }

  return {.Length = Length, .String = StringStart};
}

template <typename A> json_string *ParseString(A *Allocator, input_reader *Reader)
{
  json_raw_string RawString = AcceptString(Reader);

  if (HasFlag(Reader->Error, INPUT_READER_ERROR_PARSING))
  {
    return 0x0;
  }

  json_string *Result = Allocate(Allocator, json_string);

  if (Result)
  {
    Result->Length = RawString.Length;
    Result->Buffer = AllocateN(Allocator, char, RawString.Length + 1);
  }
  else
  {
    AllocateN(Allocator, char, RawString.Length + 1);
    SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
  }

  if (Result && Result->Buffer)
  {
    Result->Buffer[RawString.Length] = '\0';

    for (key CharIndex = 0; CharIndex < RawString.Length; CharIndex++)
    {
      Result->Buffer[CharIndex] = RawString.String[CharIndex];
    }
  }
  else
  {
    SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
  }

  return Result;
}

template <typename A> json_array *ParseArray(A *Allocator, input_reader *Reader);
template <typename A> json_object *ParseObject(A *Allocator, input_reader *Reader);

template <typename A> json_value_header ParseTrimmedValue(A *Allocator, input_reader *Reader)
{
  json_value_header Header;

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
    Header.Type = JSON_VALUE_TYPE_NUMBER;
    Header.Value.Number = ParseNumber(Reader);
    break;
  case 'n':
    Header.Type = JSON_VALUE_TYPE_NULL;
    Header.Value.Null = ParseNull(Reader);
    break;
  case 't':
  case 'f':
    Header.Type = JSON_VALUE_TYPE_BOOLEAN;
    Header.Value.Boolean = ParseBoolean(Reader);
    break;
  case '\"':
    Header.Type = JSON_VALUE_TYPE_STRING;
    Header.Value.String = ParseString(Allocator, Reader);
    break;
  case '[':
    Header.Type = JSON_VALUE_TYPE_ARRAY;
    Header.Value.Array = ParseArray(Allocator, Reader);
    break;
  case '{':
    Header.Type = JSON_VALUE_TYPE_OBJECT;
    Header.Value.Object = ParseObject(Allocator, Reader);
    break;
  default:
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
    break;
  }

  return Header;
}

template <typename A> json_value_header ParseValue(A *Allocator, input_reader *Reader)
{
  AcceptWhitespace(Reader);
  json_value_header Value = ParseTrimmedValue(Allocator, Reader);
  AcceptWhitespace(Reader);

  return Value;
}

template <typename A> json_array *ParseArray(A *Allocator, input_reader *Reader)
{
  if (Accept(Reader, '['))
  {
  }
  else
  {
    SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
    return 0x0;
  }

  AcceptWhitespace(Reader);
  bool32 FirstLoop = true;

  json_array *Result = Allocate(Allocator, json_array);

  if (Result)
  {
    Result->Length = 0;
  }
  else
  {
    SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
  }

  input_reader Backtrack;
  Backtrack.Input = Reader->Input;
  Backtrack.Offset = Reader->Offset;
  Backtrack.Error = Reader->Error;
  key ValuesLength = 0;

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
      return 0x0;
    }

    ParseValue(Allocator, &Backtrack);
    AcceptWhitespace(&Backtrack);

    ValuesLength++;
    FirstLoop = false;
  }

  if (Result)
  {
    Result->Length = ValuesLength;
    Result->Values = AllocateN(Allocator, json_value_header, ValuesLength);
  }
  else
  {
    AllocateN(Allocator, json_value_header, ValuesLength);
  }

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

    if (Result && Result->Values)
    {
      Result->Values[Index] = ParseValue(Allocator, Reader);
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

template <typename A> json_object *ParseObject(A *Allocator, input_reader *Reader)
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

  if (HasFlag(Backtrack.Error, INPUT_READER_ERROR_PARSING))
  {
    return 0x0;
  }

  FirstLoop = true;
  json_object *Result = Allocate(Allocator, json_object);
  key Capacity = f32(ObjectCount) * 1.5f;

  if (Result)
  {
    Result->Count = ObjectCount;
    Result->Capacity = Capacity; // Reduce risk of collisions
    Result->Slots = AllocateN(Allocator, json_object_slot, Capacity);
  }
  else
  {
    AllocateN(Allocator, json_object_slot, Capacity);
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
      SetFlag(&Reader->Error, INPUT_READER_ERROR_PARSING);
    }

    AcceptWhitespace(Reader);
    json_raw_string RawString = AcceptString(Reader);
    AcceptWhitespace(Reader);

    Accept(Reader, ':');

    if (Result && Result->Slots)
    {
      json_object_slot *Slot = AssignEmptySlot(Result, &RawString);
      Slot->Header = ParseValue(Allocator, Reader);
    }
    else
    {
      SetFlag(&Reader->Error, INPUT_READER_ERROR_ALLOCATION);
      ParseValue(Allocator, Reader);
    }

    if (HasFlag(Reader->Error, INPUT_READER_ERROR_PARSING))
    {
      return Result;
    }

    FirstLoop = false;
  }

  return Result;
}

json_value_header *JsonGetValue(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  return &Slot->Header;
}

f32 JsonGetNumber(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  Assert(Slot->Header.Type == JSON_VALUE_TYPE_NUMBER, "Json field was not a number.");

  return Slot->Header.Value.Number;
}

bool32 JsonGetBoolean(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  Assert(Slot->Header.Type == JSON_VALUE_TYPE_BOOLEAN, "Json field was not a boolean.");

  return Slot->Header.Value.Boolean;
}

bool32 JsonGetNull(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  Assert(Slot->Header.Type == JSON_VALUE_TYPE_NULL, "Json field was not a null value.");

  return Slot->Header.Value.Null;
}

json_string *JsonGetString(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  Assert(Slot->Header.Type == JSON_VALUE_TYPE_STRING, "Json field was not a string.");

  return Slot->Header.Value.String;
}

json_array *JsonGetArray(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  Assert(Slot->Header.Type == JSON_VALUE_TYPE_ARRAY, "Json field was not an array.");

  return Slot->Header.Value.Array;
}

json_object *JsonGetObject(const json_object *Object, const char *FieldName)
{
  json_object_slot *Slot = GetSlot(Object, FieldName);
  Assert(Slot->Header.Type == JSON_VALUE_TYPE_OBJECT, "Json field was not a number.");

  return Slot->Header.Value.Object;
}

inline json_value_header *JsonArrayGet(const json_array *Array, const key Index)
{
  Assert(Index < Array->Length, "Array Index was out of bounds.");
  return &Array->Values[Index];
}

json_value_header *JsonGetValue(const json_array *Array, const key Index)
{
  return JsonArrayGet(Array, Index);
}

f32 JsonGetNumber(const json_array *Array, const key Index)
{
  json_value_header *Header = JsonArrayGet(Array, Index);
  Assert(Header->Type == JSON_VALUE_TYPE_NUMBER, "Json field was not a number.");

  return Header->Value.Number;
}

bool32 JsonGetBoolean(const json_array *Array, const key Index)
{
  json_value_header *Header = JsonArrayGet(Array, Index);
  Assert(Header->Type == JSON_VALUE_TYPE_BOOLEAN, "Json field was not a boolean.");

  return Header->Value.Boolean;
}

bool32 JsonGetNull(const json_array *Array, const key Index)
{
  json_value_header *Header = JsonArrayGet(Array, Index);
  Assert(Header->Type == JSON_VALUE_TYPE_NULL, "Json field was not a null value.");

  return Header->Value.Null;
}

json_string *JsonGetString(const json_array *Array, const key Index)
{
  json_value_header *Header = JsonArrayGet(Array, Index);
  Assert(Header->Type == JSON_VALUE_TYPE_STRING, "Json field was not a string.");

  return Header->Value.String;
}

json_array *JsonGetArray(const json_array *Array, const key Index)
{
  json_value_header *Header = JsonArrayGet(Array, Index);
  Assert(Header->Type == JSON_VALUE_TYPE_ARRAY, "Json field was not an array.");

  return Header->Value.Array;
}

json_object *JsonGetObject(const json_array *Array, const key Index)
{
  json_value_header *Header = JsonArrayGet(Array, Index);
  Assert(Header->Type == JSON_VALUE_TYPE_OBJECT, "Json field was not a number.");

  return Header->Value.Object;
}

template <typename A> json_value_header JsonParse(A *Allocator, input_reader *Reader)
{
  return ParseValue(Allocator, Reader);
}

i32 main(const i32 Argc, const char *Argv[])
{
  input_reader Reader;
  Reader.Offset = 0;
  Reader.Error = 0;

  if (Argc > 1)
  {
    buffer JsonBuffer = LoadBufferFromFile(Argv[1]);

    if (!IsInitialized(JsonBuffer))
    {
      printf("Failed to load JSON file.\n");
      return 1;
    }

    Reader.Input = (const char *)JsonBuffer.Data;
  }
  else
  {
    printf("Missing JSON file argument.\n");
    return 1;
  }

  allocator::fake *FakeAllocator = allocator::CreateFake();

  JsonParse(FakeAllocator, &Reader);
  const key AllocationSize = allocator::MemoryUsed(FakeAllocator);

  allocator::Destroy(FakeAllocator);

  if (HasFlag(Reader.Error, INPUT_READER_ERROR_PARSING))
  {
    printf("Failed to parse json object.\n");
    return 1;
  }

  Reader.Offset = 0;
  Reader.Error = 0;
  allocator::bump *Allocator = allocator::CreateBump(AllocationSize);
  json_value_header Root = JsonParse(Allocator, &Reader);

  if (Reader.Error == INPUT_READER_ERROR_NONE)
  {
    json_object *FourthEntry = JsonGetObject(Root.Value.Array, 3);
    printf("Fourth entry name is: %s, language: %s, id: %s, bio: %s, version: %f\n",
           JsonGetString(FourthEntry, "name")->Buffer,
           JsonGetString(FourthEntry, "language")->Buffer, JsonGetString(FourthEntry, "id")->Buffer,
           JsonGetString(FourthEntry, "bio")->Buffer, JsonGetNumber(FourthEntry, "version"));
    printf("Entry count for array is: %lu\n", Root.Value.Array->Length);
  }
  else
  {
    printf("Failed to parse object.\n");
    allocator::Destroy(Allocator);
    return 1;
  }

  allocator::Destroy(Allocator);

  return 0;
}
