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

#ifndef CSS_HEADER
#define CSS_HEADER

#include <boost/filesystem.hpp>
#include <glibmm.h>
#include <set>
#include <map>
#include <vector>
#include <unordered_set>
#include <sqlite3.h>
#include <string>

using Glib::ustring;

using std::multiset;
using std::map;
using std::vector;
using std::unordered_set;
using std::string;

using namespace boost::filesystem;

#include "css/CSSSpecificity.hpp"
#include "css/CSSSelector.hpp"


/*
What do I care about here?

CLASS NAME

display: block;
	This should always cause an indent before and after.

display: inline;
	Should stay with the text.

font-size: 1em;
	Should be able to say that, if it's more than 1em it gets <big>,
	less it gets small? Not so sure about this one

font-weight: bold;
	obvious

font-style: italic
	obvious;

MARGINS :(

margin-left : IGNORE
margin-right : IGNORE
margin-top : take percentage
margin-bottom : take percentage.

SHOULD ALSO DO

*/

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

enum CSSValueType {
	CSS_VALUE_DEFAULT,
	CSS_VALUE_PX,
	CSS_VALUE_PT,
	CSS_VALUE_CM,
	CSS_VALUE_EM,
	CSS_VALUE_PERCENT
};

class CSSValue {

	public:
		double value;
		CSSValueType type;

		CSSValue();

		CSSValue(CSSValue const & cpy);
		CSSValue(CSSValue && mv) ;
		CSSValue & operator =(const CSSValue & cpy);
		CSSValue & operator =(CSSValue && mv) ;

		~CSSValue();

		inline bool operator==(const CSSValue & rhs) {
			if(value != rhs.value) {
				return false;
			}

			if(type != rhs.type) {
				return false;
			}

			return true;
		}

};

class CSSRule {

	public:
		CSSSelector selector;
		string collation_key;
		map<string, string> raw_pairs;
		DisplayType displaytype;
		FontSize fontsize;
		FontWeight fontweight;
		FontStyle fontstyle;
		CSSValue margintop;
		CSSValue marginbottom;
		bool pagebreakbefore;
		bool pagebreakafter;
		TextAlign textalign;
		CSSValue textindent;

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

class CSS {

	public:
		vector <path> files;
		multiset <CSSRule> rules;

		CSS();
		CSS(vector<path> files);
		CSS(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index);

		CSS(CSS const & cpy);
		CSS(CSS && mv) ;
		CSS & operator =(const CSS & cpy);
		CSS & operator =(CSS && mv);

		CSSRule get_rule(const ustring & selector) const;
		bool contains_rule(const ustring & selector) const;

		~CSS();

		void save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index);

};

#endif