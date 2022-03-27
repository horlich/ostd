#ifndef OSYSTEM_H_INCLUWDED
#define OSYSTEM_H_INCLUWDED

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <pwd.h>
#include <thread>
#include <mntent.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


#include "oexception.h"
#include "ofile.h"



namespace System {



/*------------------------/ UserInfo: /----------------------*/

struct PwdEntry {
   /* Eintrag in /etc/passwd. System-User werden ignoriert */
   std::string name, information, homedir, shell;
   uid_t uid;
   gid_t gid;
};



class UserInfo {
   //
   std::vector<PwdEntry> pvec;

public:
   UserInfo();

   /* Kopier- und Zuweisschutz: */
   UserInfo(const UserInfo&) = delete;
   UserInfo(const UserInfo&&) = delete;
   UserInfo& operator=(const UserInfo&) = delete;
   UserInfo& operator=(const UserInfo&&) = delete;

   inline const std::vector<PwdEntry>& getEntries() const
   {
      return pvec;
   }

   const PwdEntry& getEntry(uid_t uid) const;

   const PwdEntry& getEntry(const std::string& name) const;
};

PwdEntry getUser(uid_t uid);

PwdEntry getUser(const std::string& name);


/*-----------------------/ MountedFsList: /-----------------------*/

const std::array<std::string,4> FSYSTEMS { "ext4", "ext3", "vfat", "fuseblk" };
constexpr char MOUNTTAB_FILE[] {"/proc/mounts"};

struct MtabEntry {
   std::string device, dir, type;
};

class MountedFsList {
   //
   std::vector<MtabEntry> entries;
   int dir_width  = 0;
   int type_width = 0;

public:
   MountedFsList();

   inline const std::vector<MtabEntry>& getEntries()
   {
      return entries;
   }

   inline int getDirCol() const
   {
      return dir_width;
   }

   inline int getTypeCol() const
   {
      return type_width;
   }
};


/*----------------------------/ StatVfs /------------------------------*/

class StatVfs {
   /* siehe statvfs(3) */
   struct statvfs buf;
public:
   StatVfs(const std::string& file);

   /* Filesystem block size */
   inline int getBlockSize() const
   {
      return buf.f_bsize;
   }

   /* Speicherkapazität des Filesystems: */
   inline long long getCapacity() const
   {
      return buf.f_frsize * buf.f_blocks;
   }

   /* ...davon noch frei */
   inline long long getAvailable() const
   {
      return buf.f_frsize * buf.f_bavail;
   }
};


/*--------------------/ ProcStatusMap: /-------------------------*/

/* Liest die Prozessdaten aus /proc/[PID]/status */

struct ProcStatus {
   std::string progName;
   int pid {-1};
   int ppid{-1};
   std::array<int, 4> uid{-1,-1,-1,-1};
   std::vector<int> children;

   inline bool hasChildren()
   {
      return children.empty() == false;
   }
};

using ProcStatusMap = std::map<int, ProcStatus>;

/* ProcStatusMap enthält als Keys die PID's
   und als Values die ProcStatus-Objekte.  */
ProcStatusMap getStatusMap();



/*---------------------/ becomeDaemon: ----------------------*/

/* Adaptierung des Codebeispiels in Kerrisk S.770f */

namespace Daemon {
/* Bit-mask values for 'flags' argument of becomeDaemon() */
   constexpr int NO_CHDIR           {1};     /* Don't chdir("/") */
   constexpr int NO_CLOSE_FILES     {1<<1};  /* Don't close all open files */
   constexpr int NO_REOPEN_STD_FDS  {1<<2};  /* Don't reopen stdin, stdout, and
                                                stderr to /dev/null */
   constexpr int NO_UMASK0          {1<<3};  /* Don't do a umask(0) */
   constexpr int MAX_CLOSE          {8192};  /* Maximum file descriptors to close if
                                       sysconf(_SC_OPEN_MAX) is indeterminate */
} // namespace Daemon

int becomeDaemon(int flags = 0);



/*---------------------/ verschiedene statische Funktionen: ----------------------*/

void humanReadableBytes(long long bytes, std::ostream& os = std::cout);

/* Allgemeine Systemdaten, wie z.B. Anzahl CPU-Kerne... */
void printSystemDaten(std::ostream& os = std::cout);

/* Gibt den Speicherplatz der /home/... Directories aus: */
void printUserSpace(std::ostream& os = std::cout);
/* langsamere Variante mit fork() */
void printUserSpaceFork(std::ostream& os = std::cout);

/* Bildet du nach: */
void printFsSpace(std::ostream& os = std::cout);






/*
 *                     Exceptions:
 *
 * */


class OSystemException : public OException::Fehler {
public:
   OSystemException(const std::string& what);

   virtual ~OSystemException() = default;
};



} // Ende Namespace System



#endif // OSYSTEM_H_INCLUWDED
