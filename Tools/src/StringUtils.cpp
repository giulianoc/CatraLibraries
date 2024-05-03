/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   JSONUtils.cpp
 * Author: giuliano
 *
 * Created on March 29, 2018, 6:27 AM
 */

#include "StringUtils.h"
#include <algorithm>
#include <locale>

string StringUtils::ltrim(string s) {
  auto it = find_if(s.begin(), s.end(), [](char c) {
    return !isspace<char>(c, locale::classic());
  });
  s.erase(s.begin(), it);

  return s;
}

string StringUtils::rtrim(string s) {
  auto it = find_if(s.rbegin(), s.rend(), [](char c) {
    return !isspace<char>(c, locale::classic());
  });
  s.erase(it.base(), s.end());

  return s;
}

string StringUtils::trim(string s) { return ltrim(rtrim(s)); }

string StringUtils::ltrimNewLineToo(string s) {
  auto it = find_if(s.begin(), s.end(), [](char c) {
    return !isspace<char>(c, locale::classic()) && c != '\n';
  });
  s.erase(s.begin(), it);

  return s;
}

string StringUtils::rtrimNewLineToo(string s) {
  auto it = find_if(s.rbegin(), s.rend(), [](char c) {
    return !isspace<char>(c, locale::classic()) && c != '\n';
  });
  s.erase(it.base(), s.end());

  return s;
}

string StringUtils::trimNewLineToo(string s) {
  return ltrimNewLineToo(rtrimNewLineToo(s));
}

string StringUtils::ltrimTabToo(string s) {
  auto it = find_if(s.begin(), s.end(), [](char c) {
    return !isspace<char>(c, locale::classic()) && c != '\t';
  });
  s.erase(s.begin(), it);

  return s;
}

string StringUtils::rtrimTabToo(string s) {
  auto it = find_if(s.rbegin(), s.rend(), [](char c) {
    return !isspace<char>(c, locale::classic()) && c != '\t';
  });
  s.erase(it.base(), s.end());

  return s;
}

string StringUtils::trimTabToo(string s) { return ltrimTabToo(rtrimTabToo(s)); }

string StringUtils::ltrimNewLineAndTabToo(string s) {
  auto it = find_if(s.begin(), s.end(), [](char c) {
    return !isspace<char>(c, locale::classic()) && c != '\n' && c != '\t';
  });
  s.erase(s.begin(), it);

  return s;
}

string StringUtils::rtrimNewLineAndTabToo(string s) {
  auto it = find_if(s.rbegin(), s.rend(), [](char c) {
    return !isspace<char>(c, locale::classic()) && c != '\n' && c != '\t';
  });
  s.erase(it.base(), s.end());

  return s;
}

string StringUtils::trimNewLineAndTabToo(string s) {
  return ltrimNewLineAndTabToo(rtrimNewLineToo(s));
}

bool StringUtils::endWith(string source, string suffix) {

  return (source.size() >= suffix.size() &&
          0 == source.compare(source.size() - suffix.size(), suffix.size(),
                              suffix));
}

bool StringUtils::startWith(string source, string prefix) {
  return (source.size() >= prefix.size() &&
          0 == source.compare(0, prefix.size(), prefix));
}
