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

#include "Content.hpp"

#include <utility>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>
#include <exception>

using std::move; 
using std::pair;

#ifdef DEBUG
#include <iostream>
using std::cout; 
using std::endl; 
#endif

using namespace boost::filesystem;
using namespace xmlpp;

ContentItem::ContentItem(ContentType _type, path _file, ustring _id, ustring _content, ustring _stripped_content) :
	type(_type), 
	file(_file),
	id(_id),
	content(_content), 
	stripped_content(_stripped_content)
{
	
}
		
ContentItem::ContentItem(ContentItem const & cpy)  :
	type(cpy.type),
	file(cpy.file),
	id(cpy.id),
	content(cpy.content), 
	stripped_content(cpy.stripped_content)
{
	
}

ContentItem::ContentItem(ContentItem && mv)  :
	type(move(mv.type)),
	file(move(mv.file)),
	id(move(mv.id)),
	content(move(mv.content)),
	stripped_content(move(mv.stripped_content))
{
	
}

ContentItem& ContentItem::operator =(const ContentItem& cpy) {
	type = cpy.type;
	file = cpy.file; 
	id = cpy.id; 
	content = cpy.content;
	stripped_content = cpy.stripped_content;
	return *this;
}

ContentItem& ContentItem::operator =(ContentItem && mv)  {
	type = move(mv.type);
	file = move(mv.file);
	id = move(mv.id);
	content = move(mv.content);
	stripped_content = move(mv.stripped_content);
	return *this;
}
	
ContentItem::~ContentItem(){
	
}

///////////////////////
namespace 
{
	inline ustring __create_text(ustring nodename, ustring nodecontents) {
		ustring tmp; 
		tmp += "<";
		tmp += nodename; 
		tmp += ">";
		tmp += nodecontents; 
		tmp += "</";
		tmp += nodename;
		tmp += ">";
		return tmp;
	}

	ustring __id = ""; 

	//This whole method is fairly awful. 
	pair<ustring, ustring> __recursive_strip(vector<ContentItem> & items, const path file, const Node * const node) {
		
		const TextNode * nodeText = dynamic_cast<const TextNode*>(node);
		
		if(nodeText) {
			
			if(nodeText->is_white_space()) {
				return pair<ustring, ustring>("","");
			}
			
			ustring content = nodeText->get_content();
			
			return pair<ustring, ustring>(content, content);
		}
		
		const Element * nodeElement = dynamic_cast<const Element*>(node);
		
		const auto nodelist = nodeElement->get_children();
		
		ustring value = "";
		ustring value_stripped = ""; 
		
		for(auto niter = nodelist.begin(); niter != nodelist.end(); ++niter)
		{
			const Node * childNode = *niter; 
			
			//Still still Genuinely horrible. 
			const Element* childElement = dynamic_cast<const Element*>(childNode);
			const TextNode * childText = dynamic_cast<const TextNode*>(childNode);
			
			if(!childElement && !childText) continue;
			
			if(childText) {
				
				pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
				value += res.first; 
				value_stripped += res.second; 
				
			}
			
			if(childElement) {
				
				const ustring childname = childElement->get_name();
				
				if(childname.compare("i") == 0) {
					//italic. 
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("i", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("b") == 0) {
					//bold;
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("b", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("big") == 0) {
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("big", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("s") == 0) {
					//strikethrough
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("s", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("sub") == 0) {
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("sub", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("sup") == 0) {
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("sup", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("small") == 0) {
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("i", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("tt") == 0) {
					//monospace
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("tt", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("u") == 0) {
					//underline
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += __create_text("u", res.first); 
					value_stripped += res.second; 
				}
				else if(childname.compare("a") == 0) {
					//specific bheaviour for stripping hyperlinks within the text. 
					//I suspect that I'll have to come back to this, but at the moment I'm 
					//not completely sure how to handle it. 
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += res.first; 
					value_stripped += res.second; 
				}
				else if(childname.compare("span") == 0) {
					//specific bheaviour for stripping span tags
					pair<ustring, ustring> res = __recursive_strip(items, file, childNode);
					value += res.first; 
					value_stripped += res.second; 
				}
				else if(childname.compare("hr") == 0)  {
					//What to do if this is a nested <hr> tag within (frequently)
					//a <p> tag. 
					
					ContentType ct = HR;
					ContentItem ci(ct, file, __id, "", "");
					items.push_back(ci);
					
					value = "";
					value_stripped = "";
					
				}
				
			}
			
		}
		
		return pair<ustring, ustring>(value, value_stripped); 
		
	}

	void __recursive_find(vector<ContentItem> & items, const path file, const Node * const node) {
		
		const auto nlist = node->get_children();
		
		for(auto niter = nlist.begin(); niter != nlist.end(); ++niter)
		{
			const Node * ntmp = *niter; 
			
			//Still still Genuinely horrible. 
			const Element* tmpnode = dynamic_cast<const Element*>(ntmp);
			
			if(!tmpnode) continue;
				
			const ustring tmpnodename = tmpnode->get_name();
			
			if(tmpnodename.compare("p") != 0 &&
				tmpnodename.compare("h1") != 0 &&
				tmpnodename.compare("h2") != 0 &&
				tmpnodename.compare("hr") != 0) {
					__recursive_find(items, file, ntmp);
			}		
			else {
				
				ContentType ct = P;
				
				if(tmpnode->get_name().compare("p") == 0) {
					ct = P;
				}
				else if (tmpnode->get_name().compare("h1") == 0) {
					ct = H1;
				}
				else if (tmpnode->get_name().compare("h2") == 0) {
					ct = H2;
				}
				else if (tmpnode->get_name().compare("hr") == 0) {
					ct = HR;
				}
				
				const auto attributes = tmpnode->get_attributes();
				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
				{
					const Attribute* attribute = *iter;
					
					if(attribute->get_name().compare("id") == 0) __id = attribute->get_value();
				}
				
				pair<ustring, ustring> content = __recursive_strip(items, file, ntmp);
						
				if(content.first.compare("") == 0 && ct != HR) continue; 
						
				#ifdef DEBUG
				cout << tmpnode->get_name()  << " " << __id <<endl; 
				cout << " \t " << content.first << endl; 
				cout << " \t " << content.second << endl; 
				#endif			
				
				ContentItem ci(ct, file, __id, content.first, content.second);
				
				items.push_back(ci);
			
			}
			
		}
		
	}
} // end anonymous namespace

Content::Content(CSS& _classes, vector<path> _files) :
	classes(_classes),
	files(_files)
{
	
	for(const auto file : files) {
		
		__id = "";
		
		if(!exists(file)) {
			throw std::runtime_error("Content file specified in OPF file does not exist!");
		}
		
		#ifdef DEBUG
		cout << "Loading content file " << file << endl; 
		#endif
		
		DomParser parser; 
		parser.parse_file(file.string());
		
		const Node* root = parser.get_document()->get_root_node();
		
		const ustring rootname = root->get_name();
		
		if(rootname.compare("html") != 0) {
			throw std::runtime_error("Linked content file isn't HTML. So we can't read it. Mostly through laziness.");
		}
	
		const auto nlist = root->get_children();
	
		for(auto niter = nlist.begin(); niter != nlist.end(); ++niter)
		{
			const Node * ntmp = *niter; 
			
			//Still still Genuinely horrible. 
			const Element* tmpnode = dynamic_cast<const Element*>(ntmp);
			
			if(!tmpnode) continue;
		
			if(tmpnode->get_name().compare("body") == 0)  {
				
				__recursive_find(items, file, ntmp);
				
			}
			
		}
	}
	
}

Content::Content(Content const & cpy) :
	classes(cpy.classes),
	files(cpy.files), 
	items(cpy.items)
{
	
}

Content::Content(Content && mv) :
	classes(mv.classes),
	files(move(mv.files)),
	items(move(mv.items))
{
	
}

Content& Content::operator =(const Content& cpy) { 
	classes = cpy.classes;
	files = cpy.files;
	items = cpy.items;
	return *this; 
	
}

Content& Content::operator =(Content && mv) {
	classes = mv.classes;
	files = move(mv.files); 
	items = move(mv.items);
	return *this;
}
	
Content::~Content() {
	
}