#! /usr/bin/env python
# encoding: utf-8

from waflib import Options

# the following two variables are used by the target "waf dist"
VERSION='1.0.0'
APPNAME='c-tlv-serialize'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
	opt.load('compiler_c')
	opt.add_option('--mode', action='store', default='release', help='Compile mode: release or debug', choices=['debug', 'release'])
	opt.add_option('--cross', default='', help='Cross compiler prefix, e.g. mips-linux-gnu-')
	opt.add_option('--cc', default='gcc')

def configure(conf):
	conf.env.CROSS = conf.options.cross
	conf.env.CC = conf.env.CROSS + conf.options.cc
	conf.load('compiler_c')

	include_dirs = ['lib']
	cflags = []
	ldflags = []
	if Options.options.mode == 'debug':
		cflags += ['-DDEBUG', '-g3', '-O0']
	elif Options.options.mode == 'release':
		cflags += ['-g0', '-Os']

	conf.env.append_value('INCLUDES', include_dirs)
	conf.env.append_value('CFLAGS', cflags)
	conf.env.append_value('LINKFLAGS', ldflags)

def build(bld):
	libsrc = bld.path.ant_glob('lib/*.c')
	bld.stlib(source=libsrc, target='tlvserialize-static')
	bld.shlib(source=libsrc, target='tlvserialize-shared')
	bld.program(source='main.c', target='main', use='tlvserialize-static')
