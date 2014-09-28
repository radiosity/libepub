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

#include <gtest/gtest.h>
#include <sqlite3.h>
#include <boost/filesystem.hpp>

#include "Epub.hpp"

using namespace boost::filesystem;

TEST(EpubTest, Database)
{

	//Unpack the ebook
	ASSERT_NO_THROW(Epub book("books/PrideAndPrejudice.epub"));

	Epub file_book("books/PrideAndPrejudice.epub");

	sqlite3 * db;

	//Delete the database, if it exists.
	if(exists("database")) {
		remove("database");
	}

	ASSERT_EQ(0, sqlite3_open("database", &db));

	ASSERT_NO_THROW(file_book.save_to(db));

	sqlite3_close(db);

	ASSERT_EQ(0, sqlite3_open("database", &db));

	ASSERT_NO_THROW(Epub book(db, 1));

	Epub sql_book(db, 1);

	ASSERT_TRUE(file_book.filename == sql_book.filename);
	ASSERT_TRUE(file_book.absolute_path == sql_book.absolute_path);
	ASSERT_TRUE(file_book.hash == sql_book.hash);
	ASSERT_TRUE(file_book.hash_string == sql_book.hash_string);

	ASSERT_TRUE(file_book.container.rootfiles.size() == sql_book.container.rootfiles.size());

	unsigned int i;

	for(i = 0; i < file_book.container.rootfiles.size(); i++) {

		RootFile & rf_book = file_book.container.rootfiles[i];
		RootFile & rf_sql = sql_book.container.rootfiles[i];

		ASSERT_TRUE(rf_book.media_type == rf_sql.media_type);
		ASSERT_TRUE(rf_book.full_path == rf_sql.full_path);

		ASSERT_TRUE(file_book.opf_files[i].metadata.size() == sql_book.opf_files[i].metadata.size());

		auto book_metadata_it = file_book.opf_files[i].metadata.begin();
		auto sql_metadata_it = sql_book.opf_files[i].metadata.begin();

		while(book_metadata_it != file_book.opf_files[i].metadata.end()) {

			MetadataItem m_book = (*book_metadata_it).second;
			MetadataItem m_sql = (*sql_metadata_it).second;

			ASSERT_TRUE(m_book.type == m_sql.type);
			ASSERT_TRUE(m_book.contents.compare(m_sql.contents) == 0);

			auto book_tags_it = m_book.other_tags.begin();
			auto sql_tags_it = m_sql.other_tags.begin();

			while(book_tags_it != m_book.other_tags.end()) {

				ustring book_first = (*book_tags_it).first;
				ustring book_second = (*book_tags_it).second;
				ustring sql_first = (*sql_tags_it).first;
				ustring sql_second = (*sql_tags_it).second;

				ASSERT_TRUE(book_first.compare(sql_first) == 0);
				ASSERT_TRUE(book_second.compare(sql_second) == 0);

				++book_tags_it;
				++sql_tags_it;

			}

			++book_metadata_it;
			++sql_metadata_it;

		}

		ASSERT_TRUE(file_book.opf_files[i].manifest.size() == sql_book.opf_files[i].manifest.size());

		auto book_manifest_it = file_book.opf_files[i].manifest.begin();
		auto sql_manifest_it = sql_book.opf_files[i].manifest.begin();

		while(book_manifest_it != file_book.opf_files[i].manifest.end()) {

			ustring href_book = (*book_manifest_it).second.href;
			ustring href_sql = (*sql_manifest_it).second.href;
			ustring id_book = (*book_manifest_it).second.id;
			ustring id_sql = (*sql_manifest_it).second.id;
			ustring media_type_book = (*book_manifest_it).second.media_type;
			ustring media_type_sql = (*sql_manifest_it).second.media_type;

			ASSERT_TRUE(href_book.compare(href_sql) == 0);
			ASSERT_TRUE(id_book.compare(id_sql) == 0);
			ASSERT_TRUE(media_type_book.compare(media_type_sql) == 0);

			++book_manifest_it;
			++sql_manifest_it;

		}

		ASSERT_TRUE(file_book.opf_files[i].spine.size() == sql_book.opf_files[i].spine.size());

		for(unsigned int j = 0; j < file_book.opf_files[i].spine.size(); j++) {

			SpineItem book_si = file_book.opf_files[i].spine[j];
			SpineItem sql_si = file_book.opf_files[i].spine[j];

			ASSERT_TRUE(book_si.idref.compare(sql_si.idref) == 0);
			ASSERT_TRUE(book_si.linear == sql_si.linear);

		}

		ASSERT_TRUE(file_book.css[i].rules.size() == sql_book.css[i].rules.size());

		auto book_css_it = file_book.css[i].rules.begin();
		auto sql_css_it = sql_book.css[i].rules.begin();

		while(book_css_it != file_book.css[i].rules.end()) {

			CSSRule c_book = *book_css_it;
			CSSRule c_sql = *sql_css_it;

			ASSERT_TRUE(c_book.selector == c_sql.selector);
			ASSERT_TRUE(c_book.collation_key == c_sql.collation_key);
			ASSERT_TRUE(c_book.displaytype == c_sql.displaytype);
			ASSERT_TRUE(c_book.fontsize == c_sql.fontsize);
			ASSERT_TRUE(c_book.fontweight == c_sql.fontweight);
			ASSERT_TRUE(c_book.fontstyle == c_sql.fontstyle);
			ASSERT_TRUE(c_book.margintop == c_sql.margintop);
			ASSERT_TRUE(c_book.marginbottom == c_sql.marginbottom);
			ASSERT_TRUE(c_book.pagebreakbefore == c_sql.pagebreakbefore);
			ASSERT_TRUE(c_book.pagebreakafter == c_sql.pagebreakafter);
			ASSERT_TRUE(c_book.textalign == c_sql.textalign);
			ASSERT_TRUE(c_book.textindent == c_sql.textindent);

			auto book_tags_it = c_book.raw_pairs.begin();
			auto sql_tags_it = c_sql.raw_pairs.begin();

			while(book_tags_it != c_book.raw_pairs.end()) {

				ustring book_first = (*book_tags_it).first;
				ustring book_second = (*book_tags_it).second;
				ustring sql_first = (*sql_tags_it).first;
				ustring sql_second = (*sql_tags_it).second;

				ASSERT_TRUE(book_first.compare(sql_first) == 0);
				ASSERT_TRUE(book_second.compare(sql_second) == 0);

				++book_tags_it;
				++sql_tags_it;

			}

			++book_css_it;
			++sql_css_it;

		}


	}

	sqlite3_close(db);

	//Delete the database
	remove("database");

}


