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

#ifndef EPUB_HEADER
#define EPUB_HEADER

#include <string>
#include <utility>
#include <vector>
#include <cstdlib>
#include <sqlite3.h> 

#include "Container.hpp"
#include "OPF.hpp"
#include "Content.hpp"
#include "CSS.hpp"

using std::vector; 
using std::size_t; 

class Epub {
	
	public:
	
		string filename; 
		size_t hash; 
		string hash_string; 
	
		path directory_path; 
	
		Container container; 
		vector<OPF> opf_files;
		vector<CSS> css; 
		vector<Content> contents;
	
		Epub(string _filename);
		
		Epub(Epub const & cpy);
		Epub(Epub && mv);
		Epub& operator =(const Epub& cpy);
		Epub& operator =(Epub && mv);
		
		~Epub() ;
	
		void save_to(sqlite3 * const db); 
	
		static inline size_t compute_epub_hash(string _filename); 
		
};

#endif