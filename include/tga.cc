#include "tga.hh"

inline byte Read8(tga::reader *Reader)
{
  byte Result = Reader->Offset[0];
  Reader->Offset++;
  return Result;
}

inline u32 Read32LE(tga::reader *Reader)
{
  u32 Result = u32(Reader->Offset[3] << 24 | Reader->Offset[2] << 16 | Reader->Offset[1] << 8 |
                   Reader->Offset[0]);
  Reader->Offset += 4;
  return Result;
}

inline u32 Read24LE(tga::reader *Reader)
{
  u32 Result = u32(Reader->Offset[2] << 16 | Reader->Offset[1] << 8 | Reader->Offset[0]);
  Reader->Offset += 3;
  return Result;
}

inline word Read16LE(tga::reader *Reader)
{
  word Result = word(Reader->Offset[1] << 8 | Reader->Offset[0]);
  Reader->Offset += 2;
  return Result;
}

inline tga::pixel Read32RGBA(tga::reader *Reader)
{
  tga::pixel Result = tga::pixel{
      .R = Reader->Offset[2],
      .G = Reader->Offset[1],
      .B = Reader->Offset[0],
      .A = Reader->Offset[3],
  };

  Reader->Offset += 4;
  return Result;
}

inline tga::pixel Read24RGB(tga::reader *Reader)
{
  tga::pixel Result = tga::pixel{
      .R = Reader->Offset[2],
      .G = Reader->Offset[1],
      .B = Reader->Offset[0],
      .A = 255,
  };

  Reader->Offset += 3;
  return Result;
}

inline tga::pixel Read16RGB(tga::reader *Reader)
{
  tga::pixel Result = {
      .R = byte((Reader->Offset[1] & 0x7c) << 1),
      .G = byte(((Reader->Offset[1] & 0x03) << 6) | ((Reader->Offset[0] & 0xe0) >> 2)),
      .B = byte((Reader->Offset[0] & 0x1f) << 3),
      .A = byte((Reader->Offset[1] & 0x80)),
  };

  Reader->Offset += 2;
  return Result;
}

inline tga::pixel ReadColorTable(byte *Table, word Origin, word Length, byte Depth, key Index)
{
  key DepthByteLength = Depth / 8;
  key ByteOffset = (Origin * DepthByteLength) + (Index * DepthByteLength);

  tga::reader TempReader = {.Offset = Table + ByteOffset};

  switch (Depth)
  {
  case 24:
    return Read24RGB(&TempReader);
    break;
  case 32:
    return Read32RGBA(&TempReader);
    break;
  default:
    return tga::pixel{0, 0, 0, 0};
  };
}

inline tga::pixel ReadPixelData(tga::reader *Reader, byte Depth)
{
  switch (Depth)
  {
  case 16:
    return Read16RGB(Reader);
  case 24:
    return Read24RGB(Reader);
  case 32:
    return Read32RGBA(Reader);
  default:
    return tga::pixel{0, 0, 0, 0};
  };
}

tga::texture *tga::Decompress(key Length, void *Data, i32 *ErrorCode)
{
  if (sizeof(tga::header) >= Length)
  {
    if (ErrorCode)
    {
      *ErrorCode = tga::ERROR_CODE_DATA_SIZE;
    }
    return 0x0;
  }

  tga::reader Reader = {.Offset = (byte *)Data};

  tga::header Header;
  Header.IdLength = Read8(&Reader);
  Header.ColorMapType = Read8(&Reader);
  Header.DataTypeCode = Read8(&Reader);
  Header.ColorMapOrigin = Read16LE(&Reader);
  Header.ColorMapLength = Read16LE(&Reader);
  Header.ColorMapDepth = Read8(&Reader);
  Header.OriginX = Read16LE(&Reader);
  Header.OriginY = Read16LE(&Reader);
  Header.Width = Read16LE(&Reader);
  Header.Height = Read16LE(&Reader);
  Header.PixelDepth = Read8(&Reader);
  Header.ImageDescriptor = Read8(&Reader);

  Reader.Offset += Header.IdLength;

  byte *ColorMapData = Reader.Offset;
  Reader.Offset += Header.ColorMapType * Header.ColorMapLength * (Header.ColorMapDepth / 8);

  if (!(Header.DataTypeCode == 1 || Header.DataTypeCode == 2 || Header.DataTypeCode == 10))
  {
    if (ErrorCode)
    {
      *ErrorCode = tga::ERROR_CODE_DATA_TYPE;
    }
    return 0x0;
  }

  if (!(Header.ColorMapType == 0 || Header.ColorMapType == 1))
  {
    if (ErrorCode)
    {
      *ErrorCode = tga::ERROR_CODE_COLOR_MAP_TYPE;
    }
    return 0x0;
  }

  if (!(Header.PixelDepth == 8 || Header.PixelDepth == 16 || Header.PixelDepth == 24 ||
        Header.PixelDepth == 32))
  {
    if (ErrorCode)
    {
      *ErrorCode = tga::ERROR_CODE_PIXEL_DEPTH;
    }
    return 0x0;
  }

  tga::pixel PixelData;
  key N = 0, I, J, RLEChunk, ColorIndex;

  tga::texture *Result = (tga::texture *)__TGA__Allocate(sizeof(tga::texture));
  key DataSize = Header.Width * Header.Height;
  Result->Width = Header.Width;
  Result->Height = Header.Height;
  Result->Pixels = (tga::pixel *)__TGA__Allocate(sizeof(tga::pixel) * DataSize);

  while (N < DataSize)
  {
    if (Header.DataTypeCode == 1)
    {
      // uncompressed data
      switch (Header.PixelDepth)
      {
      case 8:
        ColorIndex = Read8(&Reader);
        break;
      case 16:
        ColorIndex = Read16LE(&Reader);
        break;
      case 24:
        ColorIndex = Read24LE(&Reader);
        break;
      case 32:
        ColorIndex = Read32LE(&Reader);
        break;
      default:
        return 0x0;
      };

      Result->Pixels[N] = ReadColorTable(ColorMapData, Header.ColorMapOrigin, Header.ColorMapLength,
                                         Header.ColorMapDepth, ColorIndex);
      N++;
    }
    else if (Header.DataTypeCode == 2)
    {
      // uncompressed non-colormap data
      PixelData = ReadPixelData(&Reader, Header.PixelDepth);

      Result->Pixels[N] = PixelData;
      N++;
    }
    else if (Header.DataTypeCode == 10)
    {
      // compressed non-colormap chunk
      J = Reader.Offset[0] & 0x7f;
      RLEChunk = Reader.Offset[0] & 0x80;

      Reader.Offset++;
      PixelData = ReadPixelData(&Reader, Header.PixelDepth);

      Result->Pixels[N] = PixelData;
      N++;

      if (RLEChunk)
      {
        // RLE chunk
        for (I = 0; I < J; I++)
        {
          Result->Pixels[N] = PixelData;
          N++;
        }
      }
      else
      {
        // normal chunk
        for (I = 0; I < J; I++)
        {
          PixelData = ReadPixelData(&Reader, Header.PixelDepth);

          Result->Pixels[N] = PixelData;
          N++;
        }
      }
    }
  }

  return Result;
}

tga::texture *tga::Decompress(key Length, void *Data)
{
  return tga::Decompress(Length, Data, 0x0);
}
