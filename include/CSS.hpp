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

using Glib::ustring;

using std::map;
using std::vector;

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

};

class CSSClass {

	public:
		ustring name;
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

		CSSClass();
		CSSClass(ustring name);

		CSSClass(CSSClass const & cpy);
		CSSClass(CSSClass && mv) ;
		CSSClass & operator =(const CSSClass & cpy);
		CSSClass & operator =(CSSClass && mv) ;

		~CSSClass();

		void add(const CSSClass & rhs);

};

class CSS {

	public:
		vector <path> files;
		map <ustring, CSSClass> classes;

		CSS();
		CSS(vector<path> files);

		CSS(CSS const & cpy);
		CSS(CSS && mv) ;
		CSS & operator =(const CSS & cpy);
		CSS & operator =(CSS && mv);

		CSSClass get_class(const ustring & name) const;

		~CSS();

		void save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index);

};

#endif