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

using std::string; 
using std::move;
using std::ifstream;
using std::pair; 
using std::regex; 
using std::regex_search; 
using std::smatch; 

#ifdef DEBUG
#include <iostream>
using std::cout; 
using std::endl;
#endif

CSSClass::CSSClass(ustring _name) :
	name(_name), 
	displaytype(DISPLAYTYPE_INLINE), 
	fontsize(FONTSIZE_NORMAL),
	fontweight(FONTWEIGHT_NORMAL),
	fontstyle(FONTSTYLE_NORMAL), 
	margintop(0.0),
	marginbottom(0.0)
{
	
}

CSSClass::CSSClass(CSSClass const & cpy) :
	name(cpy.name), 
	displaytype(cpy.displaytype), 
	fontsize(cpy.fontsize), 
	fontweight(cpy.fontweight),
	fontstyle(cpy.fontstyle), 
	margintop(cpy.margintop),
	marginbottom(cpy.marginbottom)
{
	
}

CSSClass::CSSClass(CSSClass && mv) :
	name(move(mv.name)), 
	displaytype(move(mv.displaytype)),
	fontsize(move(mv.fontsize)),
	fontweight(move(mv.fontweight)),
	fontstyle(move(mv.fontstyle)),
	margintop(move(mv.margintop)),
	marginbottom(move(mv.marginbottom))
{
	
}

CSSClass& CSSClass::operator =(const CSSClass& cpy) {
	name = cpy.name;
	displaytype = cpy.displaytype; 
	fontsize = cpy.fontsize; 
	fontweight = cpy.fontweight; 
	fontstyle = cpy.fontstyle;
	margintop = cpy.margintop;
	marginbottom = cpy.marginbottom;
	return *this;
}

CSSClass& CSSClass::operator =(CSSClass && mv)  {
	name = move(mv.name); 
	displaytype = move(mv.displaytype);
	fontsize = move(mv.fontsize); 
	fontweight = move(mv.fontweight); 
	fontstyle = move(mv.fontstyle);
	margintop = move(margintop);
	marginbottom = move(marginbottom);
	return *this;
}

CSSClass::~CSSClass() { }

CSS::CSS(vector<path> _files) : 
	files(_files), 
	classes()	
{
	
	for (path file : files) { 
		
		bool class_is_open = false; 
		CSSClass cssclass("");
		
		ifstream cssfile (file.string());
		if(cssfile.is_open()) {
			
			string line; 
			
			//prepare the regular expressions:
			regex regex_classname ("([A-Za-z0-9]+)"); 
			regex regex_attr("([A-Za-z0-9-]+)\\s*:{1}\\s*(.*);");
			
			while(!cssfile.eof()) {	

				getline(cssfile, line);			
				
				#ifdef DEBUG
				cout << "Line is "  << line << endl; 
				#endif
				
				//skip blank lines:
				if(line.length() == 0) continue; 
				
				if(line.find("}") != string::npos) {
					
					#ifdef DEBUG
					cout << "\tCSS Closing CSS class named: "  << cssclass.name << endl; 
					#endif
					
					class_is_open = false;
					classes.insert(pair<ustring, CSSClass>(cssclass.name, cssclass));
					cssclass = CSSClass("");
					
					continue; 
				}
				
				if(!class_is_open) {
					//Deal with a new class. 
					
					class_is_open = true; 
					
					smatch regex_matches; 
					regex_search(line, regex_matches, regex_classname); 
					
					if(regex_matches.size() != 0) {
						//We found the classname. 
						cssclass.name = regex_matches[0];
						
						#ifdef DEBUG
						cout << "\tCSS Class name: "  << cssclass.name << endl; 
						#endif
						
					}
				}
				
				else {
					
					smatch regex_matches; 
					regex_search(line, regex_matches, regex_attr); 
					
					if(regex_matches.size() != 0) {
						//We found an attr. 
						
						#ifdef DEBUG
						cout << "\tCSS Attribute name: "  << regex_matches[1] << endl; 
						cout << "\tCSS Attribute value "  << regex_matches[2] << endl; 
						#endif
						
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