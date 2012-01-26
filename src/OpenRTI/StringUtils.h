/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef OpenRTI_StringUtils_h
#define OpenRTI_StringUtils_h

#include <algorithm>
#include <iosfwd>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "Export.h"

namespace OpenRTI {

typedef std::set<std::string> StringSet;
typedef std::list<std::string> StringList;
typedef std::vector<std::string> StringVector;
typedef std::map<std::string, std::string> StringMap;
typedef std::map<std::string, StringList> StringStringListMap;

inline bool contains(const StringSet& stringSet, const std::string& string)
{ return stringSet.find(string) != stringSet.end(); }
inline bool contains(const StringList& stringList, const std::string& string)
{ return std::find(stringList.begin(), stringList.end(), string) != stringList.end(); }
inline bool contains(const StringVector& stringVector, const std::string& string)
{ return std::find(stringVector.begin(), stringVector.end(), string) != stringVector.end(); }

OPENRTI_API bool
endsWith(const std::string& s, const char* tail);

/// checks for a file case insensitive extension
OPENRTI_API bool
matchExtension(const std::string& name, const char* extension);

/// Internationalization concept:
/// All internal computations are done in ucs{32,16} aka wchar_t
/// Strings are sent over the wire in utf8, that means all the indices are
/// relatively easy to compute, but transferred size is kind of small.
OPENRTI_API std::string
ucsToUtf8(const std::wstring& ucs);

OPENRTI_API std::wstring
utf8ToUcs(const std::string& utf);

OPENRTI_API std::wstring
utf8ToUcs(const char* utf);

OPENRTI_API std::wstring
localeToUcs(const char* loc);

inline std::wstring localeToUcs(const std::string& loc)
{ return localeToUcs(loc.c_str()); }

OPENRTI_API std::string
ucsToLocale(const std::wstring& ucs);

inline std::string localeToUtf8(const std::string& locale)
{ return ucsToUtf8(localeToUcs(locale)); }

inline std::string localeToUtf8(const char* locale)
{ return ucsToUtf8(localeToUcs(locale)); }

inline std::string utf8ToLocale(const std::string& utf)
{ return ucsToLocale(utf8ToUcs(utf)); }

inline std::string utf8ToLocale(const char* utf)
{ return ucsToLocale(utf8ToUcs(utf)); }

OPENRTI_API std::string
asciiToUtf8(const char* ascii);

OPENRTI_API std::vector<std::string>
split(const std::string& s, const char* c = ", \t\n");

inline std::vector<std::string>
split(const char* s, const char* c = ", \t\n")
{
  if (!s)
    return std::vector<std::string>();
  return split(std::string(s), c);
}

OPENRTI_API std::string
trim(std::string s, const char* c = " \t\n");

inline std::string
trim(const char* s, const char* c = " \t\n")
{
  if (!s)
    return std::string();
  return trim(std::string(s), c);
}

OPENRTI_API std::pair<std::string, std::string>
parseInetAddress(const std::string& address);

OPENRTI_API std::string
getFilePart(const std::string& path);

OPENRTI_API std::string
getBasePart(const std::string& path);

OPENRTI_API std::pair<std::string, std::string>
getProtocolAddressPair(const std::string& url);

OPENRTI_API std::pair<std::string, std::string>
getProtocolRestPair(const std::string& url);

inline StringMap
getStringMapFromUrl(const StringMap& defaults, const std::string& url)
{
  /// FIXME, provide some override for interpreting execution names as url
  StringMap stringMap = defaults;
  // preinitialize, for all the fallthrough paths below
  stringMap["url"] = url;
  stringMap["federationExecutionName"] = url;

  /// FIXME extend that!!!

  /// Seperate this into:
  /// <protocol>://<address>/<path>/<name>
  std::string::size_type pos = url.find("://");
  if (pos == std::string::npos)
    return stringMap;

  std::string protocol = url.substr(0, pos);

  std::string::size_type pos0 = pos + 3;
  if (protocol == "trace" && pos0 < url.size()) {
    stringMap = getStringMapFromUrl(defaults, url.substr(pos0));
    stringMap["traceProtocol"] = stringMap["protocol"];
    stringMap["protocol"] = "trace";
    return stringMap;
  }

  // Find the slach terminating the address field
  pos = url.find('/', pos0);
  if (pos == std::string::npos)
    return stringMap;
  std::string address = url.substr(pos0, pos - pos0);

  pos0 = pos;
  pos = url.rfind('/');
  if (pos == std::string::npos)
    return stringMap;
  std::string path = url.substr(pos0, pos - pos0);
  pos0 = pos + 1;
  if (url.size() <= pos0)
    return stringMap;
  std::string name = url.substr(pos0);

  if (protocol  == "rti") {
    // if (!path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse rti://<address>/<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = "rti";
    stringMap["address"] = address;
    stringMap["federationExecutionName"] = name;

  } else if (protocol == "http") {
    // if (!path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse http://<address>/<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = protocol;
    stringMap["address"] = address;
    stringMap["federationExecutionName"] = name;

  } else if (protocol == "pipe") {
    // if (!address.empty() || path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse pipe:///<path to pipe>/<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = protocol;
    stringMap["address"] = path;
    stringMap["federationExecutionName"] = name;
  } else if (protocol == "thread") {
    // if (!address.empty() || !path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse thread:///<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = protocol;
    stringMap["federationExecutionName"] = name;
  }

  return stringMap;
}

inline std::ostream&
operator<<(std::ostream& stream, const std::wstring& s)
{ return stream << ucsToLocale(s); }

} // namespace OpenRTI

#endif
