#!/usr/bin/env python
# encoding: utf-8
# Brant Young, 2007

"Process *.rc* files for C/C++: X{.rc -> [.res|.rc.o]}"

import os
import re
from waflib import Task, Utils
from waflib.TaskGen import extension
from waflib.Tools import c_preproc
from waflib import Utils

@extension('.rc')
def rc_file(self, node):
	"""
	Binds the .rc extension to a winrc task
	"""
	obj_ext = '.rc.o'
	if self.env.WINRC_TGT_F == '/fo':
		obj_ext = '.res'
	rctask = self.create_task('winrc', node, node.change_ext(obj_ext))
	try:
		self.compiled_tasks.append(rctask)
	except AttributeError:
		self.compiled_tasks = [rctask]

re_lines = re.compile(
	r'(?:^[ \t]*(#|%:)[ \t]*(ifdef|ifndef|if|else|elif|endif|include|import|define|undef|pragma)[ \t]*(.*?)\s*$)|'\
	r'(?:^\w+[ \t]*(ICON|BITMAP|CURSOR|HTML|FONT|MESSAGETABLE|TYPELIB|REGISTRY|D3DFX)[ \t]*(.*?)\s*$)',
	re.IGNORECASE | re.MULTILINE)

class rc_parser(c_preproc.c_parser):
	"""
	Calculates dependencies in .rc files
	"""
	def filter_comments(self, node):
		"""
		Overrides :py:meth:`waflib.Tools.c_preproc.c_parser.filter_comments`
		"""
		code = node.read()
		if c_preproc.use_trigraphs:
			for (a, b) in c_preproc.trig_def:
				code = code.split(a).join(b)
		code = c_preproc.re_nl.sub('', code)
		code = c_preproc.re_cpp.sub(c_preproc.repl, code)
		ret = []
		for m in re.finditer(re_lines, code):
			if m.group(2):
				ret.append((m.group(2), m.group(3)))
			else:
				ret.append(('include', m.group(5)))
		return ret

class winrc(Task.Task):
	"""
	Compiles resource files
	"""
	run_str = '${WINRC} ${WINRCFLAGS} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${WINRC_TGT_F} ${TGT} ${WINRC_SRC_F} ${SRC}'
	color   = 'BLUE'
	def scan(self):
		tmp = rc_parser(self.generator.includes_nodes)
		tmp.start(self.inputs[0], self.env)
		return (tmp.nodes, tmp.names)

	def exec_command(self, cmd, **kw):
		if self.env.WINRC_TGT_F == '/fo':
			# Since winres include paths may contain spaces, they do not fit in
			# response files and are best passed as environment variables
			replace_cmd = []
			incpaths = []
			while cmd:
				# filter include path flags
				flag = cmd.pop(0)
				if flag.upper().startswith('/I'):
					if len(flag) == 2:
						incpaths.append(cmd.pop(0))
					else:
						incpaths.append(flag[2:])
				else:
					replace_cmd.append(flag)
			cmd = replace_cmd
			if incpaths:
				# append to existing environment variables in INCLUDE
				env = kw['env'] = dict(kw.get('env') or self.env.env or os.environ)
				pre_includes = env.get('INCLUDE', '')
				env['INCLUDE'] = pre_includes + os.pathsep + os.pathsep.join(incpaths)

		return super(winrc, self).exec_command(cmd, **kw)

	def quote_flag(self, flag):
		if self.env.WINRC_TGT_F == '/fo':
			# winres does not support quotes around flags in response files
			return flag

		return super(winrc, self).quote_flag(flag)


def configure(conf):
	"""
	Detects the programs RC or windres, depending on the C/C++ compiler in use
	"""
	v = conf.env
	if not v.WINRC:
		if v.CC_NAME == 'msvc':
			if Utils.is_win32:
				conf.find_program('RC', var='WINRC', path_list=v.PATH)
			else:
				llvm_env_path = conf.environ.get('LLVM_PATH')
				llvm_path = None
				if llvm_env_path:
					llvm_path = llvm_env_path
				elif 'LLVM_PATH' in v:
					llvm_path = v['LLVM_PATH']

				paths = v.PATH
				if llvm_path:
					paths = [llvm_path] + v.PATH
				conf.find_program('llvm-rc', var='WINRC', path_list=paths)

			v.WINRC_TGT_F = '/fo'
			v.WINRC_SRC_F = ''
		else:
			conf.find_program('windres', var='WINRC', path_list=v.PATH)
			v.WINRC_TGT_F = '-o'
			v.WINRC_SRC_F = '-i'

