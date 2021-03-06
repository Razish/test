#
# SCons project file
#	written by Raz0r
#
# options:
#	debug		generate debug information, value 2 also enables optimisations
#	force32		force 32 bit target when on 64 bit machine
#	target		comma-separated list of targets to build, default is 'main'
#	CC			ONLY USE WITH SCAN-BUILD - specify the underlying C compiler to use
#	CXX			ONLY USE WITH SCAN-BUILD - specify the underlying C++ compiler to use
#
# example:
#	scons -Q target=test debug=1 force32=1
#	scan-build scons -Q target=test,irc debug=1 force32=1 CC=gcc CXX=g++
#
# envvars:
#	MORE_WARNINGS	enable additional warnings
#	SCONS_DEBUG		debug scons tool detection
#

debug = int( ARGUMENTS.get( 'debug', 0 ) )
configuration = { 0: lambda x: 'release', 1: lambda x: 'debug', 2: lambda x: 'optimised-debug' }[debug](debug)
force32 = int( ARGUMENTS.get( 'force32', 0 ) )
toolStr = ARGUMENTS.get( 'tools', 'gcc,g++,ar,as,gnulink' )
tools = [x for x in toolStr.split( ',' )]
targetStr = ARGUMENTS.get( 'target', 'main' )
targets = [x for x in targetStr.split( ',' )]

# compare semantic versions (1.0.2 < 1.0.10 < 1.2.0)
def cmp_version( v1, v2 ):
	def normalise( v ):
		import re
		return [int(x) for x in re.sub( r'(\.0+)*$', '', v ).split( '.' )]

	return cmp(
		normalise( v1 ),
		normalise( v2 )
	)

import platform
plat = platform.system() # Windows or Linux
try:
	bits = int( platform.architecture()[0][:2] ) # 32 or 64
except( ValueError, TypeError ):
	bits = None
arch = None # platform-specific, set manually

# architecture settings, needed for binary names, also passed as a preprocessor definition
if force32:
	bits = 32
if bits == 32:
	if plat == 'Windows':
		arch = 'x86'
	elif plat == 'Linux':
		if platform.machine()[:3] == 'arm':
			arch = 'arm'
		else:
			arch = 'i386'
	elif plat == 'Darwin':
		arch = 'i386'
	else:
		raise Exception( 'unexpected platform: ' + plat )
elif bits == 64:
	if plat == 'Windows':
		arch = 'x64'
	elif plat == 'Linux':
		arch = 'x86_64'
	elif plat == 'Darwin':
		arch = 'x86_64'
	else:
		raise Exception( 'unexpected platform: ' + plat )
else:
	raise Exception( 'could not determine architecture width: ' + str(bits) )

clangHack = plat == 'Darwin'

# create the build environment
#FIXME: also consider LD, AS, AR in the toolset
import os
env = Environment(
	TARGET_ARCH = arch,
	tools = tools,
	ENV = { 'PATH' : os.environ['PATH'] }
)

# set the proper compiler name
realcc = ARGUMENTS.get( 'CC', None )
realcxx = ARGUMENTS.get( 'CXX', None )

if 'CC' in os.environ:
	if 'ccc' in os.environ['CC']:
		env['CC'] = os.environ['CC']
		env['CXX'] = os.environ['CXX']
		# running a scan-build
		if realcc is None or realcxx is None:
			raise Exception( 'please specify CC/CXX as such: scons CC=gcc CXX=g++' )
	else:
		# not a scan-build, do not allow inheriting CC/CXX from outside environment
		raise Exception( 'please specify CC/CXX as such: scons CC=gcc CXX=g++' )
elif realcc is None and realcxx is None:
	realcc = env['CC']
	realcxx = env['CXX']
	#FIXME: realcxx is "$CC" on Windows/MSVC

env['ENV'].update( x for x in os.environ.items() if x[0].startswith( 'CCC_' ) )

build_dir = 'build' + os.sep + configuration + os.sep + realcc + os.sep + str(bits) + os.sep
env.VariantDir( build_dir, '.', duplicate = 0 )

# targets
backend = {
	'Windows': lambda x: [],
	#TODO: add win32 api files (may need to revise this for cross-compiling and mingw etc)

	'Linux': lambda x: [],
	'Darwin': lambda x: [],
	#TODO: add posix/unix files
}[plat](plat)
build_targets = {
	'main': [
		build_dir + f for f in [
			'main.c',
		]
	] + backend,
	'minimum-char': [
		build_dir + f for f in [
			'minimum-char.c',
		]
	] + backend,
}
if env.GetOption( 'clean' ):
	targets = build_targets

# prettify the compiler output
if 'TERM' in os.environ:
	env['ENV']['TERM'] = os.environ['TERM']
import sys
textctrl = {}
enableColours = sys.stdout.isatty() # detect ANSI colour support
textctrl['bold-on'] = '\033[1m' if enableColours else ''
textctrl['bold-off'] = '\033[21m' if enableColours else ''
textctrl['grey'] = '\033[90m' if enableColours else ''
textctrl['red'] = '\033[91m' if enableColours else ''
textctrl['green'] = '\033[92m' if enableColours else ''
textctrl['yellow'] = '\033[93m' if enableColours else ''
textctrl['lightblue'] = '\033[94m' if enableColours else ''
textctrl['magenta'] = '\033[95m' if enableColours else ''
textctrl['cyan'] = '\033[96m' if enableColours else ''
textctrl['white'] = '\033[97m' if enableColours else ''
textctrl['end']  = '\033[0m' if enableColours else ''

env['SHCCCOMSTR'] = env['SHCXXCOMSTR'] = env['CCCOMSTR'] = env['CXXCOMSTR'] = \
	'%s compiling: %s$SOURCE%s' % (textctrl['cyan'], textctrl['white'], textctrl['end'])
env['ARCOMSTR'] = \
	'%s archiving: %s$TARGET%s' % (textctrl['yellow'], textctrl['white'], textctrl['end'])
env['RANLIBCOMSTR'] = \
	'%s  indexing: %s$TARGET%s' % (textctrl['yellow'], textctrl['white'], textctrl['end'])
env['ASCOMSTR'] = \
	'%sassembling: %s$TARGET%s' % (textctrl['yellow'], textctrl['white'], textctrl['end'])
env['SHLINKCOMSTR'] = env['LINKCOMSTR'] = \
	'%s   linking: %s$TARGET%s' % (textctrl['green'], textctrl['white'], textctrl['end'])

# obtain the compiler version
import commands
if realcc == 'cl':
	# msvc
	ccversion = env['MSVC_VERSION']
elif realcc == 'gcc' or realcc == 'clang':
	status, ccrawversion = commands.getstatusoutput( realcc + ' -dumpversion' )
	ccversion = None if status else ccrawversion

# scons version
import SCons
sconsversion = SCons.__version__

# git revision
status, rawrevision = commands.getstatusoutput( 'git rev-parse --short HEAD' )
revision = None if status else rawrevision

if revision:
	status, dummy = commands.getstatusoutput( 'git diff-index --quiet HEAD' )
	if status:
		revision += '*'

# set job/thread count
def GetNumCores():
	if plat == 'Linux' or plat == 'Darwin':
		# works on recent mac/linux
		status, num_cores = commands.getstatusoutput( 'getconf _NPROCESSORS_ONLN' )
		if status == 0:
			return int(num_cores)

		# only works on linux
		status, num_cores = commands.getstatusoutput( 'cat /proc/cpuinfo | grep processor | wc -l' )
		if status == 0:
			return int(num_cores)

		return 1;

	elif plat == 'Windows':
		# exists since at-least XP SP2
		return int( os.environ['NUMBER_OF_PROCESSORS'] )
env.SetOption( 'num_jobs', GetNumCores() )

# notify the user of the build configuration
if not env.GetOption( 'clean' ):
	# build tools
	msg = textctrl['white'] + 'building '
	if revision:
		revision_modified = revision[-1] == '*'
		if revision_modified:
			msg += textctrl['bold-on']
		msg += textctrl['red' if revision_modified else 'green'] + revision + textctrl['white'] + ' '
		if revision_modified:
			msg += textctrl['bold-off']
else:
	msg = textctrl['white'] + 'cleaning '

msg += 'for ' + textctrl['bold-on'] + textctrl['cyan'] + plat + ' '
msg += (textctrl['red'] if force32 else '') + str(bits) + ' bits' + textctrl['white'] + textctrl['bold-off']\
	+ ' using ' + textctrl['bold-on'] + str(env.GetOption( 'num_jobs' )) + textctrl['bold-off'] + ' threads\n'
msg += '\tconfiguration:' + ' ' + textctrl['bold-on'] + textctrl['cyan'] + configuration + textctrl['white']\
	+ textctrl['bold-off'] + '\n'
msg += '\tinstruction set:' + ' ' + textctrl['bold-on'] + textctrl['cyan'] + arch + textctrl['white']\
	+ textctrl['bold-off'] + (' with ' + textctrl['bold-on'] + textctrl['cyan']\
	+ ('x87 fpu' if 'NO_SSE' in os.environ else 'SSE')\
	+ textctrl['white'] + textctrl['bold-off'] if arch != 'arm' else '') + '\n\n'

msg += '\t' + realcc + '/' + realcxx + ':' + ' ' + textctrl['bold-on'] + ccversion + textctrl['bold-off'] + '\n'
msg += '\tpython:' + ' ' + textctrl['bold-on'] + platform.python_version() + textctrl['bold-off'] + '\n'
msg += '\tscons:' + ' ' + textctrl['bold-on'] + sconsversion + textctrl['bold-off'] + '\n\n'

# build targets
msg += 'targets:\n'
for t in build_targets:
	if t in targets:
		msg += '\t' + textctrl['bold-on'] + textctrl['green']\
			+ t + u' \u2713' + textctrl['white'] + textctrl['bold-off'] + '\n'
	else:
		msg += '\t' + textctrl['white'] + t + '\n'

# build environment
if 'SCONS_DEBUG' in os.environ:
	msg += realcc + ' located at ' + commands.getoutput( 'where ' + realcc ).split( '\n' )[0] + '\n'
	if 'AR' in env:
		msg += env['AR'] + ' located at ' + commands.getoutput( 'where ' + env['AR'] ).split( '\n' )[0] + '\n'
	if 'AS' in env:
		msg += env['AS'] + ' located at ' + commands.getoutput( 'where ' + env['AS'] ).split( '\n' )[0] + '\n'
	msg += 'python located at ' + sys.executable + '\n'
	msg += 'scons' + ' located at ' + commands.getoutput( 'where ' + 'scons' ).split( '\n' )[0] + '\n'

print( msg )

# clear default compiler/linker switches
def emptyEnv( env, e ):
	if 'SCONS_DEBUG' in os.environ:
		if e in env:
			if env[e]:
				print( 'discarding ' + e + ': ' + env[e] )
			else:
				print( 'env[' + e + '] is empty' )
		else:
			print( 'env[' + e + '] does not exist' )
	env[e] = []
emptyEnv( env, 'CPPDEFINES' )
emptyEnv( env, 'CFLAGS' )
emptyEnv( env, 'CCFLAGS' )
emptyEnv( env, 'CXXFLAGS' )
emptyEnv( env, 'LINKFLAGS' )
emptyEnv( env, 'ARFLAGS' )

# compiler switches
if realcc == 'gcc' or realcc == 'clang':
	env['CCFLAGS'] += [
		#'-M',	# show include hierarchy
	]
	# c warnings
	env['CFLAGS'] += [
		'-Wdeclaration-after-statement',
		'-Wnested-externs',
		'-Wold-style-definition',
		'-Wstrict-prototypes',
	]

	# c/cpp warnings
	env['CCFLAGS'] += [
		'-Wall',
		'-Wextra',
		'-Wno-missing-braces',
		'-Wno-missing-field-initializers',
		'-Wno-sign-compare',
		'-Wno-unused-parameter',
		'-Winit-self',
		'-Winline',
		'-Wmissing-include-dirs',
		'-Woverlength-strings',
		'-Wpointer-arith',
		'-Wredundant-decls',
		'-Wundef',
		'-Wuninitialized',
		'-Wwrite-strings',
	]

	# strict c/cpp warnings
	if 'MORE_WARNINGS' in os.environ:
		env['CCFLAGS'] += [
			'-Waggregate-return',
			'-Wbad-function-cast',
			'-Wcast-qual',
			'-Wfloat-equal',
			'-Wlong-long',
			'-Wshadow',
			'-Wsign-conversion',
			'-Wswitch-default',
			'-Wunreachable-code',
		]
		if not clangHack:
			env['CCFLAGS'] += [
				'-Wdouble-promotion',
				'-Wsuggest-attribute=const',
				'-Wunsuffixed-float-constants',
			]

	# gcc-specific warnings
	if realcc == 'gcc' and cmp_version( ccversion, '4.6' ) >= 0 and arch != 'arm':
		env['CCFLAGS'] += [
			'-Wlogical-op',
		]

		# requires gcc 4.7 or above
		if cmp_version( ccversion, '4.7' ) >= 0:
			env['CCFLAGS'] += [
				'-Wstack-usage=32768',
			]

	# disable warnings
	env['CCFLAGS'] += [
		'-Wno-char-subscripts',
	]

	# c/cpp flags
	if arch == 'arm':
		env['CCFLAGS'] += [
			'-fsigned-char',
		]
	else:
		env['CCFLAGS'] += [
			'-mstackrealign',
			#'-masm=intel', # bah, glm doesn't like this
		]
		if 'NO_SSE' in os.environ:
			env['CCFLAGS'] += [
				'-mfpmath=387',
				'-mno-sse2',
				'-ffloat-store',
			]
			if realcc == 'gcc':
				env['CFLAGS'] += [
					'-fexcess-precision=standard',
				]
		else:
			env['CCFLAGS'] += [
				'-mfpmath=sse',
				'-msse2',
			]
		if arch == 'i386':
			env['CCFLAGS'] += [
				'-march=i686',
			]
		elif arch == 'x86_64':
			env['CCFLAGS'] += [
				'-mtune=generic',
			]

		if bits == 32:
			env['CCFLAGS'] += [
				'-m32',
			]
			env['LINKFLAGS'] += [
				'-m32',
			]
	env['CCFLAGS'] += [
		'-fvisibility=hidden',
	]

	# misc settings
	#if realcc == 'gcc' and cmp_version( ccversion, '4.9' ) >= 0:
	#	env['CCFLAGS'] += [
	#		'-fdiagnostics-color',
	#	]

	# c flags
	env['CFLAGS'] += [
		'-std=gnu11',
	]

	# c++ flags
	env['CXXFLAGS'] += [
		'-fvisibility-inlines-hidden',
		'-std=c++11',
	]

	if plat == 'Darwin':
		env['FRAMEWORKS'] = [ 'OpenGL' ]

	# archive flags
	env['ARFLAGS'] = 'rc'

elif realcc == 'cl':
	# msvc
	env['CCFLAGS'] += [
		#'/showIncludes',
	]
	env['CFLAGS'] += [
		'/TC',	# compile as c
	]
	env['CXXFLAGS'] += [
		'/TP',	# compile as c++
	]
	env['CCFLAGS'] += [
		'/EHsc',	# exception handling
		'/nologo',	# remove watermark
	]

	env['LINKFLAGS'] += [
		'/ERRORREPORT:none',	# don't send error reports for internal linker errors
		'/NOLOGO',				# remove watermark
		'/MACHINE:' + arch,		# 32/64 bit linking
	]
	if bits == 64:
		env['LINKFLAGS'] += [
			'/SUBSYSTEM:WINDOWS',		# graphical application
		]
	else:
		env['LINKFLAGS'] += [
			'/SUBSYSTEM:WINDOWS,5.1',	# graphical application, XP support
		]

	env['CPPDEFINES'] += [
		'_WIN32',
	]
	if bits == 64:
		env['CPPDEFINES'] += [
			'_WIN64',
		]

	# fpu control
	if 'NO_SSE' in os.environ:
		env['CCFLAGS'] += [
			'/fp:precise',	# precise FP
		]
		if bits == 32:
			env['CCFLAGS'] += [
				'/arch:IA32',	# no sse, x87 fpu
			]
	else:
		env['CCFLAGS'] += [
			'/fp:strict',	# strict FP
		]
		if bits == 32: # and cmp_version( ccversion, '14.0' ) < 0
			env['CCFLAGS'] += [
				'/arch:SSE2',	# sse2
			]

	# strict c/cpp warnings
	if 'LESS_WARNINGS' in os.environ:
		env['CCFLAGS'] += [
			'/W2',
		]
	else:
		env['CCFLAGS'] += [
			'/W4',
			'/we 4013',
			'/we 4024',
			'/we 4026',
			'/we 4028',
			'/we 4029',
			'/we 4033',
			'/we 4047',
			'/we 4053',
			'/we 4087',
			'/we 4098',
			'/we 4245',
			'/we 4305',
			'/we 4700',
		]
	if 'MORE_WARNINGS' in os.environ:
		env['CCFLAGS'] += [
			'/Wall',
		]
	else:
		env['CCFLAGS'] += [
			'/wd 4100',
			'/wd 4127',
			'/wd 4244',
			'/wd 4706',
			'/wd 4131',
			'/wd 4996',
		]

	env['LINKFLAGS'] += [
		'/NODEFAULTLIB:LIBCMTD',
		'/NODEFAULTLIB:MSVCRT',
	]

# debug / release
if debug == 0 or debug == 2:
	if realcc == 'gcc' or realcc == 'clang':
		env['CCFLAGS'] += [
			'-O2', # O3 may not be best, due to cache size not being big enough for the amount of inlining performed
			'-fomit-frame-pointer',
		]
		if debug == 0 and realcc == 'gcc':
			env['LINKFLAGS'] += [
				'-s',	# strip unused symbols
			]
	elif realcc == 'cl':
		env['CCFLAGS'] += [
			'/O2',	# maximise speed
			'/MT',	# multi-threaded runtime
		]
		env['LINKFLAGS'] += [
			'/OPT:REF',			# remove unreferenced functions/data
			'/STACK:32768',		# stack size
		]

	if debug == 0:
		env['CPPDEFINES'] += [
			'NDEBUG',
		]

if debug:
	if realcc == 'gcc' or realcc == 'clang':
		env['CCFLAGS'] += [
			'-g3',
			#'-pg',
			#'-finstrument-functions',
			'-fno-omit-frame-pointer',
			'-fsanitize=address',
		]
		env['LINKFLAGS'] += [
		#	'-pg',
			'-fsanitize=address',
		]
	elif realcc == 'cl':
		env['CCFLAGS'] += [
			'/Od',		# disable optimisations
			'/Z7',		# emit debug information
			'/MTd',		# multi-threaded debug runtime
		]
		env['LINKFLAGS'] += [
			'/DEBUG',			# generate debug info
			'/OPT:ICF',			# enable COMDAT folding
			'/INCREMENTAL:NO',	# no incremental linking
		]

	env['CPPDEFINES'] += [
		'_DEBUG',
	]

if revision:
	env['CPPDEFINES'] += [
		'GIT_REVISION=\\"' + revision + '\\"',
	]

env['CPPDEFINES'] += [ {
		'Windows': lambda x: 'OS_WINDOWS',
		'Linux': lambda x: 'OS_LINUX',
		'Darwin': lambda x: 'OS_MAC',
	}[plat](plat)
]

# build-time settings
env['CPPDEFINES'] += [
	'COMPILER=\\"' + realcc + ' ' + ccversion + '\\"',
]

env['CPPPATH'] = [
	'#',
]

# arch width
env['CPPDEFINES'] += [
	'ARCH_WIDTH=' + str( bits ),
	'ARCH_STRING=\\"' + arch + '\\"'
]

# set up libraries to link with
if plat == 'Linux' or plat == 'Darwin':
	libs = [
		'm',
	]
elif plat == 'Windows':
	# windows libs are in e.g. "../lib/x86/libfoobar15d.lib" for 32 bit debug build of foobar 1.5
	env['LIBPATH'] = '#' + os.sep + '..' + os.sep + 'lib' + os.sep + '' + arch + os.sep
	libs = [
		#'Kernel32', # kernel api for LoadLibrary, OpenProcess etc
		#'Shell32', # win32 shell api for ShellExecute, CLI<->GUI communication
		#'User32', # mostly for window management
		#'Ws2_32', # winsock2 api
	]
	# provided libs with release + debug versions, so append the signifying 'd' based on configuration
	# lib will be prefixed so 'libfoobar15d.lib' should be 'foobar15' in this list
	dlibs = [
	]
	libs += [lib + ('d' if debug == 1 else '') for lib in dlibs]

# libraries
libraries = [
	# local projects with source to be built into a lib
]
env['LIBS'] = [
	env.SConscript(
		'#' + os.sep + lib + os.sep + 'SConscript',
		exports = [
			'arch',
			'bits',
			'build_dir',
			'configuration',
			'env',
			'plat',
			'realcc',
		]
	) for lib in libraries
]
env['LIBS'] += libs

# build targets
for target in targets:
	if target not in build_targets:
		continue
	binaryName = target + '.' + arch + env['PROGSUFFIX']
	env.Program( binaryName, build_targets[target] )

	#if not env.GetOption( 'clean' ) and not os.path.lexists( './' + target ):
	#	print( 'Suggest creating a symlink: "ln -s ' + binaryName + ' ' + target + '"' )
