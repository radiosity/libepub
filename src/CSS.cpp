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

#include "CSS.hpp"

#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include <regex>
#include <limits>

#include "SQLiteUtils.hpp"
#include "RegexUtils.hpp"

using std::string;
using std::move;
using std::ifstream;
using std::pair;
using std::regex;
using std::regex_error;
using std::regex_search;
using std::regex_match;
using std::sregex_iterator;
using std::smatch;
using std::numeric_limits;
using std::stod;

//#ifdef DEBUG
#include <iostream>
using std::cout;
using std::endl;
//#endif

namespace {

	regex regex_selector_split;
	regex regex_id_compound;
	regex regex_id_single;
	regex regex_class_compound;
	regex regex_class_single;
	regex regex_contextual;

	bool regex_ready = false;

	void _selector_regex_initialise()
	{

		//Prepare the regular expressions for the CSSSelector:
		try {
			regex_selector_split = regex("\\s*([a-zA-Z0-9.#\\s]+),*", regex::optimize);
			regex_id_compound = regex("^[a-zA-Z0-9]+#[a-zA-Z0-9-]+", regex::optimize);
			regex_id_single = regex("^#[a-zA-Z0-9-]+", regex::optimize);
			regex_class_compound = regex("^[a-zA-Z0-9]+\\.[a-zA-Z0-9-]+", regex::optimize);
			regex_class_single = regex("^\\.[a-zA-Z0-9-]+", regex::optimize);
			regex_contextual = regex("^[a-zA-Z0-9]+\\s[a-zA-Z0-9-]+", regex::optimize);
		}
		catch (regex_error re) {
			cout << "You dun goofed " << endl;

			print_regex_error_fatal(re);
		}

		regex_ready = true;

	}

}

CSSSpecificity::CSSSpecificity() :
	CSSSpecificity(0, 0, 0, 0)
{

}

CSSSpecificity::CSSSpecificity(const unsigned int _a, const unsigned int _b, const unsigned int _c, const unsigned int _d) :
	a(_a),
	b(_b),
	c(_c),
	d(_d)
{

}

CSSSpecificity::CSSSpecificity(CSSSpecificity const & cpy) :
	a(cpy.a),
	b(cpy.b),
	c(cpy.c),
	d(cpy.d)
{

}

CSSSpecificity::CSSSpecificity(CSSSpecificity && mv) :
	a(move(mv.a)),
	b(move(mv.b)),
	c(move(mv.c)),
	d(move(mv.d))
{

}

CSSSpecificity & CSSSpecificity::operator =(const CSSSpecificity & cpy)
{
	a = cpy.a;
	b = cpy.b;
	c = cpy.c;
	d = cpy.d;
	return *this;
}

CSSSpecificity & CSSSpecificity::operator =(CSSSpecificity && mv)
{
	a = move(mv.a);
	b = move(mv.b);
	c = move(mv.c);
	d = move(mv.d);
	return *this;
}

CSSSpecificity::~CSSSpecificity()
{

}

CSSSelector::CSSSelector() :
	CSSSelector("")
{

}

CSSSelector::CSSSelector(const string _raw_text) :
	selector_keys(),
	selector_text(),
	raw_text(_raw_text),
	specificity()
{

	if(raw_text.empty()) {
		return;
	}

	if(!regex_ready) {
		_selector_regex_initialise();
	}

	unsigned int a  = 0;
	unsigned int b  = 0;
	unsigned int c  = 0;
	unsigned int d  = 0;

	smatch regex_matches;
	auto line_begin = sregex_iterator(raw_text.begin(), raw_text.end(), regex_selector_split);
	auto line_end = sregex_iterator();
	selector_keys.reserve(distance(line_begin, line_end));
	selector_text.reserve(distance(line_begin, line_end));

	for (sregex_iterator i = line_begin; i != line_end; ++i) {
		smatch match = *i;
		string selector = match[1];

		if(regex_match(selector, regex_id_compound)) {
			b++;
			d++;
		}
		else if(regex_match(selector, regex_id_single)) {
			b++;
		}
		else if(regex_match(selector, regex_class_compound)) {
			c++;
			d++;
		}
		else if(regex_match(selector, regex_class_single)) {
			c++;
		}
		else if(regex_match(selector, regex_contextual)) {
			d += 2;
		}
		else {
			d++;
		}

		ustring ustr_selector(selector);
		selector_keys.insert(ustr_selector.collate_key());
		selector_text.push_back(ustr_selector);
		#ifdef DEBUG
		cout << "\tCSS Selector: "  << selector << endl;
		#endif
	}

	specificity  = CSSSpecificity(a, b, c, d);

}

CSSSelector::CSSSelector(CSSSelector const & cpy) :
	selector_keys(cpy.selector_keys),
	selector_text(cpy.selector_text),
	raw_text(cpy.raw_text),
	specificity(cpy.specificity)
{

}

CSSSelector::CSSSelector(CSSSelector && mv) :
	selector_keys(move(mv.selector_keys)),
	selector_text(move(mv.selector_text)),
	raw_text(move(mv.raw_text)),
	specificity(move(mv.specificity))
{

}

CSSSelector & CSSSelector::operator =(const CSSSelector & cpy)
{
	selector_keys = cpy.selector_keys,
	selector_text = cpy.selector_text;
	raw_text = cpy.raw_text;
	specificity = cpy.specificity;
	return *this;
}

CSSSelector & CSSSelector::operator =(CSSSelector && mv)
{
	selector_keys = move(mv.selector_keys);
	selector_text = move(mv.selector_text);
	raw_text = move(mv.raw_text);
	specificity = move(mv.specificity);
	return *this;
}

CSSSelector::~CSSSelector()
{

}

unsigned int CSSSelector::count() const
{
	return selector_keys.size();
}

bool CSSSelector::matches(const ustring & name) const
{

	string key = name.collate_key();

	return selector_keys.count(key) > 0;

}

CSSValue::CSSValue() :
	value(numeric_limits<double>::min()),
	type(CSS_VALUE_DEFAULT)
{
}

CSSValue::CSSValue(CSSValue const & cpy) :
	value(cpy.value),
	type(cpy.type)
{
}

CSSValue::CSSValue(CSSValue && mv) :
	value(move(mv.value)),
	type(move(mv.type))
{
}

CSSValue & CSSValue::operator =(const CSSValue & cpy)
{
	value = cpy.value;
	type = cpy.type;
	return *this;
}

CSSValue & CSSValue::operator =(CSSValue && mv)
{
	value = move(mv.value);
	type = move(mv.type);
	return *this;
}

CSSValue::~CSSValue()
{
}


CSSDeclaration::CSSDeclaration(CSSPropertyType _type, shared_ptr<CSSValueType> _value, CSSSpecificity _specificity, weak_ptr<CSSRule> _parent) :
	type(_type),
	value(_value),
	specificity(_specificity),
	parent(_parent)
{
	
}

CSSDeclaration::CSSDeclaration(CSSDeclaration const & cpy) :
	type(cpy.type),
	value(cpy.value),
	specificity(cpy.specificity),
	parent(cpy.parent)
{
	
}

CSSDeclaration::CSSDeclaration(CSSDeclaration && mv) :
	type(move(mv.type)),
	value(move(mv.value)), 
	specificity(move(mv.specificity)),
	parent(move(mv.parent))
{
	
}

CSSDeclaration & CSSDeclaration::operator =(const CSSDeclaration & cpy) {
	type = cpy.type; 
	value = cpy.value; 
	specificity = cpy.specificity;
	parent = cpy.parent;
	return *this; 
}

CSSDeclaration & CSSDeclaration::operator =(CSSDeclaration && mv)  {
	type = move(mv.type);
	value = move(mv.value); 
	specificity = move(mv.specificity);
	parent = move(mv.parent);
	return *this; 
}

CSSDeclaration::~CSSDeclaration() {
}



CSSRule::CSSRule() :
	CSSRule("")
{
}

CSSRule::CSSRule(string _selector) :
	selector(_selector),
	collation_key(ustring(_selector).collate_key()),
	raw_pairs(),
	displaytype(DISPLAY_INLINE),
	fontsize(FONTSIZE_NORMAL),
	fontweight(FONTWEIGHT_NORMAL),
	fontstyle(FONTSTYLE_NORMAL),
	margintop(),
	marginbottom(),
	pagebreakbefore(false),
	pagebreakafter(false),
	textalign(TEXTALIGN_LEFT),
	textindent()
{
}

CSSRule::CSSRule(CSSRule const & cpy) :
	selector(cpy.selector),
	collation_key(cpy.collation_key),
	raw_pairs(cpy.raw_pairs),
	displaytype(cpy.displaytype),
	fontsize(cpy.fontsize),
	fontweight(cpy.fontweight),
	fontstyle(cpy.fontstyle),
	margintop(cpy.margintop),
	marginbottom(cpy.marginbottom),
	pagebreakbefore(cpy.pagebreakbefore),
	pagebreakafter(cpy.pagebreakafter),
	textalign(cpy.textalign),
	textindent(cpy.textindent)
{
}

CSSRule::CSSRule(CSSRule && mv) :
	selector(move(mv.selector)),
	collation_key(move(mv.collation_key)),
	raw_pairs(move(mv.raw_pairs)),
	displaytype(move(mv.displaytype)),
	fontsize(move(mv.fontsize)),
	fontweight(move(mv.fontweight)),
	fontstyle(move(mv.fontstyle)),
	margintop(move(mv.margintop)),
	marginbottom(move(mv.marginbottom)),
	pagebreakbefore(move(mv.pagebreakbefore)),
	pagebreakafter(move(mv.pagebreakafter)),
	textalign(move(mv.textalign)),
	textindent(move(mv.textindent))
{
}

CSSRule & CSSRule::operator =(const CSSRule & cpy)
{
	selector = cpy.selector;
	collation_key = cpy.collation_key;
	raw_pairs = cpy.raw_pairs;
	displaytype = cpy.displaytype;
	fontsize = cpy.fontsize;
	fontweight = cpy.fontweight;
	fontstyle = cpy.fontstyle;
	margintop = cpy.margintop;
	marginbottom = cpy.marginbottom;
	pagebreakbefore = cpy.pagebreakbefore;
	pagebreakafter = cpy.pagebreakafter;
	textalign = cpy.textalign;
	textindent = cpy.textindent;
	return *this;
}

CSSRule & CSSRule::operator =(CSSRule && mv)
{
	selector = move(mv.selector);
	collation_key = move(mv.collation_key);
	raw_pairs = move(mv.raw_pairs);
	displaytype = move(mv.displaytype);
	fontsize = move(mv.fontsize);
	fontweight = move(mv.fontweight);
	fontstyle = move(mv.fontstyle);
	margintop = move(mv.margintop);
	marginbottom = move(mv.marginbottom);
	pagebreakbefore = move(mv.pagebreakbefore);
	pagebreakafter = move(mv.pagebreakafter);
	textalign = move(mv.textalign);
	textindent = move(mv.textindent);
	return *this;
}

CSSRule::~CSSRule() { }

void CSSRule::add ( const CSSRule & rhs )
{
	//Once we add one rule to another, it becomes
	//a derived rule. There is no selector that would
	//identify it. Therefore we should ensure that the selector is
	//voided.

	//TODO: do this without killing specificity.
	//selector = CSSSelector();

	//So. How should rules be added together?
	//
	// Go through all their declarations.
	//
	// If the declaration is set on the RHS,
	// but not set on this rule, then adopt it
	// regardless of specificity.
	//
	// If the declaration is set on the RHS
	// and also on this rule, then check
	// specificity before adopting it.

	//Do the basics:
	if(rhs.displaytype != DISPLAY_INLINE) {
		if(displaytype != DISPLAY_INLINE) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				displaytype = rhs.displaytype;
			}
		}
		else {
			displaytype = rhs.displaytype;
		}

	}

	if(rhs.fontsize != FONTSIZE_NORMAL) {
		if(fontsize != FONTSIZE_NORMAL) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				fontsize = rhs.fontsize;
			}
		}
		else {
			fontsize = rhs.fontsize;
		}
	}

	if(rhs.fontweight != FONTWEIGHT_NORMAL) {
		if(fontweight != FONTWEIGHT_NORMAL) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				fontweight = rhs.fontweight;
			}
		}
		else {
			fontweight = rhs.fontweight;
		}
	}

	if(rhs.fontstyle != FONTSTYLE_NORMAL) {
		if(fontstyle != FONTSTYLE_NORMAL) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				fontstyle = rhs.fontstyle;
			}
		}
		else {
			fontstyle = rhs.fontstyle;
		}
	}

	if(rhs.margintop.type != CSS_VALUE_DEFAULT) {
		if(margintop.type != CSS_VALUE_DEFAULT) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				margintop = rhs.margintop;
			}
		}
		else {
			margintop = rhs.margintop;
		}
	}

	if(rhs.marginbottom.type != CSS_VALUE_DEFAULT) {
		if(marginbottom.type != CSS_VALUE_DEFAULT) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				marginbottom = rhs.marginbottom;
			}
		}
		else {
			marginbottom = rhs.marginbottom;
		}
	}

	if(rhs.pagebreakbefore != false) {
		if(pagebreakbefore != false) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				pagebreakbefore = rhs.pagebreakbefore;
			}
		}
		else {
			pagebreakbefore = rhs.pagebreakbefore;
		}
	}

	if(rhs.pagebreakafter != false) {
		if(pagebreakafter != false) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				pagebreakafter = rhs.pagebreakafter;
			}
		}
		else {
			pagebreakafter = rhs.pagebreakafter;
		}
	}

	if(rhs.textalign != TEXTALIGN_LEFT) {
		if(textalign != TEXTALIGN_LEFT) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				textalign = rhs.textalign;
			}
		}
		else {
			textalign = rhs.textalign;
		}
	}

	if(rhs.textindent.type != CSS_VALUE_DEFAULT) {
		if(textindent.type != CSS_VALUE_DEFAULT) {
			//Both are set, check specificity.
			if(rhs.selector.specificity > selector.specificity) {
				textindent = rhs.textindent;
			}
		}
		else {
			textindent = rhs.textindent;
		}
	}

	//Now lets do the map of raw
	//tags

	for(pair<ustring, ustring> p : rhs.raw_pairs) {
		//check if it exists in this object.
		if(raw_pairs.count(p.first) > 0) {
			//It exists, so we update it.
			raw_pairs[p.first] = p.second;
		}
		else {
			//It doesn't exist, insert it.
			raw_pairs.insert(p);
		}
	}
}

CSS::CSS() :
	files(),
	rules()
{
}

CSS::CSS(vector<path> _files) :
	files(_files),
	rules()
{

	//prepare the regular expressions:
	regex regex_selector;
	regex regex_atrule_single;
	regex regex_atrule_multiple;
	regex regex_attr;
	regex regex_percent;
	regex regex_em;
	regex regex_px;
	regex regex_pt;
	regex regex_cm;
	regex regex_comment_single;
	regex regex_comment_single2;
	regex regex_comment_start;
	regex regex_comment_end;

	try {
		regex_selector = regex("^(.+)\\{", regex::optimize);
		regex_atrule_single = regex("^@.*;$", regex::optimize);
		regex_atrule_multiple = regex("^@.*\\{", regex::optimize);
		regex_attr = regex("([A-Za-z0-9-]+)\\s*:{1}\\s*([^;]*)", regex::optimize);
		regex_percent = regex("([\\d]+)%", regex::optimize);
		regex_em = regex("([\\d]+)em", regex::optimize);
		regex_px = regex("([\\d]+)px", regex::optimize);
		regex_pt = regex("([\\d]+)pt", regex::optimize);
		regex_cm = regex("([\\d]+)cm", regex::optimize);
		regex_comment_single = regex(R"(\/\/.*)", regex::optimize);
		regex_comment_single2 = regex(R"(\/\*.*\*\/)", regex::optimize);
		regex_comment_start = regex(R"(\/\/*.*)", regex::optimize);
		regex_comment_end = regex(R"(.*\*\/)", regex::optimize);
	}
	catch (regex_error re) {
		cout << "You dun goofed " << endl;

		print_regex_error_fatal(re);
	}

	for (path file : files) {

		#ifdef DEBUG
		cout << "CSS File is "  << file << endl;
		#endif

		bool class_is_open = false;
		bool is_at_rule = false;

		ifstream cssfile (file.string());

		if(cssfile.is_open()) {

			CSSRule rule;

			string line;

			while(!cssfile.eof()) {

				getline(cssfile, line);

				#ifdef DEBUG
				cout << "Line is "  << line << endl;
				#endif

				//skip blank lines:
				if(line.length() == 0) {
					continue;
				}

				if(line.find("}") != string::npos) {

					#ifdef DEBUG

					if (!is_at_rule) {
						cout << "\tCSS Closing CSS class " << endl;
					}

					#endif

					rules.insert(rule);

					rule = CSSRule();
					class_is_open = false;
					is_at_rule = false;
					continue;

				}

				//skip at rule lines;
				if(is_at_rule) {
					continue;
				}

				if(regex_match(line, regex_comment_single)) {
					//It's a single-line // comment
					continue;
				}

				if(regex_match(line, regex_comment_single2)) {
					//It's a single-line /* something */ comment
					continue;
				}

				if(!class_is_open) {
					//Deal with a new class.

					//First, check if it's an at rule;
					if(regex_match(line, regex_atrule_single)) {
						//It's a single-line @ rule, ignore it.
						continue;
					}
					else if (regex_match(line, regex_atrule_multiple)) {
						is_at_rule = true;
					}
					else {
						class_is_open = true;

						smatch regex_matches;
						regex_search(line, regex_matches, regex_selector);

						if(regex_matches.size() != 0) {
							string selector = regex_matches[1];
							rule.selector = CSSSelector(selector);
						}

					}

				}
				else {

					smatch regex_matches;
					regex_search(line, regex_matches, regex_attr);

					if(regex_matches.size() != 0) {
						//We found an attr.
						string attrname = regex_matches[1];
						string attrvalue = regex_matches[2];
						rule.raw_pairs.insert(pair<ustring, ustring>(attrname, attrvalue));
						#ifdef DEBUG
						cout << "\tCSS Attribute name: "  << attrname << endl;
						cout << "\tCSS Attribute value "  << attrvalue << endl;
						#endif

						if(attrname == "display") {
							if(attrvalue == "block") {
								rule.displaytype = DISPLAY_BLOCK;
							}
						}
						else if (attrname == "font-size") {
							smatch regex_match_size;
							regex_search(attrvalue, regex_match_size, regex_em);

							if(regex_match_size.size() != 0) {
								//it's of the form x.xem
								double size = stod(regex_match_size[1], NULL);

								if(size < 1.0) {
									rule.fontsize = FONTSIZE_SMALLER;
								}
								else if(size == 1.0) {
									rule.fontsize = FONTSIZE_NORMAL;
								}
								else {
									rule.fontsize = FONTSIZE_LARGER;
								}
							}
						}
						else if (attrname == "font-weight") {
							if(attrvalue == "bold") {
								rule.fontweight = FONTWEIGHT_BOLD;
							}
						}
						else if (attrname == "font-style") {
							if(attrvalue == "italic") {
								rule.fontstyle = FONTSTYLE_ITALIC;
							}
						}
						else if (attrname == "margin-top") {
							smatch regex_match_margin;

							if(regex_search(attrvalue, regex_match_margin, regex_percent)) {
								string match = regex_match_margin[1];
								rule.margintop.value = stod(match, NULL);
								rule.margintop.type = CSS_VALUE_PERCENT;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_em)) {
								string match = regex_match_margin[1];
								rule.margintop.value = stod(match, NULL);
								rule.margintop.type = CSS_VALUE_EM;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_px)) {
								string match = regex_match_margin[1];
								rule.margintop.value = stod(match, NULL);
								rule.margintop.type = CSS_VALUE_PX;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_pt)) {
								string match = regex_match_margin[1];
								rule.margintop.value = stod(match, NULL);
								rule.margintop.type = CSS_VALUE_PT;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_cm)) {
								string match = regex_match_margin[1];
								rule.margintop.value = stod(match, NULL);
								rule.margintop.type = CSS_VALUE_CM;
							}
						}
						else if (attrname == "margin-bottom") {
							smatch regex_match_margin;

							if(regex_search(attrvalue, regex_match_margin, regex_percent)) {
								string match = regex_match_margin[1];
								rule.marginbottom.value = stod(match, NULL);
								rule.marginbottom.type = CSS_VALUE_PERCENT;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_em)) {
								string match = regex_match_margin[1];
								rule.marginbottom.value = stod(match, NULL);
								rule.marginbottom.type = CSS_VALUE_EM;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_px)) {
								string match = regex_match_margin[1];
								rule.marginbottom.value = stod(match, NULL);
								rule.marginbottom.type = CSS_VALUE_PX;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_pt)) {
								string match = regex_match_margin[1];
								rule.marginbottom.value = stod(match, NULL);
								rule.marginbottom.type = CSS_VALUE_PT;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_cm)) {
								string match = regex_match_margin[1];
								rule.marginbottom.value = stod(match, NULL);
								rule.marginbottom.type = CSS_VALUE_CM;
							}
						}
						else if (attrname == "margin") {
							//to set them all.
							smatch regex_match_margin;

							if(regex_search(attrvalue, regex_match_margin, regex_percent)) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL);
								rule.marginbottom.value = margin;
								rule.margintop.value = margin;
								rule.marginbottom.type = CSS_VALUE_PERCENT;
								rule.margintop.type = CSS_VALUE_PERCENT;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_em)) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL);
								rule.marginbottom.value = margin;
								rule.margintop.value = margin;
								rule.marginbottom.type = CSS_VALUE_EM;
								rule.margintop.type = CSS_VALUE_EM;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_px)) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL);
								rule.marginbottom.value = margin;
								rule.margintop.value = margin;
								rule.marginbottom.type = CSS_VALUE_PX;
								rule.margintop.type = CSS_VALUE_PX;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_pt)) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL);
								rule.marginbottom.value = margin;
								rule.margintop.value = margin;
								rule.marginbottom.type = CSS_VALUE_PT;
								rule.margintop.type = CSS_VALUE_PT;
							}
							else if(regex_search(attrvalue, regex_match_margin, regex_cm)) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL);
								rule.marginbottom.value = margin;
								rule.margintop.value = margin;
								rule.marginbottom.type = CSS_VALUE_CM;
								rule.margintop.type = CSS_VALUE_CM;
							}
						}
						else if (attrname == "page-break-before") {
							if(attrvalue == "always") {
								rule.pagebreakbefore = true;
							}
						}
						else if (attrname == "page-break-after") {
							if(attrvalue == "always") {
								rule.pagebreakafter = true;
							}
						}
						else if (attrname == "text-align") {
							if(attrvalue == "right") {
								rule.textalign = TEXTALIGN_RIGHT;
							}
							else if(attrvalue == "center") {
								rule.textalign = TEXTALIGN_CENTER;
							}
							else {
								rule.textalign = TEXTALIGN_LEFT;
							}
						}
						else if (attrname == "text-indent") {
							smatch regex_match_margin;

							if(regex_search(attrvalue, regex_match_margin, regex_percent)) {
								string match = regex_match_margin[1];
								rule.textindent.value = stod(match, NULL);
								rule.textindent.type = CSS_VALUE_PERCENT;
							}
						}
					}
				}
			}
		}
		else {
			throw std::runtime_error("CSS File does not exist!");
		}
	}
}

CSS::CSS(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index)  :
	files(),
	rules()
{

	int rc;

	const string css_select_sql = "SELECT * FROM css WHERE epub_file_id=? AND opf_id=?;";
	const string css_tags_select_sql = "SELECT tagname, tagvalue FROM css_tags WHERE css_id=?;";

	sqlite3_stmt * css_select;
	sqlite3_stmt * css_tags_select;

	rc = sqlite3_prepare_v2(db, css_select_sql.c_str(), -1, &css_select, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, css_tags_select_sql.c_str(), -1, &css_tags_select, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	sqlite3_bind_int(css_select, 1, epub_file_id);
	sqlite3_bind_int(css_select, 2, opf_index);

	rc = sqlite3_step(css_select);

	while ( rc == SQLITE_ROW ) {

		//Default-construct the object:
		CSSRule rule;

		//Get the basic data
		unsigned int css_id = sqlite3_column_int(css_select, 0);

		string selector = sqlite3_column_string(css_select, 3);
		string collation_key = sqlite3_column_string(css_select, 4);
		DisplayType displaytype = (DisplayType) sqlite3_column_int(css_select, 5);
		FontSize fontsize = (FontSize) sqlite3_column_int(css_select, 6);
		FontWeight fontweight = (FontWeight) sqlite3_column_int(css_select, 7);
		FontStyle fontstyle = (FontStyle) sqlite3_column_int(css_select, 8);
		double margintop = sqlite3_column_double(css_select, 9);
		CSSValueType margintop_type = (CSSValueType) sqlite3_column_int(css_select, 10);
		double marginbottom = sqlite3_column_double(css_select, 11);
		CSSValueType marginbottom_type = (CSSValueType) sqlite3_column_int(css_select, 12);
		bool pagebreakbefore = (bool) sqlite3_column_int(css_select, 13);
		bool pagebreakafter = (bool) sqlite3_column_int(css_select, 14);
		TextAlign textalign = (TextAlign) sqlite3_column_int(css_select, 15);
		double textindent = sqlite3_column_double(css_select, 16);
		CSSValueType textindent_type = (CSSValueType) sqlite3_column_int(css_select, 17);

		rule.selector = CSSSelector(selector);
		rule.collation_key = collation_key;
		rule.displaytype = displaytype;
		rule.fontsize = fontsize;
		rule.fontweight = fontweight;
		rule.fontstyle = fontstyle;
		rule.margintop.value = margintop;
		rule.margintop.type = margintop_type;
		rule.marginbottom.value = marginbottom;
		rule.marginbottom.type = marginbottom_type;
		rule.pagebreakbefore = pagebreakbefore;
		rule.pagebreakafter = pagebreakafter;
		rule.textalign = textalign;
		rule.textindent.value = textindent;
		rule.textindent.type = textindent_type;

		int rc2;

		sqlite3_bind_int(css_tags_select, 1, css_id);

		rc2 = sqlite3_step(css_tags_select);

		while(rc2 == SQLITE_ROW) {

			ustring tagname = sqlite3_column_ustring(css_tags_select, 0);
			ustring tagvalue = sqlite3_column_ustring(css_tags_select, 1);

			rule.raw_pairs.insert(pair<ustring, ustring>(tagname, tagvalue));

			rc2 = sqlite3_step(css_tags_select);

		}

		rules.insert(rule);

		sqlite3_reset(css_tags_select);

		rc = sqlite3_step(css_select);

	}

	sqlite3_finalize(css_select);
	sqlite3_finalize(css_tags_select);

}

CSS::CSS(CSS const & cpy) :
	files(cpy.files),
	rules(cpy.rules)
{
}

CSS::CSS(CSS && mv) :
	files(move(mv.files)),
	rules(move(mv.rules))
{
}

CSS & CSS::operator =(const CSS & cpy)
{
	files = cpy.files;
	rules = cpy.rules;
	return *this;
}

CSS & CSS::operator =(CSS && mv)
{
	files = move(mv.files);
	rules = move(mv.rules);
	return *this;
}

CSS::~CSS() { }

CSSRule CSS::get_rule(const ustring & _selector) const
{

	for(auto & rule : rules) {
		if(rule.selector.matches(_selector)) {
			return rule;
		}
	}

	//It doesn't exist in the database. Return a CSSRule with all defaults.
	return CSSRule();

}

bool CSS::contains_rule(const ustring & _selector) const
{

	if(rules.size() == 0) {
		return false;
	}

	for(auto & rule : rules) {
		if(rule.selector.matches(_selector)) {
			return true;
		}
	}

	return false;

}

void CSS::save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index)
{
	int rc;
	char * errmsg;

	const string css_table_sql = "CREATE TABLE IF NOT EXISTS css("  \
	                             "css_id 				INTEGER PRIMARY KEY," \
	                             "epub_file_id			INTEGER NOT NULL," \
	                             "opf_id 				INTEGER NOT NULL," \
	                             "selector			 	TEX NOT NULL," \
	                             "collation_key		 	TEX NOT NULL," \
	                             "display_type	 		INTEGER NOT NULL," \
	                             "font_size		 		INTEGER NOT NULL," \
	                             "font_weight	 		INTEGER NOT NULL," \
	                             "font_style	 		INTEGER NOT NULL," \
	                             "margin_top	 		REAL NOT NULL," \
	                             "margin_top_type		INTEGER NOT NULL," \
	                             "margin_bottom		REAL NOT NULL," \
	                             "margin_bottom_type	INTEGER NOT NULL," \
	                             "pagebreakbefore		INTEGER NOT NULL," \
	                             "pagebreakafter		INTEGER NOT NULL," \
	                             "text_align			INTEGER NOT NULL," \
	                             "text_indent			REAL NOT NULL," \
	                             "text_indent_type 		INTEGER NOT NULL) ;";
	sqlite3_exec(db, css_table_sql.c_str(), NULL, NULL, &errmsg);

	const string css_tags_table_sql = "CREATE TABLE IF NOT EXISTS css_tags("  \
	                                  "css_id INTEGER NOT NULL," \
	                                  "tagname TEXT NOT NULL," \
	                                  "tagvalue TEXT NOT NULL) ;";
	sqlite3_exec(db, css_tags_table_sql.c_str(), NULL, NULL, &errmsg);

	//Tables created.
	sqlite3_stmt * css_insert;
	sqlite3_stmt * css_tags_insert;

	const string css_insert_sql = "INSERT INTO css (epub_file_id, opf_id, selector, collation_key, display_type, font_size, font_weight, font_style, margin_top, margin_top_type, margin_bottom, margin_bottom_type, pagebreakbefore, pagebreakafter, text_align, text_indent, text_indent_type) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
	const string css_tags_insert_sql = "INSERT INTO css_tags (css_id, tagname, tagvalue) VALUES (?, ?, ?);";

	rc = sqlite3_prepare_v2(db, css_insert_sql.c_str(), -1, &css_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, css_tags_insert_sql.c_str(), -1, &css_tags_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	for(auto & rule : rules) {

		sqlite3_bind_int(css_insert, 1, epub_file_id);
		sqlite3_bind_int(css_insert, 2, opf_index);
		sqlite3_bind_text(css_insert, 3, rule.selector.raw_text.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(css_insert, 4, rule.collation_key.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(css_insert, 5, (int) rule.displaytype);
		sqlite3_bind_int(css_insert, 6, (int) rule.fontsize);
		sqlite3_bind_int(css_insert, 7, (int) rule.fontweight);
		sqlite3_bind_int(css_insert, 8, (int) rule.fontstyle);
		sqlite3_bind_double(css_insert, 9, rule.margintop.value);
		sqlite3_bind_int(css_insert, 10, (int) rule.margintop.type);
		sqlite3_bind_double(css_insert, 11, rule.marginbottom.value);
		sqlite3_bind_int(css_insert, 12, (int) rule.marginbottom.type);
		sqlite3_bind_int(css_insert, 13, (int) rule.pagebreakbefore);
		sqlite3_bind_int(css_insert, 14, (int) rule.pagebreakafter);
		sqlite3_bind_int(css_insert, 15, (int) rule.textalign);
		sqlite3_bind_double(css_insert, 16, rule.textindent.value);
		sqlite3_bind_int(css_insert, 17, (int) rule.textindent.type);

		int result = sqlite3_step(css_insert);

		if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
			throw - 1;
		}

		sqlite3_reset(css_insert);

		//get the new id:
		const auto key = sqlite3_last_insert_rowid(db);

		for(auto & pair : rule.raw_pairs) {

			sqlite3_bind_int(css_tags_insert, 1, key);
			sqlite3_bind_text(css_tags_insert, 2, pair.first.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(css_tags_insert, 3, pair.second.c_str(), -1, SQLITE_STATIC);

			int result = sqlite3_step(css_tags_insert);

			if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
				throw - 1;
			}

			sqlite3_reset(css_tags_insert);

		}
	}

	//Create an index for the css tags
	const string css_tags_index_sql = "CREATE INDEX index_css_tags ON css_tags(css_id);";
	sqlite3_exec(db, css_tags_index_sql.c_str(), NULL, NULL, &errmsg);

	//Create an index for the css itself
	const string css_index_sql = "CREATE INDEX index_css ON css(epub_file_id, opf_id);";
	sqlite3_exec(db, css_index_sql.c_str(), NULL, NULL, &errmsg);

	sqlite3_finalize(css_insert);
	sqlite3_finalize(css_tags_insert);

}
