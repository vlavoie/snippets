/*
Example program to use cartridge.hh
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
#include <cartridge.hh>

// glibc
#include <stdio.h>
#include <stdlib.h>

// Arg0...N-1 are packed input files, N is the output file
i32 main(i32 Argc, const char *Argv[])
{
  if (Argc < 3)
  {
    fprintf(stderr,
            "At least 1 input file is required for packing and 1 output for the packed files.");
    return 1;
  }

  const char *OutputFile = Argv[Argc - 1];

  crpk::code ErrorCode = crpk::Package(Argc - 2, &Argv[1], OutputFile);

  if (ErrorCode >= crpk::RETURN_CODE_INPUT_FILE_ERROR)
  {
    fprintf(stderr, "Error reading input file '%s'", Argv[ErrorCode]);
    return 1;
  }
  else if (ErrorCode == crpk::RETURN_CODE_OUTPUT_ERROR)
  {
    fprintf(stderr, "Error writing output file '%s'", OutputFile);
    return 1;
  }

  return 0;
}
