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

#ifndef CSS_SELECTOR_HEADER
#define CSS_SELECTOR_HEADER

#include <unordered_set>
#include <vector>
#include <string>

using std::unordered_set;
using std::vector;
using std::string;

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
		unordered_set<string> selector_keys;
		vector<ustring> selector_text;

	public:
		string raw_text;
		CSSSpecificity specificity;

		CSSSelector();
		CSSSelector(string _raw_text);

		CSSSelector(CSSSelector const & cpy);
		CSSSelector(CSSSelector && mv) ;
		CSSSelector & operator =(const CSSSelector & cpy);
		CSSSelector & operator =(CSSSelector && mv) ;

		~CSSSelector();

		friend inline bool operator==(const CSSSelector & lhs, const CSSSelector & rhs);
		friend inline bool operator!=(const CSSSelector & lhs, const CSSSelector & rhs);

		unsigned int count() const;
		bool matches(const ustring & name) const;

};

inline bool operator==(const CSSSelector & lhs, const CSSSelector & rhs)
{
	return lhs.raw_text == rhs.raw_text;
}

inline bool operator!=(const CSSSelector & lhs, const CSSSelector & rhs)
{
	return !(rhs == lhs);
}

#endif