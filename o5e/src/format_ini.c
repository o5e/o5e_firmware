
// Program to take a TunerStudio .ini file and do the reformatting necessary to be the #define flash portion of variables.h

/*
Example input:
; comment
      man_crank_corr_variable = scalar,  U8,    544,     ... "Fixed axis","Variable axis"//      (1 byte) Table axis type?

Output:
#define man_crank_corr_variable  (*(CONST uint8_t *)(&Page_Ptr[1][544]))

*/

/*
Copyright (c) 2011 Jon Zeeff

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <string.h>

int main()
{
    char *ptr;
    static char name[1024];
    static char prev_name[1024];
    char type[1024];
    char size[1024];
    char shape[1024];
    int offset = 0;
    int prev_offset = 0;
    char line[1024];
    char line2[1024];
    int page = 0;
    int table = 0;              // currently working on a table
    int table_start;            // offset value at start of table
    int conditional = 0;        // currently working on a #if
    int i;

/*
MAP_2_Rows              = scalar,   U16,   1484,                "",      1.00000,  0.00000,        1,        1,   0;*(2 byte),  bin 0
MAP_2_Cols              = scalar,   U16,   1486,                "",      1.00000,  0.00000,        1,       16,   0;*(2 byte),  bin 0
MAP_2_Bins              = array,    S16,   1488,    [  16],    "V",    .00024414,  0.00000,    0.000,    6.000,   3;*(64 byte), bin 12, the ecu require Bins but only 16 are being used in the tuner
MAP_2_Variable          = bits,     U08,   1516,     [0:0], "Fixed axis","Variable axis";                           *(1 byte),  bin 0,  Table axis type?
MAP_2_Byte              = bits,     U08,   1517,     [0:0], "16 bit signed","8 bit unsigned"   ;                    *(1 byte),  bin 0,  Variable type?
;     MAP_2_Res1              = scalar,   U08,   1518,                "",      1.00000,  0.00000,        0,      255,   0;*(1 byte),  bin 0,  unused for now, could be a skip instead
;     MAP_2_Res2              = scalar,   U08,   1519,                "",      1.00000,  0.00000,        0,      255,   0;*(1 byte),  bin 0,  unused for now, could be a skip instead
      MAP_2_Cal               = array,    S16,   1620,    [  16],  "kPa",      .024414,  0.00000,      0.0,    600.0,   1;*(32 byte), bin 12, ECU stors as kPa/100kPa
*/

    while (fgets(line, sizeof(line), stdin)) {

        i = sscanf(line, "%*[ ]%[^, =]%*[, =]%[^, ]%*[, ]%[^, ]%*[, ]%d%*[, ]%[^, ]%s", name, type, size, &offset,
                   shape, line2);

        if (strncmp(line, "[OutputC", 8) == 0) {
            if (table)
                printf("no end of table\n");
            break;              // end of flash variables
        }

        if (table) {            // processing a table - skip the elements, stop when we see _Cal
            if (strstr(line, "_Cal")) {
                table = 0;
                prev_offset = offset;
                if (table_start + 136 != offset)
                    printf("offset error - %s\n", line);
            } else if (strstr(line, "_Byte")) {
                if (table_start + 133 != offset)
                    printf("offset error - %s\n", line);
                continue;
            }
            continue;
        }

        if ((strstr(line, "U16,") || strstr(line, "S16,")) && (offset & 1))
            printf("alignment error - %s\n", line);

        if (strstr(line, "page count")) {
            fprintf(stderr, "%s,", line + 14);
            continue;
        }

        if (strstr(line, "pageSize")) {
            fprintf(stderr, "%s,", line);
            continue;
        }

        if (strstr(line, "#if ")) {
            conditional = 1;
            continue;
        }

        if (conditional) {      // Skip for part of #if CELSIUS conditionals
            if (strstr(line, "#else"))
                conditional = 0;
            else
                continue;
        }

        if (name[0] == ';' || name[0] == '\0') {;
        } else if (i > 3) {

            if (strcmp(name, prev_name) != 0) {

                if (strcmp(type, "scalar") == 0 || strcmp(type, "bits") == 0) {
                    if (offset < prev_offset) {
                        ++page;
                        printf("// Page %d\n", page + 1);
                    }
                    prev_offset = offset;
                    ptr = strstr(name, "_Rows");        // detect a table
                    if (ptr) {
                        *ptr = '\0';
                        printf("#define %s_Table ((CONST struct table_jz *)(&Page_Ptr[%d][%d]))\n", name, page, offset);
                        table = 1;
                        table_start = offset;
                        if (table_start & 1)
                            printf("table not on even offset - %s\n", line);
                    } else {
                        ptr = strstr(name, "_1");       // detect an array that was declared as a series of scalars
                        if (ptr && *(ptr + 2) == '\0') {
                            printf("#define %s (*(CONST %s *)(&Page_Ptr[%d][%d]))\n", name, size, page, offset);
                            *ptr = '\0';
                            printf("#define %s_Array ((CONST %s *)(&Page_Ptr[%d][%d]))\n", name, size, page, offset);
                        } else {
                            // detect bit field and mask it off 0:0 -> 1, 0:1 -> 3, 0:2 -> 7
                            if (shape[0] == '[')
                                printf("#define %s (*(CONST %s *)(&Page_Ptr[%d][%d]) & ((2<<%c)-1))\n", name, size,
                                       page, offset, shape[3]);
                            else
                                printf("#define %s (*(CONST %s *)(&Page_Ptr[%d][%d]))\n", name, size, page, offset);
                        }
                    }
                }
                if (strcmp(type, "array") == 0) {
                    if (offset < prev_offset) {
                        ++page;
                        printf("// Page %d\n", page + 1);
                    }
                    prev_offset = offset;
                    //printf("// %s",line);
                    printf("#define %s_Array ((CONST %s *)(&Page_Ptr[%d][%d]))\n", name, size, page, offset);
                }

            }
            strcpy(prev_name, name);

        }

    }                           // while

    return 0;

}                               // main
