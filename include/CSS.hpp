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
#include <map>
#include <vector>
#include <sqlite3.h>
#include <string>

using Glib::ustring;

using std::map;
using std::vector;
using std::string;

using namespace boost::filesystem;

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

class CSSSpecificity {

	private:
		unsigned int a;
		unsigned int b;
		unsigned int c;
		unsigned int d;

	public:
		CSSSpecificity();
		CSSSpecificity(const unsigned int _a, const unsigned int _b, const unsigned int _c, const unsigned int _d);

		CSSSpecificity(CSSSpecificity const & cpy);
		CSSSpecificity(CSSSpecificity && mv) ;
		CSSSpecificity & operator =(const CSSSpecificity & cpy);
		CSSSpecificity & operator =(CSSSpecificity && mv) ;

		~CSSSpecificity();

		friend inline bool operator==(const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator!=(const CSSSpecificity & lhs, const CSSSpecificity & rhs);

		friend inline bool operator< (const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator> (const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator<=(const CSSSpecificity & lhs, const CSSSpecificity & rhs);
		friend inline bool operator>=(const CSSSpecificity & lhs, const CSSSpecificity & rhs);

};

inline bool operator==(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	if(lhs.d != rhs.d) {
		return false;
	}

	if(lhs.c != rhs.c) {
		return false;
	}

	if(lhs.b != rhs.b) {
		return false;
	}

	if(lhs.a != rhs.a) {
		return false;
	}

	return true;
}

inline bool operator!=(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return !(rhs == lhs);
}

inline bool operator< (const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	// Is 1,0,0,0 less than 0,1,0,0? No.
	// Is 0,1,0,0 less than 1,0,0,0? Yes

	// Check greater or less for a, then b etc.
	// Only cascade through if the values are equal.

	if(lhs.a < rhs.a) {
		return true;
	}

	if(lhs.a > rhs.a) {
		return false;
	}

	if(lhs.b < rhs.b) {
		return true;
	}

	if(lhs.b > rhs.b) {
		return false;
	}

	if(lhs.c < rhs.c) {
		return true;
	}

	if(lhs.c > rhs.c) {
		return false;
	}

	if(lhs.d < rhs.d) {
		return true;
	}

	if(lhs.d > rhs.d) {
		return false;
	}

	return false;

}
inline bool operator> (const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return rhs < lhs;
}
inline bool operator<=(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return !(lhs > rhs);
}
inline bool operator>=(const CSSSpecificity & lhs, const CSSSpecificity & rhs)
{
	return !(lhs < rhs);
}

class CSSSelector {

		/*
		For this thing, http://www.w3.org/TR/WD-css2-971104/selector.html
		will be my bible.

		There are many types of selector.

		Selector:
			p { padding: 10px; }

		Class Selector:
			p.section { padding: 10px; } //Only applies to p elements
			.bold { font-weight: bold; }

		ID selector:
			#section { padding: 10px; } 	// Can only apply once.
									NOT GOING TO ENFORCE THIS

		Contextual selector:
			p span { font-style: italic; } // all span-elements within a p-element

		h1, h2, h3 { font-family: sans-serif } is equivalent to:
		h1 { font-family: sans-serif }
		h2 { font-family: sans-serif }
		h3 { font-family: sans-serif }

		#foo td, th {
			? declarations
		}

		#foo td {
			? declarations
		}
		th {
			? declarations
		}

		*/

	private:
		string raw_text;
		vector<string> selector_keys;
		vector<ustring> selector_text;

	public:
		CSSSpecificity specificity;

		CSSSelector(string _raw_text);

		CSSSelector(CSSSelector const & cpy);
		CSSSelector(CSSSelector && mv) ;
		CSSSelector & operator =(const CSSSelector & cpy);
		CSSSelector & operator =(CSSSelector && mv) ;

		~CSSSelector();

		unsigned int count();

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
		ustring selector;
		string collation_key;
		map<ustring, ustring> raw_pairs;
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
		CSSRule(ustring selector);

		CSSRule(CSSRule const & cpy);
		CSSRule(CSSRule && mv) ;
		CSSRule & operator =(const CSSRule & cpy);
		CSSRule & operator =(CSSRule && mv) ;

		~CSSRule();

		void add(const CSSRule & rhs);

};

class CSS {

	public:
		vector <path> files;
		map <string, CSSRule> rules;

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