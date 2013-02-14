 /*! @file IncRev.c
 *  \brief Tool to increment version numbers in text files.
 *
 *  (C) by Georg Wiora 1990/2008
 *
 *  Based on an old Amiga Ansi-C program by Georg Wiora
 *  from 3rd January 1990.
 * 
 *  This program is published under the 
 *  Boost Software License V1.0 (s. below)
 *
 *  Usage:
 *  Syntax: IncRev <File> <Pattern> [<Increment>]
 *  The specified <File> must contain a line with the given pattern.
 *    \li Example:\n
 *         "textexttextpattern   123"\n
 *         IncRev will search "pattern" (case sensitive), read the number
 *         and use the space after pattern to the last digit to replace it
 *         with the incremented number, in this example 124.
 *
 *  The substitution happens in place without using a temp file!
 *  The optional <Increment> parameter is a number to add to the
 *  current version number. Default is 1.
 */

/****************************************************************************
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the IncRev and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

http://www.boost.org/LICENSE_1_0.txt
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/// Version number of IncRev
#define VERSION 2
/// Minor Version or Revision number of IncRev
#define REVISION 0

/*! @brief Search for line with pattern in file.
    Position the file pointer at the end of pattern.

    @arg \c fp File pointer to the file to search.
    @arg \c pat pointer to the pattern string that is searched in file.

    @return the number of characters available for the incremented number.
    The file pointer is repostioned right after the pattern
*/
int
findline(FILE *fp, const char *pat)
{
  // Line buffer
  char buffer[2048];
  // Current position in file
  long fpos=0;
  // End of pattern index
  long patend=0;
  // Index of first and last digit
  long firstdigit=0,lastdigit=0,startofnumber=0;
  // Start position of pattern in line
  char *patpos=NULL;
	size_t len=0;
  unsigned int i;

  // Get String length
	len = strlen(pat);

  // Seek through file for pattern
	do
	{
    // Save current file position
    fpos = ftell(fp);
    // Get line, return error on end of file
		if (fgets (buffer,sizeof(buffer),fp)==NULL)
			return -1;

    // Find first occurrence of pattern in line
   patpos = strstr(buffer,pat);

    // Loop while pattern not found
	} while (patpos == NULL);
  
  // Move file pointer to begin of line
	fseek (fp,fpos,SEEK_SET);

  // Index to one character past end of pattern
  patend = (long)(patpos - (&buffer[0])) + (long)strlen(pat);
  // Search first digit or sign in string starting one character after end of pattern
  firstdigit=(long)strcspn(buffer+patend,"+-0123456789");

  // Nothing found? Return error code
  if (firstdigit == (long)strlen(buffer))
    return -1;

  // Search last digit. This is the end of the number
  lastdigit=(long)strspn(buffer+patend+firstdigit+1,"0123456789")+1;

  // Search backwards to find first non digit, non blank. This is the space we have for the number
  for (i=patend+firstdigit-1; i>0; i--)
  {
    if (!isdigit(buffer[i]) && !isspace(buffer[i]))
    {
      // Found end of space. Store position for start of number
      startofnumber = i+1;
      break;
    }
  }
  // If nothing found, we can start right after pattern, leaving one blank
  if (i==0)
    startofnumber=1;

  // position file pointer to start of writable space
  fseek(fp,startofnumber,SEEK_CUR);

  // Return size of field
  return patend+firstdigit+lastdigit-startofnumber;
}

/*! @brief Increment a version number in a text file
    
    \li Usage: IncRev <File> <Pattern> [<Increment>]\n
    The specified <File> must contain a line with the given pattern.
      \li Example:\n
          "textexttextpattern   123"\n
          IncRev will search "pattern" (case sensitive), read the number
          and use the space after pattern to the last digit to replace it
          with the incremented number, in this example 124.
  
    The substitution happens in place without using a temp file!
    The optional <Increment> parameter is a number to add to the
    current version number. Default is 1.

    @arg \c argc Argument count.
    @arg \c argv List of argument strings.

    @return 0 for successful run and 1 for errors.
    Error messages are printed to \c stderr stream
*/

int
main (const int argc, const char *argv[])
{
	FILE *fp;
	long num=0;
	int len;
  long cpos=0;
  // Buffer for number
  char numbuf[30];
  // arguments
  const char *filename;
  const char *pattern;
  // Increment
  long increment=1;


  // Check arguments
	if ((argc < 3) || (argc>4))
	{
		fprintf (stderr,"IncRev V%d.%d\n"
      "Syntax: %s <File> <Pattern> [<Increment>]\n"
      "  The specified <File> must contain a line with the given pattern.\n"
      "  Example:\n"
      "    \"textexttextpattern   123\"\n"
      "    IncRev will search \"pattern\" (case sensitive), read the number\n"
      "    and use the space after pattern to the last digit to replace it\n"
      "    with the incremented number, in this example 124.\n"
      "  The substitution happens in place without using a temp file!\n"
      "  The optional <Increment> parameter is a number to add to the\n"
      "  current version number. Default is 1.\n",
      VERSION,REVISION,argv[0]);
		return 0;
	}

  // Assign parameters
  filename=argv[1];
  pattern=argv[2];

  // Check for increment
  if (argc==4)
  {
    // try to read it.
    if (sscanf(argv[3],"%d",&increment) != 1)
    {
      fprintf(stderr,"IncRev Error: Could not read increment number from command line parameter 4: \"%s\"!\n",argv[3]);
      return 1;
    }
  }

  // Open file for reading and writing in text mode
	if ((fp = fopen(filename,"rb+")) == NULL)
	{
    fprintf (stderr,"IncRev Error: Could not open file \"%s\"!\n",filename);
		return 1;
	}

  // Search pattern in file and position write pointer
	if ((len = findline(fp,pattern))<1)
	{
    fprintf (stderr,"IncRev Error: Pattern \"%s\" not found\n"
                    "       in file \"%s\".\n"
                    "       Left file unchanged!\n",pattern,filename);
    fclose (fp);
		return 1;
	}

  // save current position
  cpos = ftell(fp);
  // Read the number
  if (fscanf(fp,"%d",&num)!=1)
	{
    fprintf (stderr,"IncRev Error: Could not read the current version number\n"
                    "       in file \"%s\".\n"
                    "       Left file unchanged!\n",filename);
    fclose (fp);
		return 1;
	}

  // Reposition to start of number
  fseek(fp,cpos,SEEK_SET);
  // Increment number and convert to string 
  sprintf(numbuf,"%*d",len,num+increment);
  // check if space in file is sufficient
  if (strlen(numbuf)>(size_t)len)
	{
    fclose (fp);
    fprintf (stderr,"IncRev Error: Could not increment version number %d\n"
                    "       in file \"%s\"\n"
                    "       after string \"%s\",\n"
                    "       because new number %d would overwrite file contents.\n"
                    "       Please provide more blank space on the left side of the number!\n"
                    "       Left file unchanged!\n",num,filename,pattern,num+increment);
		return 1;
	}

  fprintf(stderr,"IncRev: Incrementing number %d after string \"%s\"\n"
                 "        in file \"%s\"\n"
                 "        to %d\n",num,pattern,filename,num+increment);
  
  // write number to current position
  fprintf(fp,"%*d",len,num+increment);
  // Close file
	fclose (fp);

  // No error
	return 0;
}