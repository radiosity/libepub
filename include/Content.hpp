/*
Copyright (c) 2014, Richard Martin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in thedif;
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

#ifndef Content_HEADER
#define Content_HEADER

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/flyweight.hpp>
#include <glibmm.h>

using std::vector; 

using namespace boost::filesystem;
using namespace Glib;

enum ContentType {
	P,
	H1, 
	H2, 
	HR
};

class ContentItem {
	
	public:
		ContentType type;
		path file;
		ustring id;
		ustring content;
	
		ContentItem(ContentType type, path file, ustring id, ustring content);
		
		ContentItem(ContentItem const & cpy);
		ContentItem(ContentItem && mv) ;
		ContentItem& operator =(const ContentItem& cpy);
		ContentItem& operator =(ContentItem && mv) ;
			
		~ContentItem();
	
};

class Content {
	
	public :
		vector<path> files; 
		vector<ContentItem> items; 
	
		Content(vector<path> files);
		
		Content(Content const & cpy);
		Content(Content && mv) ;
		Content& operator =(const Content& cpy);
		Content& operator =(Content && mv) ;
			
		~Content();
	
	
};


#endif