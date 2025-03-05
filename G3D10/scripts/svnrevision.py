#!/usr/bin/env python3
# Generates header file with svn revision number in G3D-base.lib/source

import os
import sys
import itertools

# get top-level g3d directory
G3D_DIR = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')

# To get access to icompile routines
sys.path.append(os.path.join(G3D_DIR, 'bin'))
import ice.utils

outputFilename = os.path.join(G3D_DIR, 'G3D-base.lib', 'source', 'generated', 'svnrevision.h')

def updateSvnRevision(revisionString):
    # get latest local revision, defaulting to 0.
    revisions = revisionString.split(':')
    revisionNumber = "".join(itertools.takewhile(str.isdigit, revisions[-1]))

    # write revision number to source file
    revisionCode = "static const String __g3dSvnRevision = \"{0}\";".format(revisionNumber)
    print("Setting svn revision to " + revisionNumber)

    # read original and see if revision updated
    originalCode = ""
    if os.path.exists(outputFilename):
        with open(outputFilename, 'r') as sourceFile:
            originalCode = sourceFile.readline()

    # write svn revision to source only if updated (or new)
    if originalCode != revisionCode:            
        os.makedirs(os.path.dirname(outputFilename), exist_ok = True)
        with open(outputFilename, 'w') as sourceFile:
            sourceFile.write(revisionCode)

# query current revision from svn
rawRevisionString = "-1"

try:
    (result, rawRevisionString, _) = ice.utils.runWithOutput("svnversion", ["-n", G3D_DIR], False)
except:
    pass

updateSvnRevision(rawRevisionString)
