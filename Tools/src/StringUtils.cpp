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

bool StringUtils::endWith(const string &source, const string &suffix) {

  return (source.size() >= suffix.size() &&
          0 == source.compare(source.size() - suffix.size(), suffix.size(),
                              suffix));
}

bool StringUtils::startWith(const string &source, const string &prefix) {
  return (source.size() >= prefix.size() &&
          0 == source.compare(0, prefix.size(), prefix));
}

string StringUtils::lowerCase(const string &str) {
  string lowerCase;
  lowerCase.resize(str.size());
  transform(str.begin(), str.end(), lowerCase.begin(),
            [](unsigned char c) { return tolower(c); });

  return lowerCase;
}

bool StringUtils::isNumber(string text) {
  return !text.empty() &&
         ranges::find_if(text.begin(), text.end(), [](unsigned char c) {
           return !isdigit(c);
         }) == text.end();
}

bool StringUtils::equalCaseInsensitive(const string s1, const string s2) {
  return s1.length() != s2.length()
             ? false
             : equal(s1.begin(), s1.end(), s2.begin(),
                     [](int c1, int c2) { return toupper(c1) == toupper(c2); });
}

int StringUtils::kmpSearch(string pat, string txt) {
  int M = pat.length();
  int N = txt.length();

  // Create lps[] that will hold the longest
  // prefix suffix values for pattern
  int lps[M];
  int j = 0; // index for pat[]

  // Preprocess the pattern (calculate lps[]
  // array)
  computeLPSArray(pat, M, lps);

  int i = 0; // index for txt[]
  int res = 0;
  int next_i = 0;

  while (i < N) {
    if (pat[j] == txt[i]) {
      j++;
      i++;
    }
    if (j == M) {

      // When we find pattern first time,
      // we iterate again to check if there
      // exists more pattern
      j = lps[j - 1];
      res++;
    }

    // Mismatch after j matches
    else if (i < N && pat[j] != txt[i]) {

      // Do not match lps[0..lps[j-1]]
      // characters, they will match anyway
      if (j != 0)
        j = lps[j - 1];
      else
        i = i + 1;
    }
  }
  return res;
}

void StringUtils::computeLPSArray(string pat, int M, int lps[]) {

  // Length of the previous longest
  // prefix suffix
  int len = 0;
  int i = 1;
  lps[0] = 0; // lps[0] is always 0

  // The loop calculates lps[i] for
  // i = 1 to M-1
  while (i < M) {
    if (pat[i] == pat[len]) {
      len++;
      lps[i] = len;
      i++;
    } else // (pat[i] != pat[len])
    {

      // This is tricky. Consider the example.
      // AAACAAAA and i = 7. The idea is similar
      // to search step.
      if (len != 0) {
        len = lps[len - 1];

        // Also, note that we do not
        // increment i here
      } else // if (len == 0)
      {
        lps[i] = len;
        i++;
      }
    }
  }
}
