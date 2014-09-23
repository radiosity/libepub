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
#include <exception>

#include "SQLiteUtils.hpp"

using std::move;
using std::pair;

#ifdef DEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace boost::filesystem;
using namespace xmlpp;

//// MetadataItem:

MetadataItem::MetadataItem(MetadataType _type, ustring _contents) : type(_type), contents(_contents)
{
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

MetadataItem & MetadataItem::operator =(const MetadataItem & cpy)
{
	type = cpy.type;
	contents = cpy.contents;
	other_tags = cpy.other_tags;
	return *this;
}

MetadataItem & MetadataItem::operator =(MetadataItem && mv)
{
	type = move(mv.type);
	contents = move(mv.contents);
	other_tags = move(mv.other_tags);
	return *this;
}

MetadataItem::~MetadataItem()
{
}

void MetadataItem::add_attribute(ustring name, ustring contents)
{
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

ManifestItem & ManifestItem::operator =(const ManifestItem & cpy)
{
	href = cpy.href;
	id = cpy.id;
	media_type = cpy.media_type;
	return *this;
}

ManifestItem & ManifestItem::operator =(ManifestItem && mv)
{
	href = move(mv.href);
	id = move(mv.id);
	media_type = move(mv.media_type);
	return *this;
}

ManifestItem::~ManifestItem()
{
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

SpineItem & SpineItem::operator =(const SpineItem & cpy)
{
	idref = cpy.idref;
	linear = cpy.linear;
	return *this;
}

SpineItem & SpineItem::operator =(SpineItem && mv)
{
	idref = move(mv.idref);
	linear = move(mv.linear);
	return *this;
}

SpineItem::~SpineItem()
{
}

//// OPF:

OPF::OPF(path to_dir, ustring file)
{
	to_file = to_dir;
	to_file /= file.raw();

	#ifdef DEBUG
	cout << to_file << endl;;
	#endif

	if(!exists(to_file)) {
		throw std::runtime_error("Content file specified in rootfiles does not exist!");
	}

	DomParser parser;
	parser.parse_file(to_file.string());
	Node * root = parser.get_document()->get_root_node();
	ustring rootname = root->get_name();

	if(rootname.compare("package") != 0) {
		throw std::runtime_error("OPF content file does not contain a <package> node as root");
	}

	auto nlist = root->get_children();

	//setup collation keys for improved speed:
	const string metadata_key = ustring("metadata").collate_key();
	const string manifest_key = ustring("manifest").collate_key();
	const string item_key = ustring("item").collate_key();
	const string href_key = ustring("href").collate_key();
	const string id_key = ustring("id").collate_key();
	const string media_type_key = ustring("media-type").collate_key();
	const string spine_key = ustring("spine").collate_key();
	const string toc_key = ustring("toc-type").collate_key();
	const string itemref_key = ustring("itemref").collate_key();
	const string idref_key = ustring("idref").collate_key();
	const string linear_key = ustring("linear").collate_key();
	const string yes_key = ustring("yes").collate_key();

	for(auto niter = nlist.begin(); niter != nlist.end(); ++niter) {

		Node * ntmp = *niter;

		//Genuinely horrible.
		const Element * mdnode = dynamic_cast<const Element *>(ntmp);

		if(!mdnode) {
			continue;
		}

		if(mdnode->get_name().collate_key() == metadata_key)  {

			auto mdlist = mdnode->get_children();

			for(auto mditer = mdlist.begin(); mditer != mdlist.end(); ++mditer) {

				Node * metadatatmp = *mditer;

				//Still genuinely horrible.
				const Element * metadatanode = dynamic_cast<const Element *>(metadatatmp);

				if(!metadatanode) {
					continue;
				}

				ustring name = metadatanode->get_name();
				MetadataType mdtype = MetadataType_from_ustring(name);

				#ifdef DEBUG

				if(mdtype == UNKNOWN) {
					cout <<  "Hmm, unknown Metadata type. Which is: " << name << endl;
				}

				#endif

				auto textnd = metadatanode->get_child_text();
				ustring content = "";

				if(textnd) {
					content = textnd->get_content();
				}

				#ifdef DEBUG
				cout << "Node is " << name << " - " << content << endl;
				#endif

				MetadataItem md(mdtype, content);
				const auto attributes = metadatanode->get_attributes();

				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter) {
					const Attribute * attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();
					md.add_attribute(attrname, attrvalue);
				}

				metadata.insert(pair<MetadataType, MetadataItem>(mdtype, md));

			}
		}

		if(mdnode->get_name().collate_key() == manifest_key)  {

			auto mlist = mdnode->get_children();

			for(auto miter = mlist.begin(); miter != mlist.end(); ++miter) {

				Node * itemtmp = *miter;

				//Still genuinely horrible.
				const Element * itemnode = dynamic_cast<const Element *>(itemtmp);

				if(!itemnode) {
					continue;
				}

				ustring name = itemnode->get_name();

				if(name.collate_key() != item_key) {
					continue;
				}

				ustring href, id, media_type;
				const auto attributes = itemnode->get_attributes();

				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter) {
					const Attribute * attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();

					if(attrname.collate_key() == href_key) {
						href = attrvalue;
					}
					else if(attrname.collate_key() == id_key) {
						id = attrvalue;
					}
					else if(attrname.collate_key() == media_type_key) {
						media_type = attrvalue;
					}
				}

				ManifestItem tmp(href, id, media_type);
				manifest.insert(pair<ustring, ManifestItem>(id, tmp));

				#ifdef DEBUG
				cout << "Manifest href: " << href << " id " << id << " media type " << media_type << endl;
				#endif

			}
		}

		if(mdnode->get_name().collate_key() == spine_key)  {

			auto mlist = mdnode->get_children();
			{

				const auto attributes = mdnode->get_attributes();

				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter) {
					const Attribute * attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();

					if(attrname.collate_key() == toc_key) {
						spine_toc = attrvalue;
					}

					#ifdef DEBUG
					cout << "toc is " << attrvalue << endl;
					#endif
				}

			}

			for(auto miter = mlist.begin(); miter != mlist.end(); ++miter) {

				Node * itemtmp = *miter;

				//Still genuinely horrible.
				const Element * itemnode = dynamic_cast<const Element *>(itemtmp);

				if(!itemnode) {
					continue;
				}

				ustring name = itemnode->get_name();

				if(name.collate_key() != itemref_key) {
					continue;
				}

				ustring idref;
				bool linear = true;
				const auto attributes = itemnode->get_attributes();

				for(auto iter = attributes.begin(); iter != attributes.end(); ++iter) {
					const Attribute * attribute = *iter;
					const ustring namespace_prefix = attribute->get_namespace_prefix();
					ustring attrname = attribute->get_name();
					ustring attrvalue = attribute->get_value();

					if(attrname.collate_key() == idref_key) {
						idref = attrvalue;
					}
					else if(attrname.collate_key() == linear_key) {
						if(attrvalue.collate_key() == yes_key) {
							linear = true;
						}
						else {
							linear = false;
						}
					};
				}

				SpineItem tmp(idref, linear);
				spine.push_back(tmp);

				#ifdef DEBUG
				cout << "Spine idref: " << idref << " linear " << linear  << endl;
				#endif

			}
		}
	}
}

OPF::OPF(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index)
{

	int rc;

	const string metadata_select_sql = "SELECT metadata_id, metadata_type, contents FROM metadata WHERE epub_file_id=? AND opf_id=?;";
	const string metadata_tags_select_sql = "SELECT tagname, tagvalue FROM metadata_tags WHERE metadata_id=?;";
	const string manifest_select_sql = "SELECT href, id, media_type FROM manifest WHERE epub_file_id=? AND opf_id=?;";
	const string spine_select_sql = "SELECT idref, linear FROM spine WHERE epub_file_id=? AND opf_id=?;";

	sqlite3_stmt * metadata_select;
	sqlite3_stmt * metadata_tags_select;
	sqlite3_stmt * manifest_select;
	sqlite3_stmt * spine_select;

	rc = sqlite3_prepare_v2(db, metadata_select_sql.c_str(), -1, &metadata_select, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, metadata_tags_select_sql.c_str(), -1, &metadata_tags_select, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, manifest_select_sql.c_str(), -1, &manifest_select, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, spine_select_sql.c_str(), -1, &spine_select, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	sqlite3_bind_int(metadata_select, 1, epub_file_id);
	sqlite3_bind_int(metadata_select, 2, opf_index);

	rc = sqlite3_step(metadata_select);

	while ( rc == SQLITE_ROW ) {

		//Get the basic data
		unsigned int metadata_id = sqlite3_column_int(metadata_select, 0);
		MetadataType mdtype = (MetadataType) sqlite3_column_int(metadata_select, 1);
		ustring content = sqlite3_column_ustring(metadata_select, 2);

		//Create the object
		MetadataItem md(mdtype, content);

		int rc2;

		sqlite3_bind_int(metadata_tags_select, 1, metadata_id);

		rc2 = sqlite3_step(metadata_tags_select);

		while(rc2 == SQLITE_ROW) {

			ustring tagname = sqlite3_column_ustring(metadata_tags_select, 0);
			ustring tagvalue = sqlite3_column_ustring(metadata_tags_select, 1);

			md.add_attribute(tagname, tagvalue);

			rc2 = sqlite3_step(metadata_tags_select);

		}

		metadata.insert(pair<MetadataType, MetadataItem>(mdtype, md));

		sqlite3_reset(metadata_tags_select);

		rc = sqlite3_step(metadata_select);

	}

	sqlite3_bind_int(manifest_select, 1, epub_file_id);
	sqlite3_bind_int(manifest_select, 2, opf_index);

	rc = sqlite3_step(manifest_select);

	while ( rc == SQLITE_ROW ) {

		ustring href = sqlite3_column_ustring(manifest_select, 0);
		ustring id = sqlite3_column_ustring(manifest_select, 1);
		ustring media_type = sqlite3_column_ustring(manifest_select, 2);

		ManifestItem tmp(href, id, media_type);
		manifest.insert(pair<ustring, ManifestItem>(id, tmp));

		rc = sqlite3_step(manifest_select);

	}

	sqlite3_bind_int(spine_select, 1, epub_file_id);
	sqlite3_bind_int(spine_select, 2, opf_index);

	rc = sqlite3_step(spine_select);

	while ( rc == SQLITE_ROW ) {

		ustring idref = sqlite3_column_ustring(spine_select, 0);
		bool linear = (bool) sqlite3_column_int(spine_select, 1);

		SpineItem tmp(idref, linear);
		spine.push_back(tmp);

		rc = sqlite3_step(spine_select);

	}

	sqlite3_finalize(metadata_select);
	sqlite3_finalize(metadata_tags_select);
	sqlite3_finalize(manifest_select);
	sqlite3_finalize(spine_select);

}

OPF::OPF(OPF const & cpy) :
	to_file(cpy.to_file),
	metadata(cpy.metadata),
	manifest(cpy.manifest),
	spine(cpy.spine),
	spine_toc(cpy.spine_toc)
{
}

OPF::OPF(OPF && mv) :
	to_file(move(mv.to_file)),
	metadata(move(mv.metadata)),
	manifest(move(mv.manifest)),
	spine(move(mv.spine)),
	spine_toc(move(mv.spine_toc))
{
}

OPF & OPF::operator =(const OPF & cpy)
{
	to_file = cpy.to_file;
	metadata = cpy.metadata;
	manifest = cpy.manifest;
	spine = cpy.spine;
	spine_toc = cpy.spine_toc;
	return *this;
}

OPF & OPF::operator =(OPF && mv)
{
	to_file = move(mv.to_file);
	metadata = move(mv.metadata);
	manifest = move(mv.manifest);
	spine = move(mv.spine);
	spine_toc = move(mv.spine_toc);
	return *this;
}

OPF::~OPF()
{
}

ManifestItem OPF::find_manifestitem_by_id(ustring id)
{
	auto sz = manifest.count(id);

	if(sz == 0) {
		throw std::runtime_error("Cannot find the id. This means that the Epub is malformed.");
	}

	auto it = manifest.find(id);
	auto pr = *it;
	return pr.second;
}

vector<ManifestItem> OPF::find_manifestitems_by_type(ustring type)
{
	vector<ManifestItem> results;

	//Going to have to do a full scan

	for(pair<ustring, ManifestItem> mi : manifest) {
		if(mi.second.media_type.compare("text/css") == 0) {
			results.push_back(mi.second);
		}
	}

	return results;
}

void OPF::save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index)
{
	int rc;
	char * errmsg;

	const string opf_table_sql = "CREATE TABLE IF NOT EXISTS opf("  \
	                             "epub_file_id INTEGER NOT NULL," \
	                             "opf_id INTEGER NOT NULL) ;";
	sqlite3_exec(db, opf_table_sql.c_str(), NULL, NULL, &errmsg);

	const string metadata_table_sql = "CREATE TABLE IF NOT EXISTS metadata("  \
	                                  "metadata_id INTEGER PRIMARY KEY," \
	                                  "epub_file_id INTEGER NOT NULL," \
	                                  "opf_id INTEGER NOT NULL," \
	                                  "metadata_type INTEGER NOT NULL," \
	                                  "contents TEXT NOT NULL) ;";
	sqlite3_exec(db, metadata_table_sql.c_str(), NULL, NULL, &errmsg);

	const string metadata_tags_table_sql = "CREATE TABLE IF NOT EXISTS metadata_tags("  \
	                                       "metadata_id INTEGER NOT NULL," \
	                                       "tagname TEXT NOT NULL," \
	                                       "tagvalue TEXT NOT NULL) ;";
	sqlite3_exec(db, metadata_tags_table_sql.c_str(), NULL, NULL, &errmsg);

	const string manifest_table_sql = "CREATE TABLE IF NOT EXISTS manifest("  \
	                                  "epub_file_id INTEGER NOT NULL," \
	                                  "opf_id INTEGER NOT NULL," \
	                                  "href TEXT NOT NULL," \
	                                  "id TEXT NOT NULL," \
	                                  "media_type TEXT NOT NULL) ;";
	sqlite3_exec(db, manifest_table_sql.c_str(), NULL, NULL, &errmsg);

	const string spine_table_sql = "CREATE TABLE IF NOT EXISTS spine("  \
	                               "epub_file_id INTEGER NOT NULL," \
	                               "opf_id INTEGER NOT NULL," \
	                               "idref TEXT NOT NULL," \
	                               "linear INTEGER NOT NULL) ;";
	sqlite3_exec(db, spine_table_sql.c_str(), NULL, NULL, &errmsg);

	//Tables created.

	sqlite3_stmt * opf_insert;
	sqlite3_stmt * metadata_insert;
	sqlite3_stmt * metadata_tags_insert;
	sqlite3_stmt * manifest_insert;
	sqlite3_stmt * spine_insert;

	const string opf_insert_sql = "INSERT INTO opf (epub_file_id, opf_id) VALUES (?, ?);";
	const string metadata_insert_sql = "INSERT INTO metadata (epub_file_id, opf_id, metadata_type, contents) VALUES (?, ?, ?, ?);";
	const string metadata_tags_insert_sql = "INSERT INTO metadata_tags (metadata_id, tagname, tagvalue) VALUES (?, ?, ?);";
	const string manifest_insert_sql = "INSERT INTO manifest (epub_file_id, opf_id, href, id, media_type) VALUES (?, ?, ?, ?, ?);";
	const string spine_insert_sql = "INSERT INTO spine (epub_file_id, opf_id, idref, linear) VALUES (?, ?, ?, ?);";

	rc = sqlite3_prepare_v2(db, opf_insert_sql.c_str(), -1, &opf_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, metadata_insert_sql.c_str(), -1, &metadata_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, metadata_tags_insert_sql.c_str(), -1, &metadata_tags_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, manifest_insert_sql.c_str(), -1, &manifest_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	rc = sqlite3_prepare_v2(db, spine_insert_sql.c_str(), -1, &spine_insert, 0);

	if(rc != SQLITE_OK && rc != SQLITE_DONE) {
		throw - 1;
	}

	//Populate the OPF table:
	sqlite3_bind_int(opf_insert, 1, epub_file_id);
	sqlite3_bind_int(opf_insert, 2, opf_index);

	rc = sqlite3_step(opf_insert);

	if(rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
		throw - 1;
	}

	for(auto & mi : metadata) {

		sqlite3_bind_int(metadata_insert, 1, epub_file_id);
		sqlite3_bind_int(metadata_insert, 2, opf_index);
		sqlite3_bind_int(metadata_insert, 3, mi.second.type);
		sqlite3_bind_text(metadata_insert, 4, mi.second.contents.c_str(), -1, SQLITE_STATIC);

		int result = sqlite3_step(metadata_insert);

		if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
			throw - 1;
		}

		sqlite3_reset(metadata_insert);

		//get the new id:
		auto key = sqlite3_last_insert_rowid(db);

		for(auto tags : mi.second.other_tags) {

			sqlite3_bind_int(metadata_tags_insert, 1, key);
			sqlite3_bind_text(metadata_tags_insert, 2, tags.first.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(metadata_tags_insert, 3, tags.second.c_str(), -1, SQLITE_STATIC);
			int result = sqlite3_step(metadata_tags_insert);

			if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
				throw - 1;
			}

			sqlite3_reset(metadata_tags_insert);

		}
	}

	for (auto & mi : manifest) {

		sqlite3_bind_int(manifest_insert, 1, epub_file_id);
		sqlite3_bind_int(manifest_insert, 2, opf_index);
		sqlite3_bind_text(manifest_insert, 3, mi.second.href.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(manifest_insert, 4, mi.second.id.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(manifest_insert, 5, mi.second.media_type.c_str(), -1, SQLITE_STATIC);
		int result = sqlite3_step(manifest_insert);

		if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
			throw - 1;
		}

		sqlite3_reset(manifest_insert);

	}

	for (auto & si : spine) {

		sqlite3_bind_int(spine_insert, 1, epub_file_id);
		sqlite3_bind_int(spine_insert, 2, opf_index);
		sqlite3_bind_text(spine_insert, 3, si.idref.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(spine_insert, 4, (int) si.linear);
		int result = sqlite3_step(spine_insert);

		if(result != SQLITE_OK && result != SQLITE_ROW && result != SQLITE_DONE) {
			throw - 1;
		}

		sqlite3_reset(spine_insert);

	}

	//Create an index for the opf table
	const string opf_index_sql = "CREATE INDEX index_opf ON opf(epub_file_id);";
	sqlite3_exec(db, opf_index_sql.c_str(), NULL, NULL, &errmsg);

	//Create an index for the metadata
	const string metadata_index_sql = "CREATE INDEX index_metadata ON metadata(epub_file_id, opf_id);";
	sqlite3_exec(db, metadata_index_sql.c_str(), NULL, NULL, &errmsg);

	//Create an index for the metadata tags
	const string metadata_tags_index_sql = "CREATE INDEX index_metadata_tags ON metadata_tags(metadata_id);";
	sqlite3_exec(db, metadata_tags_index_sql.c_str(), NULL, NULL, &errmsg);

	//Create an index for the manifest
	const string manifest_index_sql = "CREATE INDEX index_manifest ON manifest(epub_file_id, opf_id);";
	sqlite3_exec(db, manifest_index_sql.c_str(), NULL, NULL, &errmsg);

	//Create an index for the spine
	const string spine_index_sql = "CREATE INDEX index_spine ON spine(epub_file_id, opf_id);";
	sqlite3_exec(db, spine_index_sql.c_str(), NULL, NULL, &errmsg);

	sqlite3_finalize(opf_insert);
	sqlite3_finalize(metadata_insert);
	sqlite3_finalize(metadata_tags_insert);
	sqlite3_finalize(manifest_insert);
	sqlite3_finalize(spine_insert);

}