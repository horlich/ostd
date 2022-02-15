#include "osystem.h"

namespace System {



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
      entry.name = pwd->pw_name;
      entry.information = pwd->pw_gecos;
      entry.homedir = pwd->pw_dir;
      entry.shell = pwd->pw_shell;
      entry.uid = pwd->pw_uid;
      entry.gid = pwd->pw_gid;
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













} // Ende Namespace System
