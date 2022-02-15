#ifndef OSYSTEM_H_INCLUWDED
#define OSYSTEM_H_INCLUWDED

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <pwd.h>

#include "oexception.h"



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









} // Ende Namespace System



#endif // OSYSTEM_H_INCLUWDED
