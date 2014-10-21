#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find Eigen
    if conf.options.eigen_dir:
        if not conf.options.eigen_incdir:
            conf.options.eigen_incdir=conf.options.eigen_dir + "/include"
    frag="#include <eigen3/Eigen/Eigen>\n" + 'int main()\n' \
        + "{Eigen::MatrixXd M;}\n"
    if conf.options.eigen_incdir:
        eigen_inc=[conf.options.eigen_incdir]
    else:
        eigen_inc=[]

    conf.check_cxx(msg="Checking for Eigen",
                  fragment=frag,
                  includes=eigen_inc, uselib_store='eigen')

def options(opt):
    eigen=opt.add_option_group('Eigen Options')
    eigen.add_option('--eigen-dir',
                   help='Base directory where eigen is installed')
    eigen.add_option('--eigen-incdir',
                   help='Directory where eigen include files are installed')

