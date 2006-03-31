/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is jprof.
 *
 * The Initial Developer of the Original Code is
 * Kipp E.B. Hickman.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __leaky_h_
#define __leaky_h_

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "libmalloc.h"
#include "strset.h"
#include "intcnt.h"

typedef unsigned int u_int;

struct Symbol;
struct leaky;

class FunctionCount : public IntCount
{
public:
    void printReport(FILE *fp, leaky *lk);
};

struct Symbol {
  char* name;
  u_long address;
  int    timerHit;
  FunctionCount cntP, cntC;

  int regChild(int id) {return cntC.countAdd(id, 1);}
  int regParrent(int id) {return cntP.countAdd(id, 1);}

  Symbol() : timerHit(0) {}
  void Init(const char* aName, u_long aAddress) {
    name = aName ? strdup(aName) : (char *)"";
    address = aAddress;
  }
};

struct LoadMapEntry {
  char* name;			// name of .so
  u_long address;		// base address where it was mapped in
  LoadMapEntry* next;
};

struct leaky {
  leaky();
  ~leaky();

  void initialize(int argc, char** argv);
  void open();

  char*  applicationName;
  char*  logFile;
  char*  progFile;

  int   quiet;
  int   showAddress;
  u_int  stackDepth;

  int   mappedLogFile;
  malloc_log_entry* firstLogEntry;
  malloc_log_entry* lastLogEntry;

  int    stacks;

  int sfd;
  Symbol* externalSymbols;
  int     usefulSymbols;
  int     numExternalSymbols;
  StrSet exclusions;
  u_long lowestSymbolAddr;
  u_long highestSymbolAddr;

  LoadMapEntry* loadMap;

  StrSet roots;
  StrSet includes;

  void usageError();

  void LoadMap();

  void analyze();

  void dumpEntryToLog(malloc_log_entry* lep);

  void insertAddress(u_long address, malloc_log_entry* lep);
  void removeAddress(u_long address, malloc_log_entry* lep);

  void displayStackTrace(FILE* out, malloc_log_entry* lep);

  void ReadSymbols(const char* fileName, u_long aBaseAddress);
  void ReadSharedLibrarySymbols();
  void setupSymbols(const char* fileName);
  Symbol* findSymbol(u_long address);
  bool excluded(malloc_log_entry* lep);
  bool included(malloc_log_entry* lep);
  const char* indexToName(int idx) {return externalSymbols[idx].name;}

  private:
  void generateReportHTML(FILE *fp, int *countArray, int count);
  int  findSymbolIndex(u_long address);
};

#endif /* __leaky_h_ */
