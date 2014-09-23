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

#ifndef OPF_HEADER
#define OPF_HEADER

#include <string>
#include <map>
#include <vector>
#include <boost/filesystem.hpp>
#include <utility>
#include <glibmm.h>
#include <sqlite3.h>

using std::multimap;
using std::string;
using std::map;
using std::vector;

using namespace boost::filesystem;
using namespace Glib;

enum MetadataType {
	TITLE = 0,
	CREATOR = 1,
	SUBJECT = 2,
	DESCRIPTION = 3,
	PUBLISHER = 4,
	CONTRIBUTOR = 5,
	DATE = 6,
	TYPE = 7,
	FORMAT = 8,
	IDENTIFIER = 9,
	SOURCE = 10,
	LANGUAGE = 11,
	RELATION = 12,
	COVERAGE = 13,
	RIGHTS = 14,
	META = 15,
	//
	UNKNOWN = 16
};

MetadataType inline MetadataType_from_ustring(const ustring str)
{
	if(str.compare("title") == 0) {
		return TITLE;
	}

	if(str.compare("creator") == 0) {
		return CREATOR;
	}

	if(str.compare("subject") == 0) {
		return SUBJECT;
	}

	if(str.compare("description") == 0) {
		return DESCRIPTION;
	}

	if(str.compare("publisher") == 0) {
		return PUBLISHER;
	}

	if(str.compare("contributor") == 0) {
		return CONTRIBUTOR;
	}

	if(str.compare("date") == 0) {
		return DATE;
	}

	if(str.compare("type") == 0) {
		return TYPE;
	}

	if(str.compare("format") == 0) {
		return FORMAT;
	}

	if(str.compare("identifier") == 0) {
		return IDENTIFIER;
	}

	if(str.compare("source") == 0) {
		return SOURCE;
	}

	if(str.compare("language") == 0) {
		return LANGUAGE;
	}

	if(str.compare("relation") == 0) {
		return RELATION;
	}

	if(str.compare("coverage") == 0) {
		return COVERAGE;
	}

	if(str.compare("rights") == 0) {
		return RIGHTS;
	}

	if(str.compare("meta") == 0) {
		return META;
	}
	else {
		return UNKNOWN;
	}
}

class MetadataItem {

	public:
		MetadataType type;
		ustring contents;
		multimap<ustring, ustring> other_tags;

		MetadataItem(MetadataType _type, ustring _contents);

		MetadataItem(MetadataItem const & cpy);
		MetadataItem(MetadataItem && mv) ;
		MetadataItem & operator =(const MetadataItem & cpy);
		MetadataItem & operator =(MetadataItem && mv) ;

		~MetadataItem();

		void add_attribute(ustring name, ustring contents);

};

class ManifestItem {

	public:
		ustring href;
		ustring id;
		ustring media_type;

		ManifestItem(ustring _href, ustring _id, ustring _media_type);

		ManifestItem(ManifestItem const & cpy);
		ManifestItem(ManifestItem && mv) ;
		ManifestItem & operator =(const ManifestItem & cpy);
		ManifestItem & operator =(ManifestItem && mv) ;

		~ManifestItem();
};

class SpineItem {

	public:
		ustring idref;
		bool linear;

		SpineItem( ustring _idref, bool _linear = true);

		SpineItem(SpineItem const & cpy);
		SpineItem(SpineItem && mv) ;
		SpineItem & operator =(const SpineItem & cpy);
		SpineItem & operator =(SpineItem && mv) ;

		~SpineItem();

};

class OPF {

	public:
		path to_file;
		multimap<MetadataType, MetadataItem> metadata;
		map<ustring, ManifestItem> manifest;
		vector<SpineItem> spine;
		ustring spine_toc;

		OPF(path to_dir, ustring file);
		OPF(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index);

		OPF(OPF const & cpy);
		OPF(OPF && mv) ;
		OPF & operator =(const OPF & cpy);
		OPF & operator =(OPF && mv) ;

		~OPF();

		ManifestItem find_manifestitem_by_id(ustring id);
		vector<ManifestItem> find_manifestitems_by_type(ustring type);

		void save_to(sqlite3 * const db, const unsigned int epub_file_id, const unsigned int opf_index);

};

#endif