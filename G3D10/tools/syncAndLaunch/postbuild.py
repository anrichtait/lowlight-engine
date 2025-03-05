import getopt, platform, sys, os, socket, subprocess

# Settings
localBuildFilesDir=""
localBinaryDir=""
remoteBuildFilesDir=""
remoteBinaryFile=""
remotePlatform="windows"
remoteAddr = os.environ["G3D10_REMOTE_MACHINE"]

"""
TODO: determine remote platform automatically with the script instead of passing it as an argument
It should match the local platform because we're syncing the binaries (ie a Windows compiled binary would not work on a Ubuntu server).
"""
    
s = platform.system()
windows = (s == 'Windows') or (s == 'Microsoft')
freebsd = (s == 'FreeBSD')
linux   = (s == 'Linux')
osx     = (s == 'Darwin')
cygwin  = s.lower().startswith('cygwin')
unix    = freebsd or linux or osx or cygwin


try:
    opts, args = getopt.getopt(sys.argv[1:], "r:l:b:d:e:h", ["remotePlatform=", "localBuildFilesDir=", "localBinaryDir=", "remoteBuildFilesDir=", "remoteBinaryFile=", "--help"])
    
except Exception as e:
    print("Error getting options: ")
    print(e)
    exit(1)

for opt, arg in opts:
    if opt in ['-r', '--remotePlatform']:
        remotePlatform = arg
    if opt in ['-l', '--localBuildFilesDir']:
        localBuildFilesDir = arg
    if opt in ['-b', '--localBinaryDir']:
        localBinaryDir = arg
    if opt in ['-d', '--remoteBuildFilesDir']:
        remoteBuildFilesDir = arg
    if opt in ['-e', '--remoteBinaryFile']:
        remoteBinaryFile = arg
    if opt in ['-h', '--help']:
        print(
"""
InstantLaunch Prebuild Script: 
-r  --remotePlatform        the remote platform
-l  --localBuildFilesDir    the local build directory
-b  --localBinaryDir       the local binary file path
-d  --remoteBuildFilesDir   where the build will be copied to remotely
-e  --remoteBinaryFile      where the binary will be copied to remotely
-h  --help                  these instructions
"""
        )


def sanitizePath(path):
    splitString = path.replace('\\', '/').split(':')
    return '/mnt/' + splitString[0].lower() + splitString[1]    

print("Starting post build script...")

if windows:
    localHostname = socket.gethostname()
    if localHostname in remoteAddr or "localhost" in remoteAddr:
        print("Remote Machine is the same as local...Launching local binary file...")

        programName = sanitizePath(remoteBinaryFile).split('/')[-1]
        preSanitizedLocalBinaryFile = localBinaryDir
        localBinaryDir = sanitizePath(localBinaryDir)
        if not os.path.exists(preSanitizedLocalBinaryFile):
            print("Program binary not found...\nLooked for {}\nAborting...".format(preSanitizedLocalBinaryFile))
            exit(1)

        localBinaryPid = subprocess.Popen([preSanitizedLocalBinaryFile + programName]).pid
        print("Spawning program on separate process PID: {}".format(localBinaryPid))

        recordProgramNameOutput = os.system("wsl echo {} >> {}/temp.txt".format(programName, localBuildFilesDir))
        print("recording program in temp file")
            

    elif remotePlatform == "windows":
        # We're running matchmaker local to the client
        if localHostname not in os.popen("wsl ssh %G3D10_REMOTE_MACHINE% echo ^%G3D10_SERVICE_DISCOVERY^%").read() :
            print("Remote Machine is not pointing to our matchmaker... Aborting...")
            exit(1)

        localBuildFilesDir = sanitizePath(localBuildFilesDir)
        if remoteBuildFilesDir:
            remoteBuildFilesDir = sanitizePath(remoteBuildFilesDir)
        else:
            remoteBuildFilesDir = localBuildFilesDir

        wslRSyncCommand = "wsl rsync -ravpE --rsync-path='wsl rsync' -e ssh --exclude=\"*\" --include=\"*.dll\" --include=\"*.pdb\" {}/* {}:{}".format(localBuildFilesDir, remoteAddr, remoteBuildFilesDir)
        print(wslRSyncCommand)
        os.system(wslRSyncCommand)

        preSanitizedLocalBinaryFile = localBinaryDir
        localBinaryDir = sanitizePath(localBinaryDir)
        preSanitizedRemoteBinaryFile = remoteBinaryFile
        if remoteBinaryFile:
            remoteBinaryFile = sanitizePath(remoteBinaryFile)
        else:
            remoteBinaryFile = localBinaryDir
        if not os.path.exists(preSanitizedLocalBinaryFile):
            print("Program binary not found...\nLooked for {}\nAborting...".format(preSanitizedLocalBinaryFile))
            exit(1)

        wslRSyncBinary = "wsl rsync -ravpE --rsync-path='wsl rsync' -e ssh {} {}:{}".format(localBinaryDir, remoteAddr, remoteBuildFilesDir)
        print(wslRSyncBinary)
        os.system(wslRSyncBinary)

        # # uncomment to launch programs that don't require a window
        # launchRemoteBinaryOutput = os.popen("wsl ssh %G3D10_REMOTE_MACHINE% {}".format(preSanitizedRemoteBinaryFile)).read()
        # print(launchRemoteBinaryOutput)

        # programName = remoteBinaryFile.split('/')[-1]
        # recordProgramNameOutput = os.popen("wsl ssh %G3D10_REMOTE_MACHINE% wsl echo {} >> {}/temp.txt".format(programName, remoteBuildFilesDir)).read()
        # print(recordProgramNameOutput)

print("postbuild is done")