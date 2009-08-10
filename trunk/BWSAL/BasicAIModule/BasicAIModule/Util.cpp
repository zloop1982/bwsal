#include "Util.h"
#include <fstream>
#include <stdarg.h>
#include <sys/stat.h>
char buffer[1024];
void log(const char* text, ...)
{
  FILE * pFile;
  pFile = fopen ("bwapi-data/logs/BasicAIModule.log","a");
  
  va_list ap;
  va_start(ap, text);
  vsprintf(buffer,text,ap );
  va_end(ap);
  if (pFile!=NULL)
  {
    fputs (buffer,pFile);
    fputs ("\n",pFile);
    fclose (pFile);
  }
}