#include "osystem.h"

namespace System {


/*-------------------------/ UserInfo: /----------------------*/

void fillPwdEntry__(struct passwd* pwd, PwdEntry* entry)
{
   entry->name = pwd->pw_name;
   entry->information = pwd->pw_gecos;
   entry->homedir = pwd->pw_dir;
   entry->shell = pwd->pw_shell;
   entry->uid = pwd->pw_uid;
   entry->gid = pwd->pw_gid;
}


UserInfo::UserInfo()
{
   struct passwd* pwd;

   while ((pwd = getpwent()) != nullptr) {
      uid_t uid = pwd->pw_uid;
      if ((uid > 0) && (uid < 1000)) continue;
      PwdEntry entry;
      /* struct pwd kann nicht direkt verwendet werden,
         weil dieser seine char*-Werte jedesmal überschreibt.
         Deshalb müssen alle Werte nach PwdEntry kopiert
         werden: */
      fillPwdEntry__(pwd, &entry);
      pvec.push_back(std::move(entry)); /* entry darf nicht mehr verwendet werden! */
   }
   endpwent();
}


const PwdEntry& UserInfo::getEntry(uid_t uid) const
{
   auto it = std::find_if(pvec.begin(), pvec.end(), [&] (const PwdEntry e) {
      return (e.uid == uid);
   });
   if (it == pvec.end()) {
      std::stringstream buf;
      buf << __PRETTY_FUNCTION__ << ": UID " << uid << " existiert nicht.";
      throw OException::IndexOutOfBoundsException(buf.str());
   }
   return *it;
}


const PwdEntry& UserInfo::getEntry(const std::string& name) const
{
   auto it = std::find_if(pvec.begin(), pvec.end(), [&] (const PwdEntry e) {
      return (e.name == name);
   });
   if (it == pvec.end()) {
      std::stringstream buf;
      buf << __PRETTY_FUNCTION__ << ": User " << name << " existiert nicht.";
      throw OException::IndexOutOfBoundsException(buf.str());
   }
   return *it;
}


PwdEntry getUser(uid_t uid)
{
   struct passwd* pwd = getpwuid(uid);
   if (pwd == nullptr) {
      std::stringstream buf;
      buf << __PRETTY_FUNCTION__ << ": UID " << uid << " existiert nicht.";
      throw OException::IndexOutOfBoundsException(buf.str());
   }
   PwdEntry ret;
   fillPwdEntry__(pwd, &ret);
   return ret;
}


PwdEntry getUser(const std::string& name)
{
   struct passwd* pwd = getpwnam(name.c_str());
   if (pwd == nullptr) {
      std::stringstream buf;
      buf << __PRETTY_FUNCTION__ << ": User " << name << " existiert nicht.";
      throw OException::IndexOutOfBoundsException(buf.str());
   }
   PwdEntry ret;
   fillPwdEntry__(pwd, &ret);
   return ret;
}



/*---------------------------/ StatVfs /------------------------------*/

StatVfs::StatVfs(const std::string& file)
{
   if (statvfs(file.c_str(), &buf) != 0)
      throw System::OSystemException("Aufruf von statvfs() mißlungen.");
}



/*-------------------------/ MountedFsList: /----------------------*/

MountedFsList::MountedFsList()
{
   FILE* stream = setmntent(MOUNTTAB_FILE, "r");
   struct mntent* content;
   while ((content = getmntent(stream))) {
      if (! std::any_of(FSYSTEMS.begin(), FSYSTEMS.end(), [&] (std::string type) {
      return (type == content->mnt_type);
      })) continue;
      MtabEntry entry;
      entry.dir = content->mnt_dir;
      entry.device = content->mnt_fsname;
      entry.type = content->mnt_type;
      dir_width = std::max(dir_width, static_cast<int>(entry.dir.length()));
      type_width = std::max(type_width, static_cast<int>(entry.type.length()));
      entries.push_back(std::move(entry));
   }
   endmntent(stream);
}



/*---------------------------/ ProcStatusMap: /------------------------------*/

namespace fs = std::filesystem;
using PidMap = std::map<int, fs::path>;

void setName__(const std::string& str, ProcStatus& ps)
{
   for (char c : str) {
      if (isspace(c)) {
         if (ps.progName.empty()) {
            continue; /* Führende WS */
         }
         else {
            break; /* String schon eingelesen */
         }
      }
      else {
         ps.progName.push_back(c);
      }
   }
}

void setPPid__(const std::string& str, ProcStatus& ps)
{
   ps.ppid = std::stoi(str);
}

void setPid__(const std::string& str, ProcStatus& ps)
{
   ps.pid = std::stoi(str);
}

void setUid__(std::string str, ProcStatus& ps)
{
   size_t idx = 0;
   int uidsize = ps.uid.size();
   for (int i = 0; i < uidsize; ++i) {
      ps.uid[i] = std::stoi(str, &idx);
      str = str.substr(idx, str.length());
   }
}


/* Gib eine Map aus mit den PID's und den
   dazugehörigen /proc/PID-Pfaden:     */
PidMap getPidMap__()
{
   static constexpr char procpath[] {"/proc"};
   PidMap ret;

   for (const fs::directory_entry& entry : fs::directory_iterator(procpath)) {
      const fs::path& epath = entry.path();
      const std::string fn = epath.filename().string();
      bool is_int = true;
      for (char ch : fn) {
         if (! std::isdigit(ch)) {
            is_int = false;
            break;
         }
      }
      if (! is_int) continue;
      ret[std::stoi(fn)] = epath;
   }
   return ret;
}

/* Lies die Datei /proc/PID/status ein, erzeuge ein
   ProcStatus-Objekt und füge es der ProcStatusMap hinzu: */
void readStatus__(fs::path p, ProcStatusMap& smap)
{
   p /= "status";
   if (! fs::exists(p)) return;
   std::ifstream is(p);
   static constexpr int bufsize{300};
   char buf[bufsize+1];
   ProcStatus pstatus;
   int treffer = 0;
   while (is.getline(buf,bufsize)) {
      if (treffer >= 4) break; /* Daten wurden bereits vollständig eingelesen */
      std::string line(buf);
      /* Namen einlesen versuchen: */
      if (line.find("Name:") == 0) { /* TODO: alle find() durch begins_with() ersetzen! */
         setName__(line.substr(5, line.length()), pstatus);
         ++treffer;
      }
      else if (line.find("Uid:") == 0) {
         setUid__(line.substr(4, line.length()), pstatus);
         ++treffer;
      }
      else if (line.find("PPid:") == 0) {
         setPPid__(line.substr(5, line.length()), pstatus);
         ++treffer;
      }
      else if (line.find("Tgid:") == 0) {
         setPid__(line.substr(5, line.length()), pstatus);
         ++treffer;
      }
   }
   is.close();
   int parent = pstatus.ppid;
   if (parent > 0) {
      smap[parent].children.push_back(pstatus.pid);
   }
   smap[pstatus.pid] = std::move(pstatus); /* pstatus NICHT mehr verwenden! */
}


ProcStatusMap getStatusMap()
{
   ProcStatusMap smap;
   PidMap pmap = getPidMap__();
   for(auto pidpath : pmap) {
      readStatus__(pidpath.second, smap);
   }
   return smap;
}


/*------------------------/ becomeDaemon: /-------------------------*/


int                                     /* Returns 0 on success, -1 on error */
becomeDaemon(int flags)
{
    int maxfd, fd;

    switch (fork()) {                   /* Become background process */
    case -1: return -1;
    case 0:  break;                     /* Child falls through... */
    default: _exit(EXIT_SUCCESS);       /* while parent terminates */
    }

    if (setsid() == -1)                 /* Become leader of new session */
        return -1;

    switch (fork()) {                   /* Ensure we are not session leader */
    case -1: return -1;
    case 0:  break;
    default: _exit(EXIT_SUCCESS);
    }

    if (!(flags & Daemon::NO_UMASK0))
        umask(0);                       /* Clear file mode creation mask */

    if (!(flags & Daemon::NO_CHDIR))
        if (chdir("/") != 0) {/* Compilerwarnung umgehen */}  /* Change to root directory */

    if (!(flags & Daemon::NO_CLOSE_FILES)) { /* Close all open files */
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)                /* Limit is indeterminate... */
            maxfd = Daemon::MAX_CLOSE;       /* so take a guess */

        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    }
    if (!(flags & Daemon::NO_REOPEN_STD_FDS)) {
        close(STDIN_FILENO);            /* Reopen standard fd's to /dev/null */

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)         /* 'fd' should be 0 */
            return -1;
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }
    return 0;
}



/*------------------------/ statische Funktionen: /-------------------------*/

void printSystemDaten(std::ostream& os)
{
   struct utsname uts_info;
   uname(&uts_info);
   os <<   "Architektur:     " << uts_info.machine
      << "\nCPU-Kerne:       " << std::thread::hardware_concurrency()
      << "\nBetriebssystem:  " << uts_info.sysname
      << "\nVersion:         " << uts_info.version
      << "\nKernel:          " << uts_info.release
      << "\nHostname:        " << uts_info.nodename;
   char buf[L_ctermid]; /* siehe ctermid(3) */
   ctermid(buf);
   os << "\nContr. Terminal: " << buf;
   pid_t fpg = tcgetpgrp(STDIN_FILENO);
   os << "\nTerminal PGID:   " << fpg
      << "\nSession ID:      " << getsid(0) << '\n';
}


void printDirSize__(const std::string& path, std::ostream& os)
{
   try {
      OFile::GetSize sz(path);
      os << std::setw(18) << path << std::setw(8) << sz << "\n";
   }
   catch (std::exception& e) {
      os << "Verzeichnis '" << path << "' konnte nicht gelesen werden.\n";
   }
}


void printUserSpace(std::ostream& os)
{
   System::UserInfo uinfo;
   std::vector<std::thread*> threads;
   for (System::PwdEntry entry : uinfo.getEntries()) {
      if (! std::filesystem::exists(entry.homedir)) continue;
      /* zu std::ref(os) siehe:
         https://stackoverflow.com/questions/61985888/why-the-compiler-complains-that-stdthread-arguments-must-be-invocable-after-co */
      std::thread* th = new std::thread(printDirSize__, entry.homedir, std::ref(os));
      threads.push_back(th);
   }
   for (auto th : threads) {
      th->join();
      delete th;
   }
}


void printUserSpaceFork(std::ostream& os)
{
   System::UserInfo uinfo;
   for (System::PwdEntry entry : uinfo.getEntries()) {
      if (! std::filesystem::exists(entry.homedir)) continue;
      switch (fork()) {
      case -1:
         throw OSystemException("fork()");
      case 0:
         printDirSize__(entry.homedir, os);
         exit(EXIT_SUCCESS);
      default:
         wait(nullptr);
         break;
      }
   }
}



void humanReadableBytes(long long bytes, std::ostream& os)
{
   if (bytes < 1e3) {
      os << bytes << " Bytes";
      return;
   }
   auto oldf = os.setf(std::ios::fixed, std::ios::floatfield);
   auto oldp = os.precision(1);
   double quotient;
   if (bytes < 1e6) {
      quotient = bytes / 1.0e3;
      os << quotient << " K";
   }
   else if (bytes < 1e9) {
      quotient = bytes / 1.0e6;
      os << quotient << " M";
   }
   else {
      quotient = bytes / 1.0e9;
      os << quotient << " G";
   }
   os.setf(oldf);
   os.precision(oldp);
}


void printFsSpace(std::ostream& os)
{
   MountedFsList mlist;
   for (MtabEntry entry : mlist.getEntries()) {
      StatVfs sstat(entry.dir);
      std::string type = '(' + entry.type + ')';
      os << std::setw(mlist.getDirCol() + 2) << entry.dir << std::setw(mlist.getTypeCol() + 3) << type << " Frei: " << std::setw(6);
      System::humanReadableBytes(sstat.getAvailable(), os);
      os << " von " << std::setw(6);
      System::humanReadableBytes(sstat.getCapacity());
      os << "\n";
   }
}


/*-----------------------/ Exceptions: /--------------------------*/

OSystemException::OSystemException(const std::string& what) : Fehler(what) {}


} // Ende Namespace System
