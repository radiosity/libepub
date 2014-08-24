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

ContentItem::ContentItem(ContentType _type, path _file, ustring _id, ustring _content) :
	type(_type), 
	file(_file),
	id(_id),
	content(_content)
{
	
}
		
ContentItem::ContentItem(ContentItem const & cpy)  :
	type(cpy.type),
	file(cpy.file),
	id(cpy.id),
	content(cpy.content)
{
	
}

ContentItem::ContentItem(ContentItem && mv)  :
	type(move(mv.type)),
	file(move(mv.file)),
	id(move(mv.id)),
	content(move(mv.content))
{
	
}

ContentItem& ContentItem::operator =(const ContentItem& cpy) {
	type = cpy.type;
	file = cpy.file; 
	id = cpy.id; 
	content = cpy.content;
	return *this;
}

ContentItem& ContentItem::operator =(ContentItem && mv)  {
	type = move(mv.type);
	file = move(mv.file);
	id = move(mv.id);
	content = move(mv.content);
	return *this;
}
	
ContentItem::~ContentItem(){
	
}

////////////////////////
static inline ustring __create_text(ustring nodename, ustring nodecontents) {
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

static ustring __id = ""; 

//This whole method is fairly awful. 
static ustring __recursive_strip(Node * node) {
	
	const TextNode * nodeText = dynamic_cast<const TextNode*>(node);
	
	if(nodeText) {
		
		if(nodeText->is_white_space()) {
			return "";
		}
		
		return nodeText->get_content();
	}
	
	const Element * nodeElement = dynamic_cast<const Element*>(node);
	
	auto nodelist = nodeElement->get_children();
	
	ustring value = "";
	
	for(auto niter = nodelist.begin(); niter != nodelist.end(); ++niter)
	{
		Node * childNode = *niter; 
		
		//Still still Genuinely horrible. 
		const Element* childElement = dynamic_cast<const Element*>(childNode);
		const TextNode * childText = dynamic_cast<const TextNode*>(childNode);
		
		if(!childElement && !childText) continue;
		
		if(childText) {
			
			value += __recursive_strip(childNode);
			
		}
		
		if(childElement) {
			
			ustring childname = childElement->get_name();
			
			if(childname.compare("i") == 0) {
				//italic. 
				value += __create_text("i", __recursive_strip(childNode));
			}
			else if(childname.compare("b") == 0) {
				//bold;
				value += __create_text("b", __recursive_strip(childNode));
			}
			else if(childname.compare("big") == 0) {
				value += __create_text("big", __recursive_strip(childNode));
			}
			else if(childname.compare("s") == 0) {
				//strikethrough
				value += __create_text("s", __recursive_strip(childNode));
			}
			else if(childname.compare("sub") == 0) {
				value += __create_text("sub", __recursive_strip(childNode));
			}
			else if(childname.compare("sup") == 0) {
				value += __create_text("sup", __recursive_strip(childNode));
			}
			else if(childname.compare("small") == 0) {
				value += __create_text("small", __recursive_strip(childNode));
			}
			else if(childname.compare("tt") == 0) {
				//monospace
				value += __create_text("tt", __recursive_strip(childNode));
			}
			else if(childname.compare("u") == 0) {
				//underline
				value += __create_text("u", __recursive_strip(childNode));
			}
			else if(childname.compare("a") == 0) {
				//specific bheaviour for stripping hyperlinks within the text. 
				//I suspect that I'll have to come back to this, but at the moment I'm 
				//not completely sure how to handle it. 
				value += __recursive_strip(childNode);
			}
			
		}
		
	}
	
	return value; 
	
}

static void __recursive_find(vector<ContentItem> & items, path file, Node * node) {
	
	auto nlist = node->get_children();
	
	for(auto niter = nlist.begin(); niter != nlist.end(); ++niter)
	{
		Node * ntmp = *niter; 
		
		//Still still Genuinely horrible. 
		const Element* tmpnode = dynamic_cast<const Element*>(ntmp);
		
		if(!tmpnode) continue;
			
		ustring tmpnodename = tmpnode->get_name();
		
		if(tmpnodename.compare("p") != 0 &&
			tmpnodename.compare("h1") != 0 &&
			tmpnodename.compare("h2") != 0) {
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
			
			const auto attributes = tmpnode->get_attributes();
			for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
			{
				const Attribute* attribute = *iter;
				
				if(attribute->get_name().compare("id") == 0) __id = attribute->get_value();
			}
			
			ustring content = __recursive_strip(ntmp);
					
			if(content.compare("") == 0) continue; 
					
			#ifdef DEBUG
			cout << tmpnode->get_name()  << " " << __id <<endl; 
			cout << " \t " << content << endl; 
			#endif			
			
			ContentItem ci(ct, file, __id, content);
			
			items.push_back(ci);
		
		}
		
	}
	
}

Content::Content(vector<path> _files) :
	files(_files)
{
	
	for(auto file : files) {
		
		if(!exists(file)) {
			throw std::runtime_error("Content file specified in OPF file does not exist!");
		}
		
		#ifdef DEBUG
		cout << "Loading content file " << file << endl; 
		#endif
		
		DomParser parser; 
		parser.parse_file(file.string());
		
		Node* root = parser.get_document()->get_root_node();
		
		ustring rootname = root->get_name();
		
		if(rootname.compare("html") != 0) {
			throw std::runtime_error("Linked content file isn't HTML. So we can't read it. Mostly through laziness.");
		}
	
		auto nlist = root->get_children();
	
		for(auto niter = nlist.begin(); niter != nlist.end(); ++niter)
		{
			Node * ntmp = *niter; 
			
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
	files(cpy.files), 
	items(cpy.items)
{
	
}

Content::Content(Content && mv) :
	files(move(mv.files)),
	items(move(mv.items))
{
	
}

Content& Content::operator =(const Content& cpy) { 
	files = cpy.files;
	items = cpy.items;
	return *this; 
	
}

Content& Content::operator =(Content && mv) {
	files = move(mv.files); 
	items = move(mv.items);
	return *this;
}
	
Content::~Content() {
	
}