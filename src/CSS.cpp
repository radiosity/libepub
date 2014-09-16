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

using std::string; 
using std::move;
using std::ifstream;
using std::pair; 
using std::regex; 
using std::regex_search;
using std::regex_match;
using std::sregex_iterator; 
using std::smatch; 
using std::numeric_limits; 
using std::stod; 

#ifdef DEBUG
#include <iostream>
using std::cout; 
using std::endl;
#endif

CSSClass::CSSClass() :
	CSSClass("")
{
	
}

CSSClass::CSSClass(ustring _name) : 
	name(_name), 
	raw_pairs(),
	displaytype(DISPLAYTYPE_INLINE), 
	fontsize(FONTSIZE_NORMAL),
	fontweight(FONTWEIGHT_NORMAL),
	fontstyle(FONTSTYLE_NORMAL), 
	margintop(numeric_limits<double>::min()),
	marginbottom(numeric_limits<double>::min()), 
	pagebreakbefore(false),
	pagebreakafter(false), 
	textalign(TEXTALIGN_LEFT),
	textindent(numeric_limits<double>::min())
{
	
}

CSSClass::CSSClass(CSSClass const & cpy) :
	name(cpy.name), 
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

CSSClass::CSSClass(CSSClass && mv) :
	name(move(mv.name)), 
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

CSSClass& CSSClass::operator =(const CSSClass& cpy) {
	name = cpy.name;
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

CSSClass& CSSClass::operator =(CSSClass && mv)  {
	name = move(mv.name); 
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

CSSClass::~CSSClass() { }

void CSSClass::add ( const CSSClass& rhs ) {
	
	//Do the basics:
	if(rhs.displaytype != DISPLAYTYPE_INLINE) displaytype = rhs.displaytype; 
	if(rhs.fontsize != FONTSIZE_NORMAL) fontsize = rhs.fontsize; 
	if(rhs.fontweight != FONTWEIGHT_NORMAL) fontweight = rhs.fontweight; 
	if(rhs.fontstyle != FONTSTYLE_NORMAL) fontstyle = rhs.fontstyle; 
	if(rhs.margintop != numeric_limits<double>::min()) margintop = rhs.margintop; 
	if(rhs.marginbottom != numeric_limits<double>::min()) marginbottom = rhs.marginbottom; 
	
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
	classes()
{
	
}

CSS::CSS(vector<path> _files) : 
	files(_files), 
	classes()	
{
	
	//prepare the regular expressions:
	regex regex_classname ("([A-Za-z0-9\\.-]+)", regex::optimize); 
	regex regex_atrule_single("^@.*;$", regex::optimize);
	regex regex_atrule_multiple("^@.*\\{", regex::optimize);
	regex regex_attr("([A-Za-z0-9-]+)\\s*:{1}\\s*(.*)", regex::optimize);
	regex regex_percent("([\\d\\.]+)%", regex::optimize);
	regex regex_em("([\\d\\.]+)em", regex::optimize);
	
	for (path file : files) { 
		
		#ifdef DEBUG
		cout << "CSS File is "  << file << endl; 
		#endif
		
		bool class_is_open = false; 
		bool is_at_rule = false; 
		
		ifstream cssfile (file.string());
		if(cssfile.is_open()) {
			
			CSSClass cssclass;
			
			string line; 
			
			//You can declare that multiple things are affected by one declaration. 
			//For example: 
			//div, p, pre, h1, h2, h3, h4, h5, h6 {
			//	margin-left: 0;	
			//}
			//This means we need to keep a list of names for the style 
			
			vector<ustring> classnames; 
			
			while(!cssfile.eof()) {	

				getline(cssfile, line);			
				
				#ifdef DEBUG
				cout << "Line is "  << line << endl; 
				#endif
				
				//skip blank lines:
				if(line.length() == 0) continue; 
				
				if(line.find("}") != string::npos) {
					
					#ifdef DEBUG
					if (!is_at_rule) cout << "\tCSS Closing CSS class "  << endl; 
					#endif
					
					for (auto classname : classnames) {
						
						//Check if the classname exists - if it does we need to update it. 
						
						if(classes.count(classname) != 0) {
							//It exists! update it. 
							
							classes[classname].add(cssclass);
						}
						else {
							cssclass.name = classname; 
						
							classes.insert(pair<ustring, CSSClass>(classname, cssclass)); 
						}
					}
					
					cssclass = CSSClass();
					
					classnames.clear();
					
					class_is_open = false;
					is_at_rule = false; 
					
					continue; 
					
				}
				
				//skip at rule lines;
				if(is_at_rule) continue; 
				
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
						
						auto line_begin = sregex_iterator(line.begin(), line.end(), regex_classname);
						auto line_end = sregex_iterator();
	 
						classnames.reserve(distance(line_begin, line_end));
	 
						for (sregex_iterator i = line_begin; i != line_end; ++i) {
							smatch match = *i;                                                 
							string classname = match.str(); 
							
							classnames.push_back(ustring(classname));
							
							#ifdef DEBUG
							cout << "\tCSS Class name: "  << classname << endl; 
							#endif
							
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
						
						cssclass.raw_pairs.insert(pair<ustring, ustring>(attrname, attrvalue));
						
						#ifdef DEBUG
						cout << "\tCSS Attribute name: "  << attrname << endl; 
						cout << "\tCSS Attribute value "  << attrvalue << endl; 
						#endif
						
						if(attrname == "display") {
							if(attrvalue == "block") cssclass.displaytype = DISPLAYTYPE_BLOCK; 
						}
						else if (attrname == "font-size") {
							smatch regex_match_size; 
							regex_search(attrvalue, regex_match_size, regex_em); 
							if(regex_match_size.size() != 0) {
								//it's of the form x.xem
								double size = stod(regex_match_size[1], NULL); 
								if(size < 1.0) cssclass.fontsize = FONTSIZE_SMALLER; 
								else if(size == 1.0) cssclass.fontsize = FONTSIZE_NORMAL; 
								else cssclass.fontsize = FONTSIZE_LARGER; 
							}
						}
						else if (attrname == "font-weight") {
							if(attrvalue == "bold") cssclass.fontweight = FONTWEIGHT_BOLD; 
						}
						else if (attrname == "font-style") {
							if(attrvalue == "italic") cssclass.fontstyle = FONTSTYLE_ITALIC; 
						
						}
						else if (attrname == "margin-top") {
							smatch regex_match_margin; 
							regex_search(attrvalue, regex_match_margin, regex_percent); 
							if(regex_match_margin.size() != 0) {
								string match = regex_match_margin[1];
								cssclass.margintop = stod(match, NULL); 
							}
						}
						else if (attrname == "margin-bottom") {
							smatch regex_match_margin; 
							regex_search(attrvalue, regex_match_margin, regex_percent); 
							if(regex_match_margin.size() != 0) {
								string match = regex_match_margin[1];
								cssclass.marginbottom = stod(match, NULL); 
							}
						}
						else if (attrname == "margin") {
							//to set them all. 
							smatch regex_match_margin; 
							regex_search(attrvalue, regex_match_margin, regex_percent); 
							if(regex_match_margin.size() != 0) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL); 
								cssclass.marginbottom = margin; 
								cssclass.margintop = margin; 
							}
						}
						else if (attrname == "page-break-before") {
							if(attrvalue == "always") cssclass.pagebreakbefore = true; 
						
						}
						else if (attrname == "page-break-after") {
							if(attrvalue == "always") cssclass.pagebreakafter = true; 
						
						}
						else if (attrname == "text-align") {
							if(attrvalue == "right") cssclass.textalign = TEXTALIGN_RIGHT; 
							else if(attrvalue == "center") cssclass.textalign = TEXTALIGN_CENTER; 
							else cssclass.textalign = TEXTALIGN_LEFT; 
						}
						else if (attrname == "text-indent") {
							smatch regex_match_margin; 
							regex_search(attrvalue, regex_match_margin, regex_percent); 
							if(regex_match_margin.size() != 0) {
								string match = regex_match_margin[1];
								cssclass.textindent = stod(match, NULL); 
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

CSS::CSS(CSS const & cpy) :
	files(cpy.files),
	classes(cpy.classes) 
{
	
}

CSS::CSS(CSS && mv) :
	files(move(mv.files)), 
	classes(move(mv.classes)) 
{
	
}

CSS& CSS::operator =(const CSS& cpy) {
	files = cpy.files; 
	classes = cpy.classes; 
	return *this; 
}

CSS& CSS::operator =(CSS && mv)  {
	files = move(mv.files); 
	classes = move(mv.classes); 
	return *this; 
}

CSS::~CSS() { }

CSSClass CSS::get_class(ustring name) const {
	
	if(classes.count(name) == 0) {
		//It doesn't exist in the database. Return a CSSClass with all defaults. 
		return CSSClass(); 
	}
	else {
		return classes.at(name);
	}
	
}
