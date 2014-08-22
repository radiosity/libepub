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

#include "OPF.hpp"

#include <utility>
#include <boost/filesystem.hpp>
#include <libxml++/libxml++.h>
#include <iostream>
#include <exception>

using std::move; 
using std::pair;

using std::cout; 
using std::ifstream;
using std::endl; 

using namespace boost::filesystem;
using namespace xmlpp;

//// MetadataItem:

MetadataItem::MetadataItem(MetadataType _type, ustring _contents) : type(_type), contents(_contents) {
	
}
		
MetadataItem::MetadataItem(MetadataItem const & cpy) : 
	type(cpy.type), 
	contents(cpy.contents), 
	other_tags(cpy.other_tags)
{
	
}

MetadataItem::MetadataItem(MetadataItem && mv)  : 
	type(move(mv.type)), 
	contents(move(mv.contents)) ,
	other_tags(move(mv.other_tags)) 
{
	
}

MetadataItem& MetadataItem::operator =(const MetadataItem& cpy)  {
	type = cpy.type; 
	contents = cpy.contents; 
	other_tags = cpy.other_tags;
	return *this;
}

MetadataItem& MetadataItem::operator =(MetadataItem && mv)  {
	type = move(mv.type);
	contents = move(mv.contents);
	other_tags = move(mv.other_tags);
	return *this; 
}
	
MetadataItem::~MetadataItem() {
	
}

void MetadataItem::add_attribute(ustring name, ustring contents) {
	other_tags.insert(pair<ustring, ustring>(name, contents));
}

//// ManifestItem:

ManifestItem::ManifestItem(ustring _href, ustring _id, ustring _media_type) :
	href(_href),
	id(_id),
	media_type(_media_type) 
{
		
}
		
ManifestItem::ManifestItem(ManifestItem const & cpy):
	href(cpy.href),
	id(cpy.id),
	media_type(cpy.media_type) 
{
	
}

ManifestItem::ManifestItem(ManifestItem && mv) :
	href(move(mv.href)),
	id(move(mv.id)),
	media_type(move(mv.media_type))
{
		
}

ManifestItem& ManifestItem::operator =(const ManifestItem& cpy) {
	href = cpy.href; 
	id = cpy.id; 
	media_type = cpy.media_type; 
	return *this;
}

ManifestItem& ManifestItem::operator =(ManifestItem && mv)  {
	href = move(mv.href);
	id = move(mv.id);
	media_type = move(mv.media_type);
	return *this;
}
	
ManifestItem::~ManifestItem() {
	
}

//// SpineItem:

SpineItem::SpineItem( ustring _idref, bool _linear) :
	idref(_idref), 
	linear(_linear)
{

}	

SpineItem::SpineItem(SpineItem const & cpy) :
	idref(cpy.idref),
	linear(cpy.linear)
{
	
}

SpineItem::SpineItem(SpineItem && mv) :
	idref(move(mv.idref)),
	linear(move(mv.linear))
{
	
}

SpineItem& SpineItem::operator =(const SpineItem& cpy) {
	idref = cpy.idref; 
	linear = cpy.linear; 
	return *this; 
}

SpineItem& SpineItem::operator =(SpineItem && mv) {
	idref = move(mv.idref);
	linear = move(mv.linear);
	return *this;
}
	
SpineItem::~SpineItem() {

}

//// OPF:

OPF::OPF(path to_dir, ustring file) {
	
	to_file = to_dir; 
	to_file /= file.raw();
	
	cout << to_file << endl;;
	
	if(!exists(to_file)) {
		throw std::runtime_error("Content file specified in rootfiles does not exist!");
	}
	
	DomParser parser; 
	parser.parse_file(to_file.string());
	
	
	Node* root = parser.get_document()->get_root_node();
	
	ustring rootname = root->get_name();
	
	if(rootname.compare("package") != 0) {
		throw std::runtime_error("OPF content file does not contain a <package> node as root");
	}
	
	auto nlist = root->get_children();
	
	for(auto niter = nlist.begin(); niter != nlist.end(); ++niter)
	{
		Node * ntmp = *niter; 
		
		//Genuinely horrible. 
		const Element* mdnode = dynamic_cast<const Element*>(ntmp);
		
		if(!mdnode) continue;
		
		if(mdnode->get_name().compare("metadata") == 0)  {
			
			auto mdlist = mdnode->get_children();
				
			for(auto mditer = mdlist.begin(); mditer != mdlist.end(); ++mditer)
			{
				
				Node * metadatatmp = *mditer; 
		
				//Still genuinely horrible. 
				const Element* metadatanode = dynamic_cast<const Element*>(metadatatmp);
				
				if(!metadatanode) continue;
				
				ustring name = metadatanode->get_name();
				
				MetadataType mdtype = MetadataType_from_ustring(name);
				
				if(mdtype == UNKNOWN) {
					cout <<  "Hmm, unknown Metadata type. Which is: " << name << endl; 
				}
				
				auto textnd = metadatanode->get_child_text();
				ustring content = ""; 
				if(textnd) content = textnd->get_content();
				
				cout << "Node is " << name << " - " << content << endl; 
				
				MetadataItem md(mdtype, content);
			
				const auto attributes = metadatanode->get_attributes();
				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
				{
					const Attribute* attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();
					
					md.add_attribute(attrname, attrvalue);

				}
				
				metadata.insert(pair<MetadataType, MetadataItem>(mdtype, md));
			
			}
		}
		
		if(mdnode->get_name().compare("manifest") == 0)  {
			
			auto mlist = mdnode->get_children();
				
			for(auto miter = mlist.begin(); miter != mlist.end(); ++miter)
			{
				
				Node * itemtmp = *miter; 
		
				//Still genuinely horrible. 
				const Element* itemnode = dynamic_cast<const Element*>(itemtmp);
				
				if(!itemnode) continue;
				
				ustring name = itemnode->get_name();
				
				if(name.compare("item")!=0) continue;
				
				ustring href, id, media_type; 
			
				const auto attributes = itemnode->get_attributes();
				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
				{
					const Attribute* attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();
					
					if(attrname.compare("href")==0) href = attrvalue;
					else if(attrname.compare("id")==0) id = attrvalue; 
					else if(attrname.compare("media-type")==0) media_type = attrvalue; 

				}
				
				ManifestItem tmp(href, id, media_type);
				
				manifest.insert(pair<ustring, ManifestItem>(id, tmp)); 
				
				cout << "Manifest href: " << href << " id " << id << " media type " << media_type << endl; 
			
			}
		}
		
		if(mdnode->get_name().compare("spine") == 0)  {
			
			auto mlist = mdnode->get_children();
			
			{
				const auto attributes = mdnode->get_attributes();
				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
				{
					const Attribute* attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();
					
					if(attrname.compare("toc")==0) spine_toc = attrvalue;
					
					cout << "toc is " << attrvalue << endl; 
				}	
			}
				
			for(auto miter = mlist.begin(); miter != mlist.end(); ++miter)
			{
				
				Node * itemtmp = *miter; 
		
				//Still genuinely horrible. 
				const Element* itemnode = dynamic_cast<const Element*>(itemtmp);
				
				if(!itemnode) continue;
				
				ustring name = itemnode->get_name();
				
				if(name.compare("itemref")!=0) continue;
				
				ustring idref; 
				bool linear = true; 
			
				const auto attributes = itemnode->get_attributes();
				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter)
				{
					const Attribute* attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();
					
					if(attrname.compare("idref")==0) idref = attrvalue;
					else if(attrname.compare("linear")==0) {
						if(attrvalue.compare("yes")==0) linear = true; 
						else linear = false; 
					}; 

				}
				
				SpineItem tmp(idref, linear);
				
				spine.push_back(tmp);
				
				cout << "Spine idref: " << idref << " linear " << linear  << endl; 
			
			}
		}
		
			
		
	}
	
}

OPF::OPF(OPF const & cpy) :
	to_file(cpy.to_file),
	metadata(cpy.metadata)
{
	
}

OPF::OPF(OPF && mv) : 
	to_file(move(mv.to_file)),
	metadata(move(mv.metadata))
{
	
}

OPF& OPF::operator =(const OPF& cpy) {
	to_file = cpy.to_file; 
	metadata = cpy.metadata; 
	return *this;
}

OPF& OPF::operator =(OPF && mv)  {
	to_file = move(mv.to_file);
	metadata = move(mv.metadata);
	return *this; 
}
	
OPF::~OPF() {
	
}

ManifestItem OPF::find_manifestitem_by_id(ustring id) {
	
	auto sz = manifest.count(id);
	
	if(sz == 0) throw std::runtime_error("Cannot find the id. This means that the Epub is malformed.");
	
	auto it = manifest.find(id);
	
	auto pr = *it; 
	
	return pr.second;
	
}