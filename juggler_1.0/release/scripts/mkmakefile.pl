#!/usr/bin/perl

# ************** <auto-copyright.pl BEGIN do not edit this line> **************
#
# VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          $RCSfile$
# Date modified: $Date$
# Version:       $Revision$
# -----------------------------------------------------------------
#
# *************** <auto-copyright.pl END do not edit this line> ***************

require 5.004;

use strict 'vars';
use vars qw($dir_prfx $exp_file $gmake $sub_objdir);
use vars qw(%VARS);

use File::Basename;
use File::Copy;
use Getopt::Long;

# Do this to include the path to the script in @INC.
my $path;

BEGIN {
    $path = (fileparse("$0"))[1];
}

use lib qw($path /home/mystify /home/mystify/juggler/release/scripts);
use InstallOps;
use SourceList;

# Subroutine prototypes.
sub printHelp();
sub findSources(@);
sub readSources($);
sub printObjMakefile($$@);
sub printAppMakefile($$$);
sub printMultiAppMakefile($$$);
sub printAppMakefileStart_in($@);
sub printAppObjs_in($$$);
sub printAppSuffixRules_in($$);
sub printAppMakefileEnd_in($;@);
sub expandAll($);

# Set default values for all variables used as storage by GetOptions().
my $app     = '';
my %apps    = ();
my $dotin   = 1;
my $gmake   = 0;
my $help    = 0;
my @srcdirs = ();
my @subdirs = ();

$dir_prfx   = '';
$sub_objdir = '';

GetOptions('app=s' => \$app, 'apps=s' => \%apps, 'dirprefix=s' => \$dir_prfx,
           'dotin!' => \$dotin, 'expfile=s' => \$exp_file, 'gmake' => \$gmake,
           'help' => \$help, 'srcdir=s' => \@srcdirs, 'subdirs=s' => \@subdirs,
           'subobjdir=s' => \$sub_objdir)
    or printHelp() && exit(1);

printHelp() && exit(0) if $help;

# This allows source directories to be specified using multiple --srcdir
# options and/or using the form --srcdir=dir1,dir2,...,dirN.
@srcdirs = split(/,/, join(',', @srcdirs));
@subdirs = split(/,/, join(',', @subdirs));

my $all_src = findSources(@srcdirs);

# Set the generated file name once since all the subroutines will create a
# file with the same name.
my $makefile_name  = 'Makefile';
$makefile_name    .= ".in" if $dotin;

# Create $makefile_name here.
open(MAKEFILE, "> $makefile_name")
    or die "ERROR: Could not create $makefile_name: $!\n";

# Generate a single-application makefile.
if ( $app ) {
    printAppMakefile(MAKEFILE, $all_src, "$app");
}
# Generate a multi-application makefile.
elsif ( keys(%apps) ) {
    printMultiAppMakefile(MAKEFILE, $all_src, \%apps);
}
# Generate a Doozer++ object-building makefile.
else {
    printObjMakefile(MAKEFILE, $all_src, @subdirs);
}

close(MAKEFILE) or warn "WARNING: Could not close $makefile_name: $!\n";

# Now replace all the @...@ strings if this is not a Makefile.in.
unless ( $dotin ) {
    expandAll($makefile_name);
}

exit(0);

# =============================================================================
# Subroutines follow.
# =============================================================================

# -----------------------------------------------------------------------------
# Print a usage message explaining how to use this script.
# -----------------------------------------------------------------------------
sub printHelp () {
    print <<EOF;
Usage:
    $0
        [ --app=<name> | --apps <name1>=<srcs1> [ --apps <name2>=<srcs2> ... ]]
        [ --dirprefix=<prefix> ] [ --dotin | --nodotin --expfile=<file> ]
        [ --gmake ] [ --srcdir=<dirs> [...] ] [ --subdirs=<dirs> [...] ]
        [ --subobjdir=<dir> ]

For application makefiles:

    --app=<name>
        Specify the name of the application to be compiled.

  or

    --apps <name1>=<srcs1> --apps <name2>=<srcs2> ... --apps <nameN>=<srcsN>
        Specify the names of multiple applications and the lists of source
        files that will be compiled for each application.

    --gmake
        Use GNU make features.

    --srcdir=<dirs1> --srcdir=<dirs2> ... --srcdir=<dirsN>
        Give a list of directories besides the current directory where
        source files will be found.  Each directory can be given as an
        argument to a separate --srcdir option, or they can all be passed
        to a single --srcdir option as a comma-separated list (no spaces).

For object makefiles:

    --dirprefix=<prefix>
        Name a prefix for the directory containing the source files.

    --subdirs=<dirs1> --subdirs=<dirs2> ... --subdirs=<dirsN>
        Give a list of subdirectories of the current directory where the
        build must recurse.  Each subdirectory can be given as an argument
        to a separate --subdirs option, or they can all be passed to a
        single --subdirs option as a comma-separated list (no spaces).

    --subobjdir=<dir>
        Name the subdirectory of \$(OBJDIR) where the object files will go.

For both types of makefiles:

    --dotin (default)
        Generate Makefile.in

  or

    --nodotin --expfile=<Perl file>
        Generate a fully expanded Makefile using the %VARS hash in the
        file named as the argument to --expfile.
EOF
}

# -----------------------------------------------------------------------------
# Find all the source files in the given directories.
# -----------------------------------------------------------------------------
sub findSources (@) {
    my @dirs = @_;

    # Make sure that the current directory is in the list of directories to
    # search.
    my $found_dot = 0;

    foreach ( @dirs ) {
        if ( "$_" eq "." ) {
            $found_dot = 1;
            last;
        }
    }

    push(@dirs, ".") unless $found_dot;

    # Create the primary SourceList object.  It will be populated with
    # information in the following loop.
    my $srcs = new SourceList();
    my $dir;

    # Loop over the given source directories.
    foreach $dir ( @dirs ) {
        next if $srcs->hasDirectory("$dir");
        $srcs->readSources("$dir") or warn "Failed to read sources in $dir\n";
    }

    return $srcs;
}

# -----------------------------------------------------------------------------
# Generate a makefile for use with Doozer++ that can compile object files and
# recurse into subdirectories if necessary.
# -----------------------------------------------------------------------------
sub printObjMakefile ($$@) {
    my $handle  = shift;
    my $srcs    = shift;
    my @subdirs = @_;

    my $include_dir = '@includedir@';
    $include_dir .= "/$dir_prfx" if $dir_prfx;

    # Print out the basic stuff that is common to all types of Doozer++
    # makefiles.
    print $handle <<MK_START;
default: all

# Include common definitions here.
#include ...

includedir	= $include_dir
srcdir		= \@srcdir\@
top_srcdir	= \@top_srcdir\@
INSTALL		= \@INSTALL\@
MK_START

    my($has_objs, $has_subdirs) = (0, 0);

    # Get a list of all the files in the directory.
    my @src_files = $srcs->getAllFiles();
    $has_objs = 1 if $#src_files > -1;

    if ( $sub_objdir && $has_objs ) {
        print $handle "SUBOBJDIR\t= $sub_objdir\n";
    }

    print $handle "\n";

    # If there are subdirectories, list them.
    if ( $#subdirs > -1 ) {
        $has_subdirs = 1;
        print $handle "DIRPRFX\t= $dir_prfx/\n\n" if $dir_prfx;

        print $handle "# Subdirectories to compile.\n";
        print $handle "SUBDIR\t=";

        foreach ( @subdirs ) {
            print $handle " $_" unless "$_" eq "." ;
        }

        print $handle "\n\n";
    }

    # If there are object files to compile, list the source files.
    if ( $has_objs ) {
        print $handle "SRCS\t= ", join(' ', sort(@src_files)), "\n";

        # If the directory list has more than one element (which would be
        # the current directory), add an assignment for $(EXTRA_SRCS_PATH) so
        # that make will know where to find all the source files.
        my @other_dirs = $srcs->getDirectories();
        if ( $#other_dirs > 0 ) {
            print $handle "EXTRA_SRCS_PATH\t= ", join(' ', sort(@other_dirs)),
                          "\n";
        }

        print $handle "\n";
    }

    warn "No object files found, no subdirectories listed!\n"
        unless $has_objs || $has_subdirs;

    # Include the mk file that build objects and recurses if both object files
    # and subdirectories must be handled.
    if ( $has_objs && $has_subdirs ) {
        print $handle "include \$(MKPATH)/dpp.obj-subdir.mk\n\n";
    }
    # If we have only objects to build, include that mk file.
    elsif ( $has_objs ) {
        print $handle "include \$(MKPATH)/dpp.obj.mk\n\n";
    }
    # If we have only subdirectories, include the recursion file.
    elsif ( $has_subdirs ) {
        print $handle "\$(RECTARGET): recursive\n\n";
        print $handle "include \$(MKPATH)/dpp.subdir.mk\n\n";
    }

    # If this directory has object files, generate the dependency including
    # code too.
    if ( $has_objs ) {
        print $handle <<MK_END;
# -----------------------------------------------------------------------------
# Include dependencies generated automatically.
# -----------------------------------------------------------------------------
ifndef DO_CLEANDEPEND
include \$(DEPEND_FILES)
endif
MK_END
    }
}

# -----------------------------------------------------------------------------
# Generate a makefile capable of compiling a single application.  This is
# relatively simple.
# -----------------------------------------------------------------------------
sub printAppMakefile ($$$) {
    my $handle = shift;
    my $srcs   = shift;
    my $app    = shift;

    my @dirs = $srcs->getDirectories();

    print $handle "default: $app\@EXEEXT\@\n\n";

    printAppMakefileStart_in($handle, @dirs);
    printAppObjs_in($handle, 'OBJS', $srcs);

    # Print the target for the application.
    print $handle <<MK_END;
$app\@EXEEXT\@: \$(OBJS)
	\$(LINK) \@EXE_NAME_FLAG\@ \$(OBJS) \$(BASIC_LIBS) \$(EXTRA_LIBS)

MK_END

    printAppSuffixRules_in($handle, $srcs);
    printAppMakefileEnd_in($handle, "$app");
}

# -----------------------------------------------------------------------------
# Print a makefile that can compile multiple applications.  This would be
# simpler, but I put in some sanity checking that verifies that the source
# files listed on the command line actually exist.
# -----------------------------------------------------------------------------
sub printMultiAppMakefile ($$$) {
    my $handle = shift;
    my $srcs   = shift;
    my %apps   = %{$_[0]};

    my @dirs = $srcs->getDirectories();

    print $handle "default: all\n\n";

    printAppMakefileStart_in($handle, @dirs);

    my @cur_srcs = ();
    my @all_apps = sort(keys(%apps));
    my $app;

    # Loop over all the applications and verify that their specific source
    # files exist.
    foreach $app ( @all_apps ) {
        @cur_srcs = split(/,/, "$apps{$app}");

        # Create a new SourceList object for this application's source list.
        my $cur_app_src = new SourceList(@cur_srcs);

        # This the actual sanity check loop.
        my $file;
        foreach $file ( @cur_srcs ) {
            # Find the current file in the known list.
            my $dir = $srcs->findFile("$file");

            # If it was found, insert it into the correct directory block for
            # the SourceList object.
            if ( $dir ) {
                $cur_app_src->insertFile("$file", "$dir");
            }
            # If it was not found, complain.
            else {
                warn "WARING: Could not find file $file!\n";
            }
        }

        # Finally, generate the object list for this application.
        printAppObjs_in($handle, "${app}_OBJS", $cur_app_src);
    }

    # Loop over the applications again and print the targets that build the
    # actual executables.
    foreach ( @all_apps ) {
        print $handle <<MK_END;
$_\@EXEEXT\@: \$(${_}_OBJS)
	\$(LINK) \@EXE_NAME_FLAG\@ \$(${_}_OBJS) \$(BASIC_LIBS) \$(EXTRA_LIBS)

MK_END
    }

    printAppSuffixRules_in($handle, $srcs);
    printAppMakefileEnd_in($handle, @all_apps);
}

# -----------------------------------------------------------------------------
# Print the variable assignments and other "leading" information common to all
# applications to be compiled using this system.
# -----------------------------------------------------------------------------
sub printAppMakefileStart_in ($@) {
    my $handle = shift;
    my @dirs   = @_;

    # Construct the complete list of include paths.
    my $inc_line = '@APP_INCLUDES@';
    foreach ( @dirs ) {
        if ( "$_" eq "." ) {
            $inc_line .= ' -I$(srcdir)';
        }
        else {
            $inc_line .= " -I\$(srcdir)/$_";
        }
    }

    print $handle <<MK_START;
# Basic options.
srcdir		= \@srcdir\@
CFLAGS		= \@APP_CFLAGS\@ \$(EXTRA_FLAGS) \$(INCLUDES) \$(DEFS)
CXXFLAGS	= \@APP_CXXFLAGS\@ \$(EXTRA_FLAGS) \$(INCLUDES) \$(DEFS)
DEFS		= \@APP_DEFS\@
EXTRA_FLAGS	= \@APP_EXTRA_FLAGS\@ \$(DEBUG_FLAGS\)
DEBUG_FLAGS	= \@APP_DEBUG_FLAGS\@
OPTIM_FLAGS	= \@APP_OPTIM_FLAGS\@
INCLUDES	= $inc_line
LINK_FLAGS	= \@APP_LINK_FLAGS\@ \$(EXTRA_FLAGS)

# Libraries needed for linking.
BASIC_LIBS	= \@APP_BASIC_LIBS_BEGIN\@ \@APP_BASIC_LIBS\@ \@APP_BASIC_EXT_LIBS\@ \@APP_BASIC_LIBS_END\@
EXTRA_LIBS	= \@APP_EXTRA_LIBS_BEGIN\@ \@APP_EXTRA_LIBS\@ \@APP_EXTRA_LIBS_END\@ 

# Commands to execute.
C_COMPILE	= \@APP_CC\@ \$(CFLAGS)
CXX_COMPILE	= \@APP_CXX\@ \$(CXXFLAGS)
LINK		= \@APP_LINK\@ \$(LINK_FLAGS)

MK_START

    # Fill in the VPATH info.  We'll take advantage of GNU make syntax where
    # possible.
    if ( $gmake ) {
        print $handle "vpath \$(srcdir)";

        foreach ( @dirs ) {
            if ( "$_" ne "." ) {
                print $handle " \$(srcdir)/$_";
            }
        }
    }
    else {
        print $handle "VPATH = \@srcdir\@";

        foreach ( @dirs ) {
            if ( "$_" ne "." ) {
                print $handle ":\@srcdir\@/$_";
            }
        }
    }

    # Finish the VPATH line.
    print $handle "\n\n";
}

# -----------------------------------------------------------------------------
# Print the assignment for the list of object files to be compiled.  This code
# looks pretty simple, but the ugliness is being hidden by the SourceList
# class.
# -----------------------------------------------------------------------------
sub printAppObjs_in ($$$) {
    my($handle, $obj_var, $srcs) = @_;

    # Get the list of files as a scalar and change the suffixes to @OBJEXT@.
    my $files = join(' ', sort($srcs->getAllFiles()));
    $files    =~ s/\.(cpp|cxx|c\+\+|cc|c)/.\@OBJEXT\@/gi;

    # XXX: Wrap me!
    print $handle "$obj_var\t= $files\n\n";
}

# -----------------------------------------------------------------------------
# Print the suffix rules needed for this appliation makefile.  Only the rules
# needed for the known sources are generated to keep the file simple.
# -----------------------------------------------------------------------------
sub printAppSuffixRules_in ($$) {
    my($handle, $srcs) = @_;

    my @suffixes = sort($srcs->getSuffixes());

    print $handle "# Suffix rules for building object files.\n";
    print $handle ".SUFFIXES: ", join(' ', @suffixes), " .\@OBJEXT\@\n\n";

    # XXX: Could use GNU stuff too.
    foreach ( @suffixes ) {
        print $handle "$_.\@OBJEXT\@:\n";

        # C source file.
        if ( "$_" eq ".c" ) {
            print $handle "\t\$(C_COMPILE) \@OBJ_NAME_FLAG\@ \@OBJ_BUILD_FLAG\@ \$<\n\n";
        }
        # C++ source file.
        else {
            print $handle "\t\$(CXX_COMPILE) \@OBJ_NAME_FLAG\@ \@OBJ_BUILD_FLAG\@ \$<\n\n";
        }
    }
}

# -----------------------------------------------------------------------------
# Finish off the application makefile by adding 'clean' and 'clobber' targets.
# -----------------------------------------------------------------------------
sub printAppMakefileEnd_in ($;@) {
    my $handle = shift;
    my @apps   = @_;

    my $dirt ='';
    foreach ( @_ ) {
        $dirt .= "$_\@EXEEXT\@ ";
    }

    print $handle <<MK_END;
# Clean-up targets.
clean:
	rm -f *.\@OBJEXT\@ core*
	rm -rf ii_files

clobber:
	\@\$(MAKE) clean
	rm -f $dirt
MK_END
}

# -----------------------------------------------------------------------------
# Expand all the @...@ strings using the %VARS hash.
# -----------------------------------------------------------------------------
sub expandAll ($) {
    my $filename = shift;

    # Suck in the full %VARS hash.
    require "$exp_file" if $exp_file;

    warn "WARNING: No expansion values!\n" unless keys(%VARS) && $exp_file;

    replaceTags("$filename", %VARS);
}
