#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#define INCL_NOPM
#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSNLS
#include <os2.h>

#define MAX_MINUTES 20
#define AUTO_ON     ((unsigned char)0xE3)
#define AUTO_OFF    ((unsigned char)0xE2)

extern const char _far * _cdecl _near _pgmptr;

#pragma optimize("",off)
#pragma check_stack(off)
int _far _pascal _export ShipCmd(unsigned char cmd, unsigned char periods);
#pragma alloc_text(IOPL_SEG,ShipCmd)
int _far _pascal _export ShipCmd(unsigned char cmd, unsigned char periods)
   {
   _asm
      {
      cli
      mov dx, 0x3F6
      jmp SHORT $+2
      jmp SHORT $+2
      jmp SHORT $+2
      in al,dx
      and al, 0x80        // FÅhrt noch letzten Befehl aus
      jnz Busy

      mov al,0x0B         // Command: Read Interrupt service register
      out 0xA0, al
      jmp SHORT $+2
      jmp SHORT $+2
      jmp SHORT $+2
      in al, 0xA0         // Service register
      and al, 0x40        // Irq14 (Platte) aktiv ?
      jnz Busy

      mov dx, 0x1F2
      mov al, periods
      jmp SHORT $+2
      jmp SHORT $+2
      jmp SHORT $+2
      out dx, al
      mov dx, 0x1F7
      mov al, cmd
      jmp SHORT $+2
      jmp SHORT $+2
      jmp SHORT $+2
      out dx, al
      mov ax, 1
      jmp SHORT Done

      Busy:
      xor ax, ax
      Done:
      sti
      }
   };
#pragma optimize("",on)
#pragma check_stack()

static USHORT _fastcall _near QueryCountry(void)
   {
   USHORT ciLen = 0;
   COUNTRYCODE cc;
   COUNTRYINFO ci;

   memset(&cc,0,sizeof(cc));
   memset(&ci,0,sizeof(ci));

   if (!DosGetCtryInfo(sizeof(ci), &cc, &ci, &ciLen)
       && ciLen >= sizeof(USHORT))
      return ci.country;
   return 0;
   };

int _cdecl main (int argC, char *argV[])
   {
   int minutes;
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];
   char path[_MAX_PATH];

   USHORT country = QueryCountry(); // 18-DEC-92

   if (argC == 2
       && ( ((minutes = atoi(argV[1])) && abs(minutes) <= MAX_MINUTES)
            || !strcmp(argV[1],"0")))
      {
      time_t now = time(NULL);
      do
         {
         if (ShipCmd((minutes < 0 ? AUTO_OFF : AUTO_ON),
// Falsch    (unsigned char)((unsigned char)abs((unsigned char)minutes)*(unsigned char)12)))
/*06-JUN-91*/(unsigned char)((unsigned char)abs(minutes)*(unsigned char)12)))
            return 0;
         DosSleep(10L);
         }
      while ((time(NULL)-now) < 3);

      if (country == 49)
         fprintf(stderr, "\aPlattensteuerung ist z.Zt. beschÑftigt.\a");
      else
         fprintf(stderr, "\aDrive is busy. Please retry later.\a");      

      return 2;
      };

   if (argC > 2
       || (argC == 2 && strcmp(argV[1],"?")))
      putc('\a',stderr);
   
   _fstrncpy(path,_pgmptr,sizeof(path)-1);
   path[sizeof(path)-1] = '\0';
   _splitpath(path,drive,dir,fname,ext);
   if (country == 49)
      {
      fprintf(stderr, "%s ist ein Hilfsprogramm\n", path);
      fprintf(stderr, "\tzum Einstellen der Nachlaufzeit von Conner 3´\" AT-Bus-Platten.\n");
      fprintf(stderr, "Aufruf: %s Nachlauf der Festplatte in Minuten\n", fname);
      fprintf(stderr, "\t Minuten zwischen 1 und %u: Abschaltzeit nach Zugriffen\n", MAX_MINUTES);
      fprintf(stderr, "\t Minuten zwischen -1 und -%u: ZusÑtzlich Platte sofort aus\n", MAX_MINUTES);
      fprintf(stderr, "\t Minuten 0: Platte ein, kein autom. Abschalten\n");
      fprintf(stderr, "Hinweise: Getestet unter OS/2 2.0, 1.x und DOS fÅr:\n");
      fprintf(stderr, "\t   CP3024 (20MB), CP3044 (40MB), CP30104 (115MB).\n");
      fprintf(stderr, "\t  NICHT MIT ANDEREN PLATTEN VERWENDEN! VERWENDUNG AUF EIGENE GEFAHR!\n");
      fprintf(stderr, "Version 3 (englisch und deutsch), R.W.Saddey, Berlin; Dez. 92");
      }
   else
      {
      fprintf(stderr, "%s is a utility\n", path);
      fprintf(stderr, "\tto set automatic power-down for Conner 3´ inch IDE-drives.\n");
      fprintf(stderr, "Call: %s Power-down delay (minutes)\n", fname);
      fprintf(stderr, "\tdelay from 1 to %u: Power-down after (minutes) idle time\n", MAX_MINUTES);
      fprintf(stderr, "\tdelay from -1 to -%u: Additionally turn off drive now\n", MAX_MINUTES);
      fprintf(stderr, "\tdelay = 0: Turn on drive, disable auto power-down\n");
      fprintf(stderr, "Notes: Tested with OS/2 2.0, 1.x and DOS for:\n");
      fprintf(stderr, "\tCP3024 (20MB), CP3044 (40MB), CP30104 (115MB).\n");
      fprintf(stderr, "       DO NOT USE WITH OTHER DRIVES! USE AT YOUR OWN RISK ONLY!\n");
      fprintf(stderr, "Version 3 (US & German message), R.W.Saddey, Berlin, Germany; DEC-92");
      };
   return 1;
   };
