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

#include "EpubFile.hpp"

#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <exception>
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <sys/wait.h>

using std::string;
using std::move;

using std::cout; 
using std::ifstream;
using std::endl; 

using namespace boost::filesystem;

EpubFile::EpubFile(string _filename) : filename(_filename) {

	//check if the file exists first. 
	if(!exists(_filename)) {
		throw std::runtime_error("No such filename");
	}
		
	//Now to do work. 
	
	path to_tmp = temp_directory_path(); 
	
	cout << "Temporary path is " << to_tmp << endl; 
	
	to_tmp /= "epub";
	directory_path = to_tmp;
	
	cout << "Temporary directory is " << to_tmp << endl; 
	
	create_directory(to_tmp);
	
	//OK, so we have some stuff to play with. Now. Time to inflate. 
	//This is a hack and I bloody hate it. But it'll suffice for now. 
	
	const char * program_path = "/usr/bin/unzip";
	const char * program_name = "unzip";
	
	auto pid = fork(); 
	
	if (pid == -1) {
		throw std::runtime_error("Some problem with fork()");
	}
	else if (pid == 0) {
		const char* argv0 = program_name; 
		const char* argv1 = filename.c_str();
		const char* argv2 = "-d";
		const char* argv3 = directory_path.string().c_str();
		execl(program_path, argv0, argv1, argv2, argv3, NULL);
		throw std::runtime_error("Some problem with execl");
	}
	else{
		int status; 
		waitpid(pid, &status, 0);			
	}

	path to_mimetype = to_tmp;
	to_mimetype /= "mimetype";
	
	ifstream mimetypefile (to_mimetype.string());
	if(mimetypefile.is_open()) {
		string line; 
		getline(mimetypefile, line);
		const string target = "application/epub+zip";
		int res = line.compare(target);
		if(res != 0) {
			throw std::runtime_error("mimetype file present, but invalid");
		}
	}
	else {
		throw std::runtime_error("No mimetype file, is this an epub?");
	}
	
	path to_container = to_tmp; 
	to_container /= "META-INF";
	to_container /= "container.xml";
	
	if(!exists(to_container)) {
		throw std::runtime_error("container.xml does not exist within META-INF dir");
	}
	
	//OK, file is validated. 
	
}

EpubFile::EpubFile(EpubFile const & cpy) : filename(cpy.filename), directory_path(cpy.directory_path){}
EpubFile::EpubFile(EpubFile && mv)  : filename (move(mv.filename)), directory_path(move(mv.directory_path)){}
EpubFile& EpubFile::operator =(const EpubFile& cpy) { 
	filename = cpy.filename; 
	directory_path = cpy.directory_path;
	return *this; 
}
EpubFile& EpubFile::operator =(EpubFile && mv) { 
	filename = move(mv.filename); 
	directory_path = move(directory_path);
	return *this; 
}
	
void EpubFile::cleanup() {
	cout << "Cleaning up EpubFile" << endl; 
	remove_all(directory_path);
}

EpubFile::~EpubFile() {}
	
const path EpubFile::get_directory_path() const {
	return directory_path;
}
