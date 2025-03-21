#! /usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2015

"""
Force files to also depend on the timestamps of those located in the build directory. You may
want to use this to force partial rebuilds, see playground/track_output_files/ for a working example.

Note that there is a variety of ways to implement this, one may want use timestamps on source files too for example,
or one may want to hash theA files in the source directory only under certain conditions (md5_tstamp tool)
"""

import os
from waflib import Node, Utils

def get_bld_sig(self):
	if not self.is_bld() or self.ctx.bldnode is self.ctx.srcnode:
		return Utils.h_file(self.abspath())

	val = Utils.h_file(self.abspath()) + str(os.stat(self.abspath()).st_mtime).encode('latin-1')
	return val

Node.Node.get_bld_sig = get_bld_sig

