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

#ifndef CSS_RULE_HEADER
#define CSS_RULE_HEADER

class CSSRule; 

#include "CSSDeclaration.hpp"

enum DisplayType {
	DISPLAY_INLINE, //DEFAULT
	DISPLAY_BLOCK
};

enum FontSize {
	FONTSIZE_NORMAL, //Default, but doesn't corespond directly.
	FONTSIZE_LARGER,
	FONTSIZE_SMALLER
};

enum FontWeight {
	FONTWEIGHT_NORMAL, //DEFAULT
	FONTWEIGHT_BOLD
};

enum FontStyle {
	FONTSTYLE_NORMAL, //DEFAULT
	FONTSTYLE_ITALIC
};

enum TextAlign {
	TEXTALIGN_LEFT,  //DEFAULT
	TEXTALIGN_RIGHT,
	TEXTALIGN_CENTER
};

class CSSRule {

	public:
		CSSSelector selector;
		string collation_key;
		map<string, string> raw_pairs;

		CSSRule();
		CSSRule(string selector);

		CSSRule(CSSRule const & cpy);
		CSSRule(CSSRule && mv) ;
		CSSRule & operator =(const CSSRule & cpy);
		CSSRule & operator =(CSSRule && mv) ;

		~CSSRule();

		friend inline bool operator< (const CSSRule & lhs, const CSSRule & rhs);
		friend inline bool operator> (const CSSRule & lhs, const CSSRule & rhs);
		friend inline bool operator<=(const CSSRule & lhs, const CSSRule & rhs);
		friend inline bool operator>=(const CSSRule & lhs, const CSSRule & rhs);

		void add(const CSSRule & rhs);

};

inline bool operator< (const CSSRule & lhs, const CSSRule & rhs)
{
	//Defer to specificity.
	return(lhs.selector.specificity < rhs.selector.specificity);
}
inline bool operator> (const CSSRule & lhs, const CSSRule & rhs)
{
	return rhs < lhs;
}
inline bool operator<=(const CSSRule & lhs, const CSSRule & rhs)
{
	return !(lhs > rhs);
}
inline bool operator>=(const CSSRule & lhs, const CSSRule & rhs)
{
	return !(lhs < rhs);
}

#endif