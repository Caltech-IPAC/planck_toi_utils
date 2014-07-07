#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    import os
    wcs_incdir=[]
    wcs_libdir=[]
    for d in ['CM_ENV_DIR','CM_TPS_DIR']:
        env_dir=os.getenv(d)
        if not env_dir:
            break
        if d=='CM_ENV_DIR':
            env_dir=env_dir+'/core'
        if env_dir!=None:
            wcs_incdir.append(env_dir+'/include')
            wcs_libdir.append(env_dir+'/lib')

    if not conf.options.wcs_dir:
        conf.fatal("No value given for wcs-dir.  Tried environment "
                   + "variable $CM_BASE_DIR")

    # Find wcs
    if conf.options.wcs_dir:
        if not conf.options.wcs_incdir:
            conf.options.wcs_incdir=conf.options.wcs_dir + "/include"
        if not conf.options.wcs_libdir:
            conf.options.wcs_libdir=conf.options.wcs_dir + "/lib"

    if conf.options.wcs_incdir:
        wcs_incdir.append(conf.options.wcs_incdir)
    if conf.options.wcs_libdir:
        wcs_libdir.append(conf.options.wcs_libdir)

    if conf.options.wcs_libs:
        wcs_libs=conf.options.wcs_libs.split()
    else:
        wcs_libs=['wcs','wcstools']

    wcs_fragment='extern "C" {\n' + \
        '#include "wcs.h"\n' + \
        '}\n' + \
        'int main()\n{\n ' + \
        'wcscon(WCS_J2000,WCS_GALACTIC,0,0,nullptr,nullptr,1950);\n' + '}\n'

    import copy
    found_wcs=False
    for wcs_lib in wcs_libs:
        msg="Checking for WCS library"
        if wcs_lib:
            test_libs=[wcs_lib]
            msg+=" with " + wcs_lib
        else:
            test_libs=copy.copy(wcs_libs)
        try:
            conf.check_cc(features="cxx cxxprogram",
                          msg=msg,
                          fragment=wcs_fragment,
                          includes=wcs_incdir,
                          uselib_store='wcs',
                          libpath=wcs_libdir,
                          rpath=wcs_libdir,
                          lib=test_libs,
                          execute=False)
        except conf.errors.ConfigurationError:
            continue
        else:
            found_wcs=True
            break
    if not found_wcs:
        conf.fatal("Could not find WCS libraries")


def options(opt):
    wcs=opt.add_option_group('WCS Options')
    wcs.add_option('--wcs-dir',
                   help='Base directory where wcs is installed')
    wcs.add_option('--wcs-incdir',
                   help='Directory where wcs include files are installed')
    wcs.add_option('--wcs-libdir',
                   help='Directory where wcs library files are installed')
    wcs.add_option('--wcs-libs',
                   help='Names of the wcs libraries without prefix or suffix\n'
                   '(e.g. "wcs")')
