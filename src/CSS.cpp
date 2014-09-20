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

CSSValue& CSSValue::operator =(const CSSValue& cpy) {
	value = cpy.value; 
	type = cpy.type;
	return *this; 
}

CSSValue& CSSValue::operator =(CSSValue && mv) {
	value = move(mv.value);
	type = move(mv.type);
	return *this; 
}

CSSValue::~CSSValue() {
}


CSSClass::CSSClass() :
	CSSClass("")
{
	
}

CSSClass::CSSClass(ustring _name) : 
	name(_name), 
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
	if(rhs.displaytype != DISPLAY_INLINE) displaytype = rhs.displaytype; 
	if(rhs.fontsize != FONTSIZE_NORMAL) fontsize = rhs.fontsize; 
	if(rhs.fontweight != FONTWEIGHT_NORMAL) fontweight = rhs.fontweight; 
	if(rhs.fontstyle != FONTSTYLE_NORMAL) fontstyle = rhs.fontstyle; 
	if(rhs.margintop.type != CSS_VALUE_DEFAULT) margintop = rhs.margintop; 
	if(rhs.marginbottom.type != CSS_VALUE_DEFAULT) marginbottom = rhs.marginbottom; 
	if(rhs.pagebreakbefore != false) pagebreakbefore = true; 
	if(rhs.pagebreakafter != false) pagebreakafter = true; 
	if(rhs.textalign != TEXTALIGN_LEFT) textalign = rhs.textalign; 
	if(rhs.textindent.type != CSS_VALUE_DEFAULT) textindent = rhs.textindent; 
	
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
	regex regex_attr("([A-Za-z0-9-]+)\\s*:{1}\\s*([^;]*)", regex::optimize);
	regex regex_percent("([\\d]+)%", regex::optimize);
	regex regex_em("([\\d]+)em", regex::optimize);
	
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
							if(attrvalue == "block") cssclass.displaytype = DISPLAY_BLOCK; 
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
								cssclass.margintop.value = stod(match, NULL); 
								cssclass.margintop.type = CSS_VALUE_PERCENT;
							}
						}
						else if (attrname == "margin-bottom") {
							smatch regex_match_margin; 
							regex_search(attrvalue, regex_match_margin, regex_percent); 
							if(regex_match_margin.size() != 0) {
								string match = regex_match_margin[1];
								cssclass.marginbottom.value = stod(match, NULL); 
								cssclass.marginbottom.type = CSS_VALUE_PERCENT;
							}
						}
						else if (attrname == "margin") {
							//to set them all. 
							smatch regex_match_margin; 
							regex_search(attrvalue, regex_match_margin, regex_percent); 
							if(regex_match_margin.size() != 0) {
								string match = regex_match_margin[1];
								double margin = stod(match, NULL); 
								cssclass.marginbottom.value = margin; 
								cssclass.margintop.value = margin; 
								cssclass.marginbottom.type = CSS_VALUE_PERCENT;
								cssclass.margintop.type = CSS_VALUE_PERCENT;
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
								cssclass.textindent.value = stod(match, NULL); 
								cssclass.textindent.type = CSS_VALUE_PERCENT;
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

void CSS::save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index) {
	
	int rc; 
	char* errmsg;
	
	const string css_table_sql = "CREATE TABLE IF NOT EXISTS css("  \
						"css_id 				INTEGER PRIMARY KEY," \
						"epub_file_id			INTEGER NOT NULL," \
						"opf_id 				INTEGER NOT NULL," \
						"name			 	TEX NOT NULL," \
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
	
	/*
		ustring name; 
		map<ustring, ustring> raw_pairs;
		DisplayType displaytype; 
		FontSize fontsize; 
		FontWeight fontweight; 
		FontStyle fontstyle; 
		double margintop; 
		double marginbottom; 
		bool pagebreakbefore; 
		bool pagebreakafter; 
		TextAlign textalign; 
		double textindent;
	*/
	
	const string css_insert_sql = "INSERT INTO css (epub_file_id, opf_id, name, display_type, font_size, font_weight, font_style, margin_top, margin_top_type, margin_bottom, margin_bottom_type, pagebreakbefore, pagebreakafter, text_align, text_indent, text_indent_type) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
	const string css_tags_insert_sql = "INSERT INTO css_tags (css_id, tagname, tagvalue) VALUES (?, ?, ?);";
	
	rc = sqlite3_prepare_v2(db, css_insert_sql.c_str(), -1, &css_insert, 0);
	if(rc != SQLITE_OK && rc != SQLITE_DONE) throw -1;
	rc = sqlite3_prepare_v2(db, css_tags_insert_sql.c_str(), -1, &css_tags_insert, 0);
	if(rc != SQLITE_OK && rc != SQLITE_DONE) throw -1;
	
	for(auto & csspair : classes) {
		
		CSSClass cssclass = csspair.second;
		
		sqlite3_bind_int(css_insert, 1, epub_file_id);
		sqlite3_bind_int(css_insert, 2, opf_index);
		sqlite3_bind_text(css_insert, 3, cssclass.name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(css_insert, 4, (int) cssclass.displaytype);
		sqlite3_bind_int(css_insert, 5, (int) cssclass.fontsize);
		sqlite3_bind_int(css_insert, 6, (int) cssclass.fontweight);
		sqlite3_bind_int(css_insert, 7, (int) cssclass.fontstyle);
		sqlite3_bind_double(css_insert, 8, cssclass.margintop.value);
		sqlite3_bind_int(css_insert, 9, (int) cssclass.margintop.type);
		sqlite3_bind_double(css_insert, 10, cssclass.marginbottom.value);
		sqlite3_bind_int(css_insert, 11, (int) cssclass.marginbottom.type);
		sqlite3_bind_int(css_insert, 12, (int) cssclass.pagebreakbefore);
		sqlite3_bind_int(css_insert, 13, (int) cssclass.pagebreakafter);
		sqlite3_bind_int(css_insert, 14, (int) cssclass.textalign);
		sqlite3_bind_double(css_insert, 15, cssclass.textindent.value);
		sqlite3_bind_int(css_insert, 16, (int) cssclass.textindent.type);
		
		int result = sqlite3_step(css_insert);
		if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) throw -1;
		
		sqlite3_reset(css_insert);
		
		//get the new id:
		const auto key = sqlite3_last_insert_rowid(db);
		
		for(auto & pair : cssclass.raw_pairs) {
			
			sqlite3_bind_int(css_tags_insert, 1, key);
			sqlite3_bind_text(css_tags_insert, 2, pair.first.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(css_tags_insert, 3, pair.second.c_str(), -1, SQLITE_STATIC);
			
			int result = sqlite3_step(css_tags_insert);
			if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) throw -1;
		
			sqlite3_reset(css_tags_insert);	
			
		}
		
	}
	
	//Create an index for the css tags
	const string css_index_sql = "CREATE INDEX index_css_tags ON css_tags(css_id);";
	sqlite3_exec(db, css_index_sql.c_str(), NULL, NULL, &errmsg);
	
	sqlite3_finalize(css_insert); 
	sqlite3_finalize(css_tags_insert); 
	
}
