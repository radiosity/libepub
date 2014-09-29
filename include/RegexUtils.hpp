/*
Copyright (c) 2014, Richard Martin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Richard Martin nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL RICHARD MARTIN BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REGEXUTILS_HEADER
#define REGEXUTILS_HEADER

#include <regex>
#include <iostream>
#include <stdlib.h>

using std::regex_error;
using std::cerr;
using std::endl;
using namespace std::regex_constants;

void print_regex_error(const regex_error & re)
{

	if (re.code() == error_collate) {
		cerr << "The expression contained an invalid collating element name." << endl;
	}
	else if (re.code() == error_ctype) {
		cerr << "The expression contained an invalid character class name." << endl;
	}
	else if (re.code() == error_escape) {
		cerr << "The expression contained an invalid escaped character, or a trailing escape." << endl;
	}
	else if (re.code() == error_backref) {
		cerr << "The expression contained an invalid back reference." << endl;
	}
	else if (re.code() == error_brack) {
		cerr << "The expression contained mismatched brackets ([ and ])" << endl;
	}
	else if (re.code() == error_paren) {
		cerr << "The expression contained mismatched parentheses (( and ))." << endl;
	}
	else if (re.code() == error_brace) {
		cerr << "The expression contained mismatched braces ({ and })." << endl;
	}
	else if (re.code() == error_badbrace) {
		cerr << "The expression contained an invalid range between braces ({ and })." << endl;
	}
	else if (re.code() == error_range) {
		cerr << "The expression contained an invalid character range." << endl;
	}
	else if (re.code() == error_space) {
		cerr << "There was insufficient memory to convert the expression into a finite state machine." << endl;
	}
	else if (re.code() == error_badrepeat) {
		cerr << "The expression contained a repeat specifier (one of *?+{) that was not preceded by a valid regular expression." << endl;
	}
	else if (re.code() == error_complexity) {
		cerr << "The complexity of an attempted match against a regular expression exceeded a pre-set level." << endl;
	}
	else if (re.code() == error_stack) {
		cerr << "There was insufficient memory to determine whether the regular expression could match the specified character sequence." << endl;
	}

}

void print_regex_error_fatal(const regex_error & re)
{

	print_regex_error(re);

	exit(EXIT_FAILURE);

}

#endif