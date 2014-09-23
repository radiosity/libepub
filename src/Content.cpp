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
using std::string;

#ifdef DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace boost::filesystem;
using namespace xmlpp;

ContentItem::ContentItem(ContentType _type, CSSClass _cssclass, path _file, ustring _id, ustring _content, ustring _stripped_content) :
	type(_type),
	cssclass(_cssclass),
	file(_file),
	id(_id),
	content(_content),
	stripped_content(_stripped_content)
{
}

ContentItem::ContentItem(ContentItem const & cpy)  :
	type(cpy.type),
	cssclass(cpy.cssclass),
	file(cpy.file),
	id(cpy.id),
	content(cpy.content),
	stripped_content(cpy.stripped_content)
{
}

ContentItem::ContentItem(ContentItem && mv)  :
	type(move(mv.type)),
	cssclass(move(mv.cssclass)),
	file(move(mv.file)),
	id(move(mv.id)),
	content(move(mv.content)),
	stripped_content(move(mv.stripped_content))
{
}

ContentItem & ContentItem::operator =(const ContentItem & cpy)
{
	type = cpy.type;
	cssclass = cpy.cssclass;
	file = cpy.file;
	id = cpy.id;
	content = cpy.content;
	stripped_content = cpy.stripped_content;
	return *this;
}

ContentItem & ContentItem::operator =(ContentItem && mv)
{
	type = move(mv.type);
	cssclass = move(mv.cssclass);
	file = move(mv.file);
	id = move(mv.id);
	content = move(mv.content);
	stripped_content = move(mv.stripped_content);
	return *this;
}

ContentItem::~ContentItem()
{
}

///////////////////////
namespace {
	inline ustring __create_text(const ustring & nodename, const ustring & nodecontents)
	{
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

	string i_key;
	string b_key;
	string big_key;
	string s_key;
	string sub_key;
	string sup_key;
	string small_key;
	string tt_key;
	string u_key;
	string a_key;
	string span_key;
	string class_key;
	string hr_key;
	string p_key;
	string h1_key;
	string h2_key;
	string id_key;
	string _blank_key;

	//This whole method is fairly awful.
	pair<ustring, ustring> __recursive_strip(vector<ContentItem> & items, const CSS & classes, const path file, const Node * const node)
	{

		const TextNode * nodeText = dynamic_cast<const TextNode *>(node);

		if(nodeText) {
			if(nodeText->is_white_space()) {
				return pair<ustring, ustring>("", "");
			}

			ustring content = nodeText->get_content();
			return pair<ustring, ustring>(content, content);
		}

		const Element * nodeElement = dynamic_cast<const Element *>(node);
		const auto nodelist = nodeElement->get_children();

		ustring value = "";
		ustring value_stripped = "";

		for(auto niter = nodelist.begin(); niter != nodelist.end(); ++niter) {

			const Node * childNode = *niter;

			//Still still Genuinely horrible.
			const Element * childElement = dynamic_cast<const Element *>(childNode);
			const TextNode * childText = dynamic_cast<const TextNode *>(childNode);

			if(!childElement && !childText) {
				continue;
			}

			if(childText) {
				pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
				value += res.first;
				value_stripped += res.second;
			}

			if(childElement) {

				const ustring childname = childElement->get_name();
				const string childname_key = childname.collate_key();

				if(childname_key == i_key) {
					//italic.
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("i", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == b_key) {
					//bold;
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("b", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == big_key) {
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("big", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == s_key) {
					//strikethrough
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("s", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == sub_key) {
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("sub", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == sup_key) {
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("sup", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == small_key) {
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("i", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == tt_key) {
					//monospace
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("tt", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == u_key) {
					//underline
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += __create_text("u", res.first);
					value_stripped += res.second;
				}
				else if(childname_key == a_key) {
					//specific bheaviour for stripping hyperlinks within the text.
					//I suspect that I'll have to come back to this, but at the moment I'm
					//not completely sure how to handle it.
					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);
					value += res.first;
					value_stripped += res.second;
				}
				else if(childname_key == span_key) {
					//specific bheaviour for stripping span tags
					//Try to find a class attribute.
					CSSClass tmp;
					const auto attributes = childElement->get_attributes();

					for(auto iter = attributes.begin(); iter != attributes.end(); ++iter) {

						const Attribute * attribute = *iter;

						if(attribute->get_name().collate_key() == class_key) {
							//We've found a class here.
							ustring cname = attribute->get_value();
							//Need to do this better in the future:
							tmp = classes.get_class(ustring(".") + cname);
						}
					}

					pair<ustring, ustring> res = __recursive_strip(items, classes, file, childNode);

					if(tmp.fontweight == FONTWEIGHT_BOLD) {
						value += __create_text("b", res.first);
					}
					else if (tmp.fontstyle == FONTSTYLE_ITALIC) {
						value += __create_text("i", res.first);
					}
					else {
						value += res.first;
					}

					value_stripped += res.second;

				}
				else if(childname_key == hr_key)  {
					//What to do if this is a nested <hr> tag within (frequently)
					//a <p> tag.
					ContentType ct = HR;

					//See if we can find a CSS class for this.
					CSSClass cssclass = classes.get_class("hr");
					ContentItem ci(ct, cssclass, file, __id, "", "");

					items.push_back(ci);

					value = "";
					value_stripped = "";
				}
			}
		}

		return pair<ustring, ustring>(value, value_stripped);
	}

	void __recursive_find(vector<ContentItem> & items, const CSS & classes, const path file, const Node * const node)
	{
		const auto nlist = node->get_children();

		for(auto niter = nlist.begin(); niter != nlist.end(); ++niter) {

			const Node * ntmp = *niter;

			//Still still Genuinely horrible.
			const Element * tmpnode = dynamic_cast<const Element *>(ntmp);

			if(!tmpnode) {
				continue;
			}

			const ustring tmpnodename = tmpnode->get_name();
			const string name_key = tmpnodename.collate_key();

			if(name_key != p_key && name_key !=  h1_key && name_key != h2_key && name_key != hr_key) {
				__recursive_find(items, classes, file, ntmp);
			}
			else {

				ContentType ct = P;
				//Try to get a class for the content type.
				CSSClass cssclass;

				if(name_key == p_key) {
					ct = P;
					cssclass = classes.get_class("p");
				}
				else if (name_key == h1_key) {
					ct = H1;
					cssclass = classes.get_class("h1");
				}
				else if (name_key == h2_key) {
					ct = H2;
					cssclass = classes.get_class("h2");
				}
				else if (name_key == hr_key) {
					ct = HR;
					cssclass = classes.get_class("hr");
				}

				const auto attributes = tmpnode->get_attributes();

				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter) {
					const Attribute * attribute = *iter;

					const string attr_key = attribute->get_name().collate_key();

					if(attr_key == id_key) {
						__id = attribute->get_value();
					}

					if(attr_key == class_key) {
						//We've found an additional class here.
						ustring cname = attribute->get_value();
						CSSClass tmp = classes.get_class(cname);
						cssclass.add(tmp);
					}
				}

				pair<ustring, ustring> content = __recursive_strip(items, classes, file, ntmp);

				if(ct != HR && content.first.empty()) {
					continue;
				}

				#ifdef DEBUG
				cout << tmpnode->get_name()  << " " << __id << endl;
				cout << " \t " << content.first << endl;
				cout << " \t " << content.second << endl;
				#endif
				ContentItem ci(ct, cssclass, file, __id, content.first, content.second);
				items.push_back(ci);

			}
		}
	}
} // end anonymous namespace

Content::Content(CSS & _classes, vector<path> _files) :
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

		//The DomParser futzes with the locale,
		//which means that the collation keys
		//created _before_ this will be incompatible
		//with the collation keys created during the
		//recursive methods.
		//
		//So, setup the keys here.
		i_key = ustring("i").collate_key();
		b_key = ustring("b").collate_key();
		big_key = ustring("big").collate_key();
		s_key = ustring("s").collate_key();
		sub_key = ustring("sub").collate_key();
		sup_key = ustring("sup").collate_key();
		small_key = ustring("small").collate_key();
		tt_key = ustring("tt").collate_key();
		u_key = ustring("u").collate_key();
		a_key = ustring("a").collate_key();
		span_key = ustring("span").collate_key();
		class_key = ustring("class").collate_key();
		hr_key = ustring("hr").collate_key();
		p_key = ustring("p").collate_key();
		h1_key = ustring("h1").collate_key();
		h2_key = ustring("h2").collate_key();
		id_key = ustring("id").collate_key();
		_blank_key = ustring ("").collate_key();

		const Node * root = parser.get_document()->get_root_node();
		const ustring rootname = root->get_name();

		if(rootname.compare("html") != 0) {
			throw std::runtime_error("Linked content file isn't HTML. So we can't read it. Mostly through laziness.");
		}

		const auto nlist = root->get_children();

		for(auto niter = nlist.begin(); niter != nlist.end(); ++niter) {

			const Node * ntmp = *niter;
			//Still still Genuinely horrible.
			const Element * tmpnode = dynamic_cast<const Element *>(ntmp);

			if(!tmpnode) {
				continue;
			}

			if(tmpnode->get_name().compare("body") == 0)  {
				__recursive_find(items, classes, file, ntmp);
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

Content & Content::operator =(const Content & cpy)
{
	classes = cpy.classes;
	files = cpy.files;
	items = cpy.items;
	return *this;
}

Content & Content::operator =(Content && mv)
{
	classes = mv.classes;
	files = move(mv.files);
	items = move(mv.items);
	return *this;
}

Content::~Content()
{
}

void Content::save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index)
{

	int rc;
	char * errmsg;

	const string content_table_sql = "CREATE TABLE IF NOT EXISTS content("  \
	                                 "content_id 				INTEGER PRIMARY KEY," \
	                                 "epub_file_id			INTEGER NOT NULL," \
	                                 "opf_id 				INTEGER NOT NULL," \
	                                 "type			 		INTEGER NOT NULL," \
	                                 "css_class		 		TEXT NOT NULL," \
	                                 "filename	 			TEXT NOT NULL," \
	                                 "id			 		TEXT NOT NULL," \
	                                 "content	 			TEXT NOT NULL," \
	                                 "stripped_content		TEXT NOT NULL) ;";
	sqlite3_exec(db, content_table_sql.c_str(), NULL, NULL, &errmsg);

	//Tables created.

	sqlite3_stmt * content_insert;

	const string content_insert_sql = "INSERT INTO content (epub_file_id, opf_id, type, css_class, filename, id, content, stripped_content) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

	rc = sqlite3_prepare_v2(db, content_insert_sql.c_str(), -1, &content_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	for(auto & contentitem : items) {

		sqlite3_bind_int(content_insert, 1, epub_file_id);
		sqlite3_bind_int(content_insert, 2, opf_index);
		sqlite3_bind_int(content_insert, 3, (int) contentitem.type);
		sqlite3_bind_text(content_insert, 4, contentitem.cssclass.name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(content_insert, 5, contentitem.file.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(content_insert, 6,  contentitem.id.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(content_insert, 7, contentitem.content.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(content_insert, 8, contentitem.stripped_content.c_str(), -1, SQLITE_STATIC);

		int result = sqlite3_step(content_insert);

		if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
			throw - 1;
		}

		sqlite3_reset(content_insert);

	}

	sqlite3_finalize(content_insert);

}
