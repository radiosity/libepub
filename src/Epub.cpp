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

#include "Epub.hpp"

#include <utility>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <locale>
#include <sys/types.h>
#include <sys/wait.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>

using std::string;
using std::move;
using std::size_t;
using std::stringstream;
using std::locale; 
using std::hex; 
using std::ifstream;
using namespace boost::filesystem;
using boost::lexical_cast; 
using boost::hash_combine; 

#ifdef DEBUG
#include <iostream>
using std::cout; 
using std::endl;
#endif

Epub::Epub(string _filename) : 
	filename(_filename)
{
	
	//check if the file exists first. 
	if(!exists(_filename)) {
		throw std::runtime_error("No such filename");
	}
	
	//It does exist. 
	
	path file(_filename);
	unsigned int size = file_size(file);
	time_t lmtime = last_write_time(file);
	string timestring = lexical_cast<std::string>(lmtime);
	
	#ifdef DEBUG
	cout << "File Exists" << endl; 
	cout << "\t Name: " << _filename << endl; 
	cout << "\t Size: " << size << endl; 
	cout << "\t Last Modified: " << timestring << endl; 
	#endif
	
	//Calculate the hash. 
	
	hash = 0; 
	hash_combine<string>(hash, _filename);
	hash_combine<unsigned int>(hash, size);
	hash_combine<string>(hash, timestring);
	
	stringstream stream;
	//Have to set the locale on the stringstream 
	//to "C" otherwise it does insane things like
	//formatting hex with decimal comma groups. 
	//Yeah. About that. 
	locale cloc("C");
	stream.imbue(cloc);
	stream << std::hex << (unsigned long) hash;
	hash_string = stream.str();
	
	#ifdef DEBUG
	cout << "\t Hash: " << hash_string << endl; 
	#endif
		
	//Now to do work. 
	
	path to_tmp = temp_directory_path(); 
	
	#ifdef DEBUG
	cout << "Temporary path is " << to_tmp << endl; 
	#endif
	
	to_tmp /= "epub";
	create_directory(to_tmp);
	to_tmp /= hash_string; 
	directory_path = to_tmp;
	
	#ifdef DEBUG
	cout << "Temporary directory is " << to_tmp << endl; 
	#endif
	
	create_directory(directory_path);
	
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
		#ifdef DEBUG
		cout << "Waiting for pid " << pid << endl; 
		#endif
		waitpid(pid, &status, 0);
		#ifdef DEBUG
		cout << "PID joined " << pid << endl; 
		#endif		
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
	
	//OK, file is validated and unpacked in a temporary directory
	
	container.load(directory_path);
	
	for(auto rf : container.rootfiles) {
		
		OPF tmp(directory_path, rf.full_path);
		
		opf_files.push_back(tmp);
		
		vector<path> contentfiles; 
		
		for(auto si : tmp.spine) {
			ustring idref = si.idref;

			ManifestItem item = tmp.find_manifestitem_by_id(idref);
			
			path tmp(rf.full_path);
			path parent = tmp.parent_path();
			
			path contentfile = directory_path;
			contentfile /= parent;
			contentfile /= path(item.href); 
			
			contentfiles.push_back(contentfile);
			
		}
		
		Content content(contentfiles);
			
		contents.push_back(content);
		
	}
	
}

Epub::Epub(Epub const & cpy) : 
	filename(cpy.filename), 
	hash(cpy.hash),
	hash_string(cpy.hash_string),
	directory_path(cpy.directory_path),
	container(cpy.container),
	opf_files(cpy.opf_files),
	contents(cpy.contents)
{
	
}

Epub::Epub(Epub && mv)  : 
	filename (move(mv.filename)), 
	hash(move(mv.hash)),
	hash_string(move(mv.hash_string)),
	directory_path(move(mv.directory_path)),
	container(move(mv.container)),
	opf_files(move(mv.opf_files)),
	contents(move(mv.contents))
{
	
}

Epub& Epub::operator =(const Epub& cpy) { 
	filename = cpy.filename;
	hash = cpy.hash; 
	hash_string = cpy.hash_string; 
	directory_path = cpy.directory_path; 
	container = cpy.container;
	opf_files = cpy.opf_files;
	contents = cpy.contents;
	return *this; 
}

Epub& Epub::operator =(Epub && mv) { 
	filename = move(mv.filename); 
	hash = move(mv.hash);  
	hash_string = move(mv.hash_string);
	directory_path = move(mv.directory_path);
	container = move(mv.container);
	opf_files = move(mv.opf_files);
	contents = move(mv.contents);
	return *this; 
}

Epub::~Epub() {
	#ifdef DEBUG
	cout << "Cleaning up EpubFile" << endl; 
	#endif
	
	remove_all(directory_path);
}

