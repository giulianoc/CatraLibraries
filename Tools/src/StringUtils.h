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
};

#endif

