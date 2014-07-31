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

#ifndef CONTAINER_HEADER
#define CONTAINER_HEADER

#include <vector>
#include <boost/filesystem.hpp>
#include <glibmm.h>

using std::vector;

using namespace boost::filesystem;
using namespace Glib;

class RootFile {
	
public:
	ustring media_type;
	ustring full_path;

	RootFile(ustring _m, ustring _f);
		
	RootFile(RootFile const & cpy);
	RootFile(RootFile && mv) ;
	RootFile& operator =(const RootFile& cpy);
	RootFile& operator =(RootFile && mv) ;
		
	~RootFile();
	
};


class Container {

	public:
	
		vector<RootFile> rootfiles;
	
		Container(path to_dir);
		
		Container(Container const & cpy);
		Container(Container && mv);
		Container& operator =(const Container& cpy);
		Container& operator =(Container && mv);
		
		~Container();
	
};

#endif