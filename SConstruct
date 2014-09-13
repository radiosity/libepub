#Copyright (c) 2013, Richard Martin
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#    * Neither the name of Richard Martin nor the
#      names of its contributors may be used to endorse or promote products
#      derived from this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL RICHARD MARTIN BE LIABLE FOR ANY
#DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

VariantDir('build/debug', 'src')
VariantDir('build/release', 'src')

#

envLibRelease = Environment()

envLibRelease['CXXFLAGS'] = "-O2 -std=c++11 -Wall -Wfatal-errors -pedantic"
envLibRelease['CPPPATH'] = "include"
	
envLibRelease.ParseConfig('pkg-config libxml++-2.6 glibmm-2.4 --cflags --libs')
envLibRelease.Append(LIBS=['boost_system', 'boost_filesystem', 'cryptopp'])
 
sources = Glob('build/release/*.cpp') 
 
rst = envLibRelease.SharedLibrary('bin/epub++', sources)
rsh = envLibRelease.StaticLibrary('bin/epub++', sources)

#

envLibDebug = Environment()

envLibDebug['CXXFLAGS'] = "-O0 -g -std=c++11 -Wall -Wfatal-errors -pedantic"
envLibDebug['CPPPATH'] = "include"
	
envLibDebug.ParseConfig('pkg-config libxml++-2.6 glibmm-2.4 --cflags --libs')
envLibDebug.Append(LIBS=['boost_system', 'boost_filesystem', 'cryptopp'])
envLibDebug.Append(CPPDEFINES=['DEBUG'])
 
sources = Glob('build/debug/*.cpp') 
 
envLibDebug.SharedLibrary('bin/epub++-debug', sources)
envLibDebug.StaticLibrary('bin/epub++-debug', sources)

#

envRelease = Environment()

envRelease['CXXFLAGS'] = "-O2 -std=c++11 -Wall -Wfatal-errors -pedantic"
envRelease['CPPPATH'] = "include"
	
envRelease.ParseConfig('pkg-config libxml++-2.6 glibmm-2.4 --cflags --libs')
envRelease.Append(LIBS=['boost_system', 'boost_filesystem', 'cryptopp'])
 
sources = Glob('build/release/cli/*.cpp') 
sources += ['bin/libepub++.a']
 
envRelease.Program('bin/release', sources)

#

envDebug = Environment()

envDebug['CXXFLAGS'] = "-O0 -g -std=c++11 -Wall -Wfatal-errors -pedantic"
envDebug['CPPPATH'] = "include"
	
envDebug.ParseConfig('pkg-config libxml++-2.6 glibmm-2.4 --cflags --libs')
envDebug.Append(LIBS=['boost_system', 'boost_filesystem', 'cryptopp'])
envDebug.Append(CPPDEFINES=['DEBUG'])
 
sources = Glob('build/debug/cli/*.cpp') 
sources += ['bin/libepub++-debug.a']
 
envDebug.Program('bin/debug', sources)
