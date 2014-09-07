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

#include "Container.hpp"

#include <vector>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>
#include <stdlib.h>
#include <utility>
#include <exception>

using std::vector;
using std::move;

#ifdef DEBUG
#include <iostream>
using std::cout; 
using std::endl; 
#endif

using namespace boost::filesystem;
using namespace Glib;
using namespace xmlpp;

RootFile::RootFile(ustring _m, ustring _f)  : media_type(_m), full_path(_f) {
	
}
	
RootFile::RootFile(RootFile const & cpy) :
	media_type(cpy.media_type),
	full_path(cpy.full_path) {
		
}

RootFile::RootFile(RootFile && mv) :
	media_type(move(mv.media_type)),
	full_path(move(mv.full_path)) {
		
}
	
RootFile& RootFile::operator =(const RootFile& cpy) { 
	media_type = cpy.media_type;
	full_path = cpy.full_path;
	return *this; 
}

RootFile& RootFile::operator =(RootFile && mv) { 
	media_type = move(mv.media_type);
	full_path = move(mv.full_path);
	return *this; 
}
	
RootFile::~RootFile() {
	
}

void Container::load(path to_dir) {
	
	auto to_container = to_dir; 
	to_container /= "META-INF";
	to_container /= "container.xml";
	
	if(!exists(to_container)) {
		throw std::runtime_error("container.xml does not exist within META-INF dir");
	}
	
	DomParser parser; 
	parser.parse_file(to_container.string());
	
	Node* root = parser.get_document()->get_root_node();
	
	ustring rootname = root->get_name();
	
	if(rootname.compare("container") != 0) {
		throw std::runtime_error("container.xml does not contain a <container> node as root");
	}
	
	auto rfslist = root->get_children();
	
	for(auto rfsiter = rfslist.begin(); rfsiter != rfslist.end(); ++rfsiter)
	{
		Node * rfstmp = *rfsiter; 
		
		//Genuinely horrible. 
		const Element* rfsnode = dynamic_cast<const Element*>(rfstmp);
		
		if(!rfsnode) continue;
		
		if(rfsnode->get_name().compare("rootfiles") == 0) {
			
			auto rflist = rfsnode->get_children();
			
			for(auto rfiter = rflist.begin(); rfiter != rflist.end(); ++rfiter)
			{
				
				Node * rftmp = *rfiter; 
		
				//Genuinely horrible. 
				const Element* rfnode = dynamic_cast<const Element*>(rftmp);
				
				if(!rfnode) continue;
				
				if(rfnode->get_name().compare("rootfile") == 0)  {
					
					ustring mt;
					ustring fp; 
					
					const auto attributes = rfnode->get_attributes();
					for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
					{
						const Attribute* attribute = *iter;
						const ustring namespace_prefix = attribute->get_namespace_prefix();

						//cout << "  Attribute ";
						//if(!namespace_prefix.empty()) cout << namespace_prefix  << ":";
						//cout << attribute->get_name() << " = " << attribute->get_value() << endl;
						
						if(attribute->get_name().compare("media-type") == 0) mt = attribute->get_value();
						else if(attribute->get_name().compare("full-path") == 0) fp = attribute->get_value();
					}
					
					#ifdef DEBUG 
					cout << "Root file " << mt << " " << fp << endl; 
					#endif
					
					rootfiles.push_back(RootFile(mt, fp));
					
				}
				
			}
			
		}					
		
	}
			
}

Container::Container() {
	
}

Container::Container(Container const & cpy) : rootfiles(cpy.rootfiles) {

}
	
Container::Container(Container && mv) : rootfiles(move(mv.rootfiles)) {
	
}

Container& Container::operator =(const Container& cpy) { 
	rootfiles = cpy.rootfiles;
	return *this; 
}

Container& Container::operator =(Container && mv) { 
	rootfiles = move(mv.rootfiles);
	return *this; 
}

Container::~Container() {}






