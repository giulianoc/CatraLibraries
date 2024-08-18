/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   StringUtils.h
 * Author: giuliano
 *
 * Created on March 29, 2018, 6:27 AM
 */

#ifndef StringUtils_h
#define StringUtils_h

#include <string>

using namespace std;

class StringUtils {

public:
  static string ltrim(string s);
  static string rtrim(string s);
  static string trim(string s);

  static string ltrimNewLineToo(string s);
  static string rtrimNewLineToo(string s);
  static string trimNewLineToo(string s);

  static string ltrimTabToo(string s);
  static string rtrimTabToo(string s);
  static string trimTabToo(string s);

  static string ltrimNewLineAndTabToo(string s);
  static string rtrimNewLineAndTabToo(string s);
  static string trimNewLineAndTabToo(string s);

  static bool endWith(const string &source, const string &suffix);
  static bool startWith(const string &source, const string &prefix);

  static string lowerCase(const string &str);

  static bool isNumber(string text);

  static bool equalCaseInsensitive(const string s1, const string s2);

  // conta quante volte abbiamo pat in txt usando l'algoritmo KMP
  // Time Complexity: O(M + N)
  // Auxiliary Space: O(M) As an array of size M is used to store the longest
  // prefix suffix values for the pattern.
  static int kmpSearch(string pat, string txt);

private:
  // usato da kmpSearch
  static void computeLPSArray(string pat, int M, int lps[]);
};

#endif
